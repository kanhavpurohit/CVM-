#include "opcodes.h"
#include <iostream>
#include <iomanip>
#include <cstring>

const char* opName(OpCode op) {
    switch (op) {
        case OP_PUSH_INT:  return "PUSH_INT";
        case OP_PUSH_BOOL: return "PUSH_BOOL";
        case OP_LOAD:      return "LOAD";
        case OP_STORE:     return "STORE";
        case OP_ADD:       return "ADD";
        case OP_SUB:       return "SUB";
        case OP_MUL:       return "MUL";
        case OP_DIV:       return "DIV";
        case OP_MOD:       return "MOD";
        case OP_EQ:        return "EQ";
        case OP_NEQ:       return "NEQ";
        case OP_LT:        return "LT";
        case OP_GT:        return "GT";
        case OP_LEQ:       return "LEQ";
        case OP_GEQ:       return "GEQ";
        case OP_NEG:       return "NEG";
        case OP_PRINT:     return "PRINT";
        case OP_INPUT:     return "INPUT";
        case OP_JMP:       return "JMP";
        case OP_JMP_FALSE: return "JMP_FALSE";
        case OP_POP:       return "POP";
        case OP_HALT:      return "HALT";
    }
    return "??";
}

static uint16_t readU16(const std::vector<uint8_t>& code, size_t i) {
    return (uint16_t)code[i] | ((uint16_t)code[i+1] << 8);
}
static int32_t readI32(const std::vector<uint8_t>& code, size_t i) {
    int32_t v;
    std::memcpy(&v, &code[i], 4);
    return v;
}

void disassemble(const std::vector<uint8_t>& code,
                 const std::vector<std::string>& vars) {
    std::cout << "=== Bytecode (" << code.size() << " bytes, "
              << vars.size() << " vars) ===\n";
    for (size_t i = 0; i < code.size(); ) {
        std::cout << std::setw(4) << std::setfill('0') << i << "  ";
        OpCode op = (OpCode)code[i++];
        std::cout << std::setfill(' ') << std::left << std::setw(10) << opName(op);
        switch (op) {
            case OP_PUSH_INT: {
                int32_t v = readI32(code, i); i += 4;
                std::cout << v;
                break;
            }
            case OP_PUSH_BOOL: {
                std::cout << (code[i] ? "true" : "false");
                i += 1;
                break;
            }
            case OP_LOAD: case OP_STORE: case OP_INPUT: {
                uint16_t s = readU16(code, i); i += 2;
                std::cout << s;
                if (s < vars.size()) std::cout << "   ; " << vars[s];
                break;
            }
            case OP_JMP: case OP_JMP_FALSE: {
                uint16_t t = readU16(code, i); i += 2;
                std::cout << "-> " << t;
                break;
            }
            default: break;
        }
        std::cout << "\n";
    }
    std::cout << "=== end ===\n";
}
