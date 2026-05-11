# CVM++

A tiny stack-based virtual machine for a toy C-like language, written in C++17.

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
| CLI          | `main.cpp`                      | File runner + REPL + debug flags                   |

## Language

```
let x = 10;
let y = 5;
print x + y;

if (x > y) {
    print true;
} else {
    print false;
}

let i = 0;
while (i < 3) {
    print i;
    i = i + 1;
}

input n;
print n * 2;
```

Types: integers, booleans (`true`/`false`).
Ops: `+ - * / % == != < > <= >=`, unary `-`.
Statements: `let`, assignment, `if`/`else`, `while`, `print`, `input`, blocks `{ }`.
Comments: `// ...`.

## ISA

| Opcode       | Operand               | Effect                                       |
|--------------|-----------------------|----------------------------------------------|
| `PUSH_INT`   | int32                 | Push integer                                 |
| `PUSH_BOOL`  | uint8                 | Push boolean                                 |
| `LOAD`       | uint16 slot           | Push variable value                          |
| `STORE`      | uint16 slot           | Pop, store into variable                     |
| `ADD..MOD`   | —                     | Pop 2 ints, push int                         |
| `EQ..GEQ`    | —                     | Pop 2, push bool                             |
| `NEG`        | —                     | Pop int, push -int                           |
| `PRINT`      | —                     | Pop, print to stdout                         |
| `INPUT`      | uint16 slot           | Read int from stdin into variable            |
| `JMP`        | uint16 target         | Unconditional jump                           |
| `JMP_FALSE`  | uint16 target         | Pop, jump if value is false/0                |
| `POP`        | —                     | Pop and discard                              |
| `HALT`       | —                     | Stop the VM                                  |

## Build

```bash
cmake -B build
cmake --build build
```

## Run

```bash
./build/cvm                          # REPL
./build/cvm examples/loop.cvm        # run a script
./build/cvm --debug examples/loop.cvm # show AST + bytecode then run
./build/cvm --ast examples/hello.cvm  # AST only
./build/cvm --bc  examples/hello.cvm  # bytecode only
./build/cvm --trace examples/loop.cvm # trace dispatch
```
