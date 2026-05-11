#include "vm.h"
#include "opcodes.h"
#include <iostream>
#include <stdexcept>
#include <cstring>

static uint16_t readU16(const std::vector<uint8_t>& code, size_t i) {
    return (uint16_t)code[i] | ((uint16_t)code[i+1] << 8);
}
static int32_t readI32(const std::vector<uint8_t>& code, size_t i) {
    int32_t v;
    std::memcpy(&v, &code[i], 4);
    return v;
}

static void printValue(const Value& v) {
    if (v.type == Value::BOOL) std::cout << (v.i ? "true" : "false");
    else                       std::cout << v.i;
}

Value VM::run(const std::vector<uint8_t>& code, size_t numVars, bool trace) {
    stack.clear();
    globals.assign(numVars, Value::mkInt(0));
    Value last = Value::mkInt(0);

    size_t ip = 0;
    while (ip < code.size()) {
        OpCode op = (OpCode)code[ip++];
        if (trace) std::cerr << "[ip=" << (ip-1) << "] " << opName(op)
                             << "  stack=" << stack.size() << "\n";
        switch (op) {
            case OP_PUSH_INT: {
                int32_t v = readI32(code, ip); ip += 4;
                stack.push_back(Value::mkInt(v));
                break;
            }
            case OP_PUSH_BOOL: {
                stack.push_back(Value::mkBool(code[ip] != 0));
                ip += 1;
                break;
            }
            case OP_LOAD: {
                uint16_t s = readU16(code, ip); ip += 2;
                stack.push_back(globals[s]);
                break;
            }
            case OP_STORE: {
                uint16_t s = readU16(code, ip); ip += 2;
                if (stack.empty()) throw std::runtime_error("VM: stack underflow on STORE");
                globals[s] = stack.back();
                last = stack.back();
                stack.pop_back();
                break;
            }
            case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: case OP_MOD: {
                if (stack.size() < 2) throw std::runtime_error("VM: stack underflow on arith");
                Value b = stack.back(); stack.pop_back();
                Value a = stack.back(); stack.pop_back();
                int32_t r = 0;
                switch (op) {
                    case OP_ADD: r = a.i + b.i; break;
                    case OP_SUB: r = a.i - b.i; break;
                    case OP_MUL: r = a.i * b.i; break;
                    case OP_DIV:
                        if (b.i == 0) throw std::runtime_error("VM: division by zero");
                        r = a.i / b.i; break;
                    case OP_MOD:
                        if (b.i == 0) throw std::runtime_error("VM: modulo by zero");
                        r = a.i % b.i; break;
                    default: break;
                }
                stack.push_back(Value::mkInt(r));
                break;
            }
            case OP_EQ: case OP_NEQ: case OP_LT: case OP_GT: case OP_LEQ: case OP_GEQ: {
                if (stack.size() < 2) throw std::runtime_error("VM: stack underflow on cmp");
                Value b = stack.back(); stack.pop_back();
                Value a = stack.back(); stack.pop_back();
                bool r = false;
                switch (op) {
                    case OP_EQ:  r = a.i == b.i; break;
                    case OP_NEQ: r = a.i != b.i; break;
                    case OP_LT:  r = a.i <  b.i; break;
                    case OP_GT:  r = a.i >  b.i; break;
                    case OP_LEQ: r = a.i <= b.i; break;
                    case OP_GEQ: r = a.i >= b.i; break;
                    default: break;
                }
                stack.push_back(Value::mkBool(r));
                break;
            }
            case OP_NEG: {
                if (stack.empty()) throw std::runtime_error("VM: stack underflow on NEG");
                Value a = stack.back(); stack.pop_back();
                stack.push_back(Value::mkInt(-a.i));
                break;
            }
            case OP_PRINT: {
                if (stack.empty()) throw std::runtime_error("VM: stack underflow on PRINT");
                Value v = stack.back(); stack.pop_back();
                printValue(v);
                std::cout << "\n";
                last = v;
                break;
            }
            case OP_INPUT: {
                uint16_t s = readU16(code, ip); ip += 2;
                int32_t v;
                if (!(std::cin >> v)) throw std::runtime_error("VM: input read failed");
                globals[s] = Value::mkInt(v);
                break;
            }
            case OP_JMP: {
                uint16_t t = readU16(code, ip);
                ip = t;
                break;
            }
            case OP_JMP_FALSE: {
                uint16_t t = readU16(code, ip); ip += 2;
                if (stack.empty()) throw std::runtime_error("VM: stack underflow on JMP_FALSE");
                Value v = stack.back(); stack.pop_back();
                if (!v.truthy()) ip = t;
                break;
            }
            case OP_POP: {
                if (stack.empty()) throw std::runtime_error("VM: stack underflow on POP");
                last = stack.back();
                stack.pop_back();
                break;
            }
            case OP_HALT:
                return last;
            default:
                throw std::runtime_error("VM: unknown opcode " + std::to_string((int)op));
        }
    }
    return last;
}
