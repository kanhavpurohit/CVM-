#pragma once
#include <cstdint>
#include <vector>
#include <string>

// ── CVM++ Instruction Set Architecture ─────────────
// Stack-based VM. Each opcode is 1 byte, possibly
// followed by inline operand bytes (little-endian).
enum OpCode : uint8_t {
    OP_PUSH_INT,    // + int32 operand              -> pushes integer
    OP_PUSH_BOOL,   // + uint8 operand (0/1)        -> pushes boolean
    OP_LOAD,        // + uint16 var-slot            -> pushes value of variable
    OP_STORE,       // + uint16 var-slot            -> pops value, stores to var
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,    // pop 2 ints, push int
    OP_EQ, OP_NEQ, OP_LT, OP_GT, OP_LEQ, OP_GEQ, // pop 2,  push bool
    OP_NEG,                                     // pop int, push -int
    OP_PRINT,                                   // pop and print value
    OP_INPUT,       // + uint16 var-slot            -> read int from stdin into var
    OP_JMP,         // + uint16 absolute target     -> unconditional jump
    OP_JMP_FALSE,   // + uint16 absolute target     -> pop, jump if false
    OP_POP,                                     // pop and discard
    OP_HALT
};

const char* opName(OpCode op);

// Disassembles bytecode to stdout for debugging.
void disassemble(const std::vector<uint8_t>& code,
                 const std::vector<std::string>& vars);

// JSON disassembly (for the web visualizer).
#include <iosfwd>
void bytecodeToJson(const std::vector<uint8_t>& code,
                    const std::vector<std::string>& vars,
                    std::ostream& out);
