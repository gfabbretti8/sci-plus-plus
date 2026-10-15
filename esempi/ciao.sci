#mettece <iostream>
#mettece <vector>
#mettece <string>

usa paese std;

robba Persona {
    chiacchiera nome;
    ntero anni;
};

scìonò e_vecchio(fermo Persona& p) {
    ridà p.anni >= 65;
}

stampo <tipo T>
T somma(fermo fila<T>& v) {
    T tot{};
    pe' (fermo vedetu& x : v) tot += x;
    ridà tot;
}

ntero principale() {
    fila<Persona> gente = {{"Sor Augusto", 82}, {"Giulietta", 23}, {"Pierino", 9}};

    strilla << "Ciao a tutti, so' de Ancona!" << accapo;

    pe' (fermo vedetu& p : gente) {
        se (e_vecchio(p)) {
            strilla << p.nome << ": pensionato" << accapo;
        } sennò se (p.anni < 18) {
            strilla << p.nome << ": va a scola" << accapo;
        } sennò {
            strilla << p.nome << ": lavora" << accapo;
        }
    }

    ntero i = 0;
    finché (scì) {
        se (++i > 4) fermete;
        se (i == 2) tirannanzi;     // er 2 lo saltamo
        strilla << "giro " << i << accapo;
    }

    strilla << "somma: " << somma(fila<ntero>{1, 2, 3, 4}) << accapo;

    provace {
        tira 42;
    } chiappa (ntero n) {
        lagna << "M'hai tirato " << n << accapo;
    }

    ridà 0;
}
