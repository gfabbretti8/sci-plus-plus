# sci++

**C++, but in Marchigiano.** *Scì*, it compiles.

sci++ is a source-to-source translator plus compiler driver: it takes `.sci` files written in
C++ with keywords replaced by words from the dialects of the Marche (Ancona, Macerata & co.),
translates them to plain C++ and hands them to `g++` (or whatever is in `$SCIPP_CXX`).

```cpp
#mettece <iostream>
usa paese std;

ntero principale() {
    pe' (ntero i = 0; i < 3; ++i) {
        se (i == 1) tirannanzi;
        strilla << "giro " << i << accapo;
    }
    ridà 0;
}
```

## Build & use

```sh
make            # builds ./sci++
make test       # compiles and runs esempi/ciao.sci
make install    # copies sci++ to /usr/local/bin

./sci++ file.sci -o programma        # any g++ flag works
./sci++ --traduci file.sci           # just print the generated C++
SCIPP_CXX=clang++ ./sci++ file.sci   # use another compiler
```

## How it works

- A small lexer that leaves string/char/raw literals, comments and numbers (`1'000`) alone and
  only translates identifiers — so `"so' de Ancona"` stays intact.
- Keywords ending in an apostrophe (`pe'`, `fa'`) are supported.
- Line numbers are preserved and a `#line` directive is emitted, so compiler errors point at
  your `.sci` file and show your original dialect source. Diagnostics get translated too:
  `error:` → `sbàjo:`, `warning:` → `occhio:`.

Known limitation: error *columns* refer to the translated text, so the `^` caret can be a few
characters off on lines where a dialect word is longer/shorter than its keyword.

## Vocabolario

| sci++ | C++ | | sci++ | C++ |
|---|---|---|---|---|
| `ntero` | `int` | | `se` | `if` |
| `lungo` / `corto` | `long` / `short` | | `sennò` | `else` |
| `senzasegno` | `unsigned` | | `pe'` | `for` |
| `lettera` | `char` | | `finché` | `while` |
| `virgola` / `virgolina` | `double` / `float` | | `fa'` | `do` |
| `scìonò` | `bool` | | `ridà` | `return` |
| `gnènte` | `void` | | `fermete` | `break` |
| `vedetu` | `auto` | | `tirannanzi` | `continue` |
| `scì` / `farzo` | `true` / `false` | | `scegli` / `caso` / `comunque` | `switch` / `case` / `default` |
| `gnisciù` | `nullptr` | | `provace` / `chiappa` / `tira` | `try` / `catch` / `throw` |
| `robba` / `classe` | `struct` / `class` | | `fermo` / `fisso` | `const` / `static` |
| `pubblico` / `privato` | `public` / `private` | | `paese` / `usa` | `namespace` / `using` |
| `finto` / `rifatto` | `virtual` / `override` | | `stampo` / `tipo` | `template` / `typename` |
| `nuovo` / `butta` | `new` / `delete` | | `pure` / `oppure` / `manco` | `and` / `or` / `not` |
| `principale` | `main` | | `strilla` / `ascorta` / `lagna` | `cout` / `cin` / `cerr` |
| `chiacchiera` | `string` | | `fila` / `accapo` | `vector` / `endl` |

Directives: `#mettece` (include), `#defini` (define), `#sedefinito` / `#senondefinito`,
`#finisce` (endif), `#unavorta` (pragma once).

Unaccented spellings (`sennò` → `senno`, `ridà` → `rida`, …) work too. The whole vocabulary is
the `kParole` table at the top of `scipp.cpp` — PRs from native speakers welcome.
