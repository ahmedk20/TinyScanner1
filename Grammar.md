# Tiny Language — Grammar Analysis

## 1. Original Grammar

```
P  -> S; P  | S;                 // program
S  -> F | R | A | K | W          // statement
F  -> if C then P O              // if-statement
O  -> end | else P end           // option (else / end)
R  -> repeat P until C           // repeat
C  -> E < E | E = E              // condition
E  -> T H E | T                  // expression
T  -> D | N                      // term
N  -> N G | G                    // number
H  -> + | - | * | /              // arithmetic operator
D  -> D L | D G | L              // identifier
L  -> [A-Z] | [a-z]              // letter
G  -> [0-9]                      // digit
A  -> D := E                     // assignment
K  -> read D                     // read
W  -> write D | write "D"        // write
```

---

## 2. Left-Recursion Elimination

Rule of the form `A -> A α | β` is rewritten as:
```
A  -> β A'
A' -> α A' | ε
```

### 2.1  N -> N G | G
```
N  -> G N'
N' -> G N' | ε
```

### 2.2  D -> D L | D G | L
```
D  -> L D'
D' -> L D' | G D' | ε
```

All other productions are either non-recursive or **right-recursive**
(P, E), so they need no left-recursion treatment.

---

## 3. Left Factoring

Rule of the form `A -> α β1 | α β2 | …` is rewritten as:
```
A  -> α A'
A' -> β1 | β2 | …
```

### 3.1  P -> S; P | S;
Common prefix `S;`
```
P  -> S; P'
P' -> P | ε
```

### 3.2  C -> E < E | E = E
Common prefix `E`
```
C  -> E C'
C' -> < E | = E
```

### 3.3  E -> T H E | T
Common prefix `T`
```
E  -> T E'
E' -> H E | ε
```

### 3.4  W -> write D | write "D"
Common prefix `write`
```
W  -> write W'
W' -> D | "D"
```

`O -> end | else P end` has **no** common prefix, so it is left as-is.

---

## 4. Final Grammar (after both transformations)

```
P   -> S; P'
P'  -> P | ε
S   -> F | R | A | K | W
F   -> if C then P O
O   -> end | else P end
R   -> repeat P until C
C   -> E C'
C'  -> < E | = E
E   -> T E'
E'  -> H E | ε
T   -> D | N
N   -> G N'
N'  -> G N' | ε
H   -> + | - | * | /
D   -> L D'
D'  -> L D' | G D' | ε
L   -> [A-Z] | [a-z]
G   -> [0-9]
A   -> D := E
K   -> read D
W   -> write W'
W'  -> D | "D"
```

---

## 5. FIRST Sets

> Note: at the parser level, the scanner emits `ID` (for D) and `NUMBER`
> (for N) as terminals. For completeness, both views are shown.

| Non-terminal | FIRST |
|---|---|
| L  | letter (a–z, A–Z) |
| G  | digit (0–9) |
| D  | letter |
| D' | letter, digit, **ε** |
| N  | digit |
| N' | digit, **ε** |
| T  | letter, digit  *(i.e. ID, NUMBER)* |
| H  | `+`, `-`, `*`, `/` |
| E' | `+`, `-`, `*`, `/`, **ε** |
| E  | letter, digit |
| C' | `<`, `=` |
| C  | letter, digit |
| O  | `end`, `else` |
| F  | `if` |
| R  | `repeat` |
| A  | letter |
| K  | `read` |
| W' | letter, `"` |
| W  | `write` |
| S  | `if`, `repeat`, letter, `read`, `write` |
| P  | `if`, `repeat`, letter, `read`, `write` |
| P' | `if`, `repeat`, letter, `read`, `write`, **ε** |

---

## 6. FOLLOW Sets

Start symbol is **P**, so `$` ∈ FOLLOW(P).

| Non-terminal | FOLLOW |
|---|---|
| P  | `$`, `end`, `until`, `else` |
| P' | `$`, `end`, `until`, `else` |
| S  | `;` |
| F  | `;` |
| O  | `;` |
| R  | `;` |
| A  | `;` |
| K  | `;` |
| W  | `;` |
| W' | `;` |
| C  | `then`, `;` |
| C' | `then`, `;` |
| E  | `<`, `=`, `then`, `;` |
| E' | `<`, `=`, `then`, `;` |
| T  | `+`, `-`, `*`, `/`, `<`, `=`, `then`, `;` |
| H  | letter, digit  *(= FIRST(E))* |
| D  | `+`, `-`, `*`, `/`, `<`, `=`, `then`, `;`, `:=` |
| D' | same as FOLLOW(D) |
| N  | `+`, `-`, `*`, `/`, `<`, `=`, `then`, `;` |
| N' | same as FOLLOW(N) |
| L  | letter, digit, FOLLOW(D) |
| G  | letter, digit, FOLLOW(D), FOLLOW(N) |

### Derivation notes

- **FOLLOW(P)**:
  - P appears at the end of `O -> else P end` → adds `end`
  - P appears in `R -> repeat P until C` → adds `until`
  - P appears in `F -> if C then P O` → adds FIRST(O) = {`end`, `else`}
  - P is the start symbol → adds `$`

- **FOLLOW(S) = {`;`}**: from `P -> S; P'`.

- **FOLLOW(C) = {`then`, `;`}**:
  - C in `F -> if C then P O` → `then`
  - C at end of `R -> repeat P until C` → FOLLOW(R) = `;`

- **FOLLOW(E) = {`<`, `=`, `then`, `;`}**:
  - E in `C -> E C'` → FIRST(C') = `<`, `=`
  - E at end of `C' -> < E` and `C' -> = E` → FOLLOW(C') = `then`, `;`
  - E at end of `A -> D := E` → FOLLOW(A) = `;`

- **FOLLOW(T) = FIRST(E') \ {ε} ∪ FOLLOW(E)** = `+`, `-`, `*`, `/`, `<`, `=`, `then`, `;`.

- **FOLLOW(H) = FIRST(E)** = letter, digit (since H is followed by E in `E' -> H E`).

---

## 7. The grammar is **LL(1)**

After both transformations:

- No left recursion.
- All alternatives of every non-terminal have **disjoint** FIRST sets, e.g.
  - `S`: `if` / `repeat` / letter / `read` / `write` — all distinct.
  - `O`: `end` vs `else` — distinct.
  - `C'`: `<` vs `=` — distinct.
  - `W'`: letter vs `"` — distinct.
- For each non-terminal `X` that derives ε, FIRST(X) ∩ FOLLOW(X) = ∅
  (e.g. FIRST(E') = {`+`,`-`,`*`,`/`} and FOLLOW(E') = {`<`,`=`,`then`,`;`}).

The grammar can therefore be parsed by **recursive-descent** with one
token of look-ahead — which is exactly what the practical part
(`Form1.h`) implements.
