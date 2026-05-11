#pragma once
#include "ast.h"
#include "opcodes.h"
#include <vector>
#include <string>
#include <cstdint>

class Compiler {
public:
    std::vector<uint8_t> compile(const Node* program);
    const std::vector<std::string>& getVars() const { return vars; }

private:
    std::vector<uint8_t>     code;
    std::vector<std::string> vars; // index = variable slot

    int getOrCreateVar(const std::string& name);
    int findVar(const std::string& name) const;

    void emit(uint8_t b);
    void emitU16(uint16_t v);
    void emitI32(int32_t v);
    size_t emitJump(OpCode op);          // returns the index of the operand to patch
    void   patchJump(size_t operandIdx); // patches to current code.size()

    void compileNode(const Node* node);
    void compileBinary(const class BinaryOp* node);
};
