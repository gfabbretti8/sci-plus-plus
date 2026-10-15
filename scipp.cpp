// sci++ — 'r C++ marchigiano.
// Traduce i file .sci in C++ normale e po' chiama 'r compilatore.
//
//   sci++ [flag del compilatore] file.sci [altri file] -o programma
//   sci++ --traduci file.sci          # stampa solo 'r C++ tradotto
//
// Compilatore: g++ di default, oppure quello in $SCIPP_CXX.

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Vocabolario: dialetto -> C++. Le parole co' l'apostrofo (pe', fa') vanno
// scritte attaccate e seguite da 'n carattere che nun è lettera o cifra.
// ---------------------------------------------------------------------------
static const std::unordered_map<std::string, std::string> kParole = {
    // tipi
    {"ntero", "int"},          {"lungo", "long"},        {"corto", "short"},
    {"senzasegno", "unsigned"},{"colsegno", "signed"},   {"lettera", "char"},
    {"virgola", "double"},     {"virgolina", "float"},
    {"scìonò", "bool"},        {"scionò", "bool"},       {"sciono", "bool"},
    {"gnènte", "void"},        {"gnente", "void"},       {"vedetu", "auto"},
    // letterali
    {"scì", "true"},           {"sci", "true"},
    {"farzo", "false"},        {"nò", "false"},
    {"gnisciù", "nullptr"},    {"gnisciuno", "nullptr"},
    // controllo
    {"se", "if"},              {"sennò", "else"},        {"senno", "else"},
    {"pe'", "for"},            {"pe", "for"},            {"fa'", "do"},
    {"finché", "while"},       {"finche", "while"},
    {"ridà", "return"},        {"rida", "return"},
    {"fermete", "break"},      {"tirannanzi", "continue"},
    {"scegli", "switch"},      {"caso", "case"},         {"comunque", "default"},
    {"vacce", "goto"},
    // eccezioni
    {"provace", "try"},        {"chiappa", "catch"},     {"tira", "throw"},
    // classi e compagnia
    {"robba", "struct"},       {"classe", "class"},      {"elenco", "enum"},
    {"pubblico", "public"},    {"privato", "private"},   {"protetto", "protected"},
    {"finto", "virtual"},      {"rifatto", "override"},  {"amico", "friend"},
    {"questo", "this"},        {"nuovo", "new"},         {"butta", "delete"},
    {"operatore", "operator"},
    // varie
    {"fermo", "const"},        {"fisso", "static"},      {"paese", "namespace"},
    {"usa", "using"},          {"stampo", "template"},   {"tipo", "typename"},
    {"quantoè", "sizeof"},     {"quantoe", "sizeof"},
    {"pure", "and"},           {"oppure", "or"},         {"manco", "not"},
    // libreria standard
    {"principale", "main"},    {"strilla", "cout"},      {"ascorta", "cin"},
    {"lagna", "cerr"},         {"accapo", "endl"},       {"chiacchiera", "string"},
    {"fila", "vector"},
};

static const std::unordered_map<std::string, std::string> kDirettive = {
    {"mettece", "include"},        {"defini", "define"},
    {"sedefinito", "ifdef"},       {"senondefinito", "ifndef"},
    {"se", "if"},                  {"sennò", "else"},
    {"finisce", "endif"},          {"unavorta", "pragma once"},
};

static bool inizioNome(unsigned char c) { return std::isalpha(c) || c == '_' || c >= 0x80; }
static bool pezzoNome(unsigned char c)  { return std::isalnum(c) || c == '_' || c >= 0x80; }

// Copia 'n letterale "..." o '...' così com'è. Ritorna l'indice dopo la fine.
static size_t copiaLetterale(const std::string& s, size_t i, std::string& out) {
    const char q = s[i];
    out += s[i++];
    while (i < s.size()) {
        char c = s[i++];
        out += c;
        if (c == '\\' && i < s.size()) { out += s[i++]; continue; }
        if (c == q || c == '\n') break;
    }
    return i;
}

// Copia 'na raw string R"delim(...)delim". i punta alle virgolette.
static size_t copiaRaw(const std::string& s, size_t i, std::string& out) {
    size_t apre = s.find('(', i + 1);
    if (apre == std::string::npos) { out.append(s, i, std::string::npos); return s.size(); }
    std::string fine = ")" + s.substr(i + 1, apre - i - 1) + "\"";
    size_t e = s.find(fine, apre + 1);
    e = (e == std::string::npos) ? s.size() : e + fine.size();
    out.append(s, i, e - i);
    return e;
}

// Lo traduttore vero e proprio. Stringhe, caratteri e commenti nun se toccano,
// e le righe restano le stesse, così l'errori puntano a la riga giusta.
std::string traduci(const std::string& s) {
    std::string out;
    out.reserve(s.size() + s.size() / 8);
    const size_t n = s.size();
    size_t i = 0;
    bool inizioRiga = true;

    while (i < n) {
        const unsigned char c = s[i];
        const unsigned char d = i + 1 < n ? s[i + 1] : 0;

        if (c == '/' && d == '/') {                               // commento //
            size_t e = s.find('\n', i);
            if (e == std::string::npos) e = n;
            out.append(s, i, e - i); i = e; continue;
        }
        if (c == '/' && d == '*') {                               // commento /* */
            size_t e = s.find("*/", i + 2);
            e = (e == std::string::npos) ? n : e + 2;
            out.append(s, i, e - i); i = e; continue;
        }
        if (c == '"' || c == '\'') {                              // letterali
            i = copiaLetterale(s, i, out); inizioRiga = false; continue;
        }
        if (std::isdigit(c) || (c == '.' && std::isdigit(d))) {   // numeri (pure 1'000'000)
            size_t j = i + 1;
            while (j < n) {
                unsigned char x = s[j];
                if (std::isalnum(x) || x == '_' || x == '.') { ++j; continue; }
                if (x == '\'' && j + 1 < n && std::isalnum((unsigned char)s[j + 1])) { ++j; continue; }
                if ((x == '+' || x == '-') && std::strchr("eEpP", s[j - 1])) { ++j; continue; }
                break;
            }
            out.append(s, i, j - i); i = j; inizioRiga = false; continue;
        }
        if (c == '#' && inizioRiga) {                             // direttive
            out += s[i++];
            while (i < n && (s[i] == ' ' || s[i] == '\t')) out += s[i++];
            size_t j = i;
            while (j < n && pezzoNome(s[j])) ++j;
            std::string w = s.substr(i, j - i);
            auto it = kDirettive.find(w);
            std::string dir = it != kDirettive.end() ? it->second : w;
            out += dir; i = j; inizioRiga = false;
            if (dir == "include") {                               // <header> intatto
                while (i < n && (s[i] == ' ' || s[i] == '\t')) out += s[i++];
                if (i < n && s[i] == '<') {
                    size_t e = s.find('>', i);
                    e = (e == std::string::npos) ? n : e + 1;
                    out.append(s, i, e - i); i = e;
                }
            }
            continue;
        }
        if (inizioNome(c)) {                                      // parole
            size_t j = i;
            while (j < n && pezzoNome(s[j])) ++j;
            std::string w = s.substr(i, j - i);
            inizioRiga = false;
            if (j < n && s[j] == '"' && (w == "R" || w == "u8R" || w == "uR" || w == "UR" || w == "LR")) {
                out += w; i = copiaRaw(s, j, out); continue;
            }
            if (j < n && (s[j] == '"' || s[j] == '\'') && (w == "u8" || w == "u" || w == "U" || w == "L")) {
                out += w; i = copiaLetterale(s, j, out); continue;
            }
            if (j < n && s[j] == '\'' && !(j + 1 < n && pezzoNome(s[j + 1]))) {
                auto it = kParole.find(w + "'");
                if (it != kParole.end()) { out += it->second; i = j + 1; continue; }
            }
            auto it = kParole.find(w);
            out += it != kParole.end() ? it->second : w;
            i = j; continue;
        }

        out += s[i++];
        if (c == '\n') inizioRiga = true;
        else if (c != ' ' && c != '\t' && c != '\r') inizioRiga = false;
    }
    return out;
}

// ---------------------------------------------------------------------------
// Driver
// ---------------------------------------------------------------------------
static std::string traMenzo(const std::string& a) {           // quoting pe' la shell
    std::string r = "'";
    for (char c : a) r += (c == '\'') ? std::string("'\\''") : std::string(1, c);
    return r + "'";
}

static std::string scappa(const std::string& a) {             // escaping pe' #line
    std::string r;
    for (char c : a) { if (c == '\\' || c == '"') r += '\\'; r += c; }
    return r;
}

static void sostituisci(std::string& s, const std::string& da, const std::string& a) {
    for (size_t p = 0; (p = s.find(da, p)) != std::string::npos; p += a.size()) s.replace(p, da.size(), a);
}

static void aiuto() {
    std::cerr << "sci++ — 'r C++ de le Marche\n\n"
                 "  sci++ [flag] file.sci [...] [-o programma]\n"
                 "  sci++ --traduci file.sci     stampa 'r C++ tradotto\n\n"
                 "Compilatore: $SCIPP_CXX (de solito g++).\n";
}

int main(int argc, char** argv) {
    if (argc < 2) { aiuto(); return 1; }

    const char* env = std::getenv("SCIPP_CXX");
    const std::string cxx = env ? env : "g++";
    const fs::path tmp = fs::temp_directory_path() / ("scipp-" + std::to_string(getpid()));

    bool soloTraduci = false, haStd = false;
    std::vector<std::string> args;

    for (int k = 1; k < argc; ++k) {
        std::string a = argv[k];
        if (a == "--traduci" || a == "--translate") { soloTraduci = true; continue; }
        if (a == "-h" || a == "--aiuto" || a == "--help") { aiuto(); return 0; }
        if (a.rfind("-std=", 0) == 0) haStd = true;

        if (fs::path(a).extension() == ".sci") {
            std::ifstream in(a, std::ios::binary);
            if (!in) { std::cerr << "sci++: 'ndo sta " << a << "? Nun lo trovo.\n"; return 1; }
            std::stringstream buf; buf << in.rdbuf();
            std::string cpp = "#line 1 \"" + scappa(a) + "\"\n" + traduci(buf.str());

            if (soloTraduci) { std::cout << cpp; continue; }

            fs::create_directories(tmp);
            fs::path out = tmp / (fs::path(a).stem().string() + ".cpp");
            std::ofstream(out, std::ios::binary) << cpp;
            args.push_back(out.string());
            continue;
        }
        args.push_back(a);
    }
    if (soloTraduci) return 0;

    std::string cmd = traMenzo(cxx) + (haStd ? "" : " -std=c++17");
    for (auto& a : args) cmd += " " + traMenzo(a);
    cmd += " 2>&1";

    FILE* p = popen(cmd.c_str(), "r");
    if (!p) { std::cerr << "sci++: nun riesco a parti' " << cxx << "\n"; return 1; }
    char riga[4096];
    while (std::fgets(riga, sizeof riga, p)) {
        std::string r = riga;
        sostituisci(r, "fatal error:", "disastro:");
        sostituisci(r, "error:", "sbàjo:");
        sostituisci(r, "warning:", "occhio:");
        sostituisci(r, "note:", "nota bè:");
        std::cerr << r;
    }
    int st = pclose(p);

    std::error_code ec;
    fs::remove_all(tmp, ec);
    return WIFEXITED(st) ? WEXITSTATUS(st) : 1;
}
