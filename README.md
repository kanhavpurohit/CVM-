# CVM++

A tiny stack-based virtual machine for a toy C-like language, written in C++17, plus a FastAPI web visualizer that renders the compilation pipeline live.

## Pipeline

```
source.cvm → Lexer → Tokens → Parser → AST → Compiler → Bytecode → VM → Output
```

| Module       | Files                           | Job                                                |
|--------------|---------------------------------|----------------------------------------------------|
| Lexer        | `lexer.h/.cpp`                  | Source string → `Token` stream                     |
| Parser       | `parser.h/.cpp`, `ast.h/.cpp`   | Tokens → AST (recursive descent w/ precedence)     |
| Compiler     | `compiler.h/.cpp`, `opcodes.h`  | AST → flat `std::vector<uint8_t>` bytecode         |
| VM           | `vm.h/.cpp`                     | Stack-based dispatch loop over bytecode            |
| CLI          | `main.cpp`                      | File runner + REPL + debug + JSON flags            |
| Web          | `web/`                          | FastAPI server + browser visualizer                |

---

## Language reference

### Lexical rules
- **Identifiers**: start with letter or `_`, followed by letters/digits/underscores.
- **Numbers**: decimal integer literals (`0`, `42`, `1234`).
- **Booleans**: keywords `true` and `false`.
- **Comments**: `//` to end of line.
- **Whitespace**: ignored.
- **Reserved keywords**: `let`, `if`, `else`, `while`, `print`, `input`, `true`, `false`, `fn`, `return` (last two reserved; not yet implemented).

### Types
| Type    | Literals          | Notes                                  |
|---------|-------------------|----------------------------------------|
| int     | `0`, `42`, `-7`   | 32-bit signed                          |
| bool    | `true`, `false`   | result of comparisons                  |

### Operators (precedence, lowest → highest)

| Level | Operators            | Description           | Associativity |
|-------|----------------------|-----------------------|---------------|
| 1     | `==` `!=`            | equality              | left          |
| 2     | `<` `>` `<=` `>=`    | comparison            | left          |
| 3     | `+` `-`              | addition/subtraction  | left          |
| 4     | `*` `/` `%`          | multiplication/divide | left          |
| 5     | unary `-`            | negation              | right         |
| 6     | `( expr )`           | grouping              | —             |

### Grammar (EBNF)

```ebnf
program     = { statement } ;

statement   = varDecl
            | assignment
            | ifStmt
            | whileStmt
            | printStmt
            | inputStmt
            | block
            | exprStmt ;

varDecl     = "let" IDENT "=" expression ";" ;
assignment  = IDENT "=" expression ";" ;
ifStmt      = "if" "(" expression ")" block [ "else" ( ifStmt | block ) ] ;
whileStmt   = "while" "(" expression ")" block ;
printStmt   = "print" [ "(" ] expression [ ")" ] ";" ;
inputStmt   = "input" IDENT ";" ;
block       = "{" { statement } "}" ;
exprStmt    = expression ";" ;

expression  = equality ;
equality    = comparison { ( "==" | "!=" ) comparison } ;
comparison  = term       { ( "<" | ">" | "<=" | ">=" ) term } ;
term        = factor     { ( "+" | "-" ) factor } ;
factor      = unary      { ( "*" | "/" | "%" ) unary } ;
unary       = "-" unary | primary ;
primary     = NUMBER
            | "true" | "false"
            | IDENT
            | "(" expression ")" ;
```

### Examples

```c
// Declaration + arithmetic
let x = 10;
let y = 32;
print x + y;        // 42

// Booleans + comparisons
print 5 < 10;       // true
print 7 == 8;       // false

// Conditional
if (x > y) {
    print x;
} else {
    print y;
}

// Loop
let i = 1;
let sum = 0;
while (i <= 10) {
    sum = sum + i;
    i = i + 1;
}
print sum;          // 55

// Input
input n;
print n * 2;
```

---

## ISA (Instruction Set Architecture)

Each opcode is **1 byte**, optionally followed by inline operand bytes (little-endian).

| Opcode       | Operand bytes         | Stack effect / behavior                       |
|--------------|-----------------------|-----------------------------------------------|
| `PUSH_INT`   | int32                 | push integer                                  |
| `PUSH_BOOL`  | uint8                 | push boolean                                  |
| `LOAD`       | uint16 slot           | push variable value                           |
| `STORE`      | uint16 slot           | pop, store into variable                      |
| `ADD..MOD`   | —                     | pop 2 ints, push int                          |
| `EQ..GEQ`    | —                     | pop 2, push bool                              |
| `NEG`        | —                     | pop int, push negated int                     |
| `PRINT`      | —                     | pop, print to stdout                          |
| `INPUT`      | uint16 slot           | read int from stdin into variable             |
| `JMP`        | uint16 target         | unconditional jump                            |
| `JMP_FALSE`  | uint16 target         | pop; jump if value is false/zero              |
| `POP`        | —                     | pop and discard                               |
| `HALT`       | —                     | stop the VM                                   |

---

## Building the VM

You need **CMake** and a **C++17 compiler** (MSVC, g++, or clang++).

```powershell
.\build.ps1            # PowerShell — Release build, copies cvm.exe to project root
```
```bash
./build.sh             # Bash — same thing
```

Then:

```powershell
.\cvm.exe                            # REPL
.\cvm.exe examples\loop.cvm          # run a script (prints 55)
.\cvm.exe --debug examples\loop.cvm  # show AST + bytecode + run
.\cvm.exe --json  examples\loop.cvm  # emit JSON (used by web visualizer)
```

---

## Web visualizer

A FastAPI server with a CodeMirror editor and a D3 parse-tree renderer.

### Run it

1. **Build the VM first** (see above) — the web server shells out to `cvm.exe`.
2. **Start the server**:

   ```powershell
   .\web\run.ps1        # PowerShell (Windows)
   ```
   ```bash
   ./web/run.sh         # Bash (macOS/Linux/Git Bash)
   ```

   First run creates a `.venv`, installs FastAPI/Uvicorn, then launches Uvicorn on port 8000.

3. Open **http://127.0.0.1:8000** in your browser.

### What you see

- **Source pane** — CodeMirror editor (Ctrl/Cmd+Enter to run). Buttons load sample programs.
- **AST tab** — the parse tree, rendered with D3. Drag to pan, scroll to zoom, double-click background to reset.
- **Tokens tab** — every token the lexer produced.
- **Bytecode tab** — the compiled instructions with addresses, opcodes, and operands.
- **Output tab** — what the VM printed.
- **stdin field** — values consumed by `input` statements (space-separated).

If a stage fails (lexer/parser/compiler/vm error), a red banner shows which stage and why.

---

## Project layout

```
.
├── lexer.{h,cpp}        # Tokenizer
├── parser.{h,cpp}       # Recursive-descent parser
├── ast.{h,cpp}          # AST node types + debug & JSON printers
├── opcodes.{h,cpp}      # ISA + disassembler + JSON disasm
├── compiler.{h,cpp}     # AST → bytecode
├── vm.{h,cpp}           # Stack-based execution loop
├── main.cpp             # CLI entry point (REPL / file / JSON modes)
├── CMakeLists.txt
├── build.{ps1,sh}       # Build scripts
├── examples/*.cvm       # Sample programs
└── web/
    ├── app.py           # FastAPI server
    ├── requirements.txt
    ├── run.{ps1,sh}     # Server launch scripts
    └── static/          # index.html, style.css, app.js
```
