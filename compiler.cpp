#include "compiler.h"
#include <stdexcept>
#include <cstring>

int Compiler::getOrCreateVar(const std::string& name) {
    for (size_t i = 0; i < vars.size(); ++i)
        if (vars[i] == name) return (int)i;
    vars.push_back(name);
    return (int)vars.size() - 1;
}
int Compiler::findVar(const std::string& name) const {
    for (size_t i = 0; i < vars.size(); ++i)
        if (vars[i] == name) return (int)i;
    return -1;
}

void Compiler::emit(uint8_t b)         { code.push_back(b); }
void Compiler::emitU16(uint16_t v)     { code.push_back(v & 0xFF); code.push_back((v >> 8) & 0xFF); }
void Compiler::emitI32(int32_t v) {
    uint8_t buf[4];
    std::memcpy(buf, &v, 4);
    for (int i = 0; i < 4; ++i) code.push_back(buf[i]);
}

size_t Compiler::emitJump(OpCode op) {
    emit(op);
    size_t idx = code.size();
    emitU16(0xFFFF); // placeholder
    return idx;
}
void Compiler::patchJump(size_t operandIdx) {
    uint16_t target = (uint16_t)code.size();
    code[operandIdx]     = target & 0xFF;
    code[operandIdx + 1] = (target >> 8) & 0xFF;
}

std::vector<uint8_t> Compiler::compile(const Node* program) {
    code.clear();
    vars.clear();
    compileNode(program);
    emit(OP_HALT);
    return std::move(code);
}

void Compiler::compileBinary(const BinaryOp* n) {
    compileNode(n->left.get());
    compileNode(n->right.get());
    const std::string& op = n->op;
    if      (op == "+")  emit(OP_ADD);
    else if (op == "-")  emit(OP_SUB);
    else if (op == "*")  emit(OP_MUL);
    else if (op == "/")  emit(OP_DIV);
    else if (op == "%")  emit(OP_MOD);
    else if (op == "==") emit(OP_EQ);
    else if (op == "!=") emit(OP_NEQ);
    else if (op == "<")  emit(OP_LT);
    else if (op == ">")  emit(OP_GT);
    else if (op == "<=") emit(OP_LEQ);
    else if (op == ">=") emit(OP_GEQ);
    else throw std::runtime_error("Unknown binary op: " + op);
}

void Compiler::compileNode(const Node* node) {
    if (auto* n = dynamic_cast<const NumberLit*>(node)) {
        emit(OP_PUSH_INT); emitI32(n->value); return;
    }
    if (auto* n = dynamic_cast<const BoolLit*>(node)) {
        emit(OP_PUSH_BOOL); emit(n->value ? 1 : 0); return;
    }
    if (auto* n = dynamic_cast<const Identifier*>(node)) {
        int slot = findVar(n->name);
        if (slot < 0) throw std::runtime_error("Undefined variable: " + n->name);
        emit(OP_LOAD); emitU16((uint16_t)slot); return;
    }
    if (auto* n = dynamic_cast<const BinaryOp*>(node)) { compileBinary(n); return; }
    if (auto* n = dynamic_cast<const UnaryOp*>(node)) {
        compileNode(n->operand.get());
        if (n->op == "-") emit(OP_NEG);
        else throw std::runtime_error("Unknown unary op: " + n->op);
        return;
    }
    if (auto* n = dynamic_cast<const VarDecl*>(node)) {
        compileNode(n->value.get());
        int slot = getOrCreateVar(n->name);
        emit(OP_STORE); emitU16((uint16_t)slot);
        return;
    }
    if (auto* n = dynamic_cast<const Assign*>(node)) {
        int slot = findVar(n->name);
        if (slot < 0) throw std::runtime_error("Undefined variable: " + n->name);
        compileNode(n->value.get());
        emit(OP_STORE); emitU16((uint16_t)slot);
        return;
    }
    if (auto* n = dynamic_cast<const PrintStmt*>(node)) {
        compileNode(n->value.get());
        emit(OP_PRINT);
        return;
    }
    if (auto* n = dynamic_cast<const InputStmt*>(node)) {
        int slot = getOrCreateVar(n->name);
        emit(OP_INPUT); emitU16((uint16_t)slot);
        return;
    }
    if (auto* n = dynamic_cast<const IfStmt*>(node)) {
        compileNode(n->cond.get());
        size_t jElse = emitJump(OP_JMP_FALSE);
        compileNode(n->thenBlock.get());
        if (n->elseBlock) {
            size_t jEnd = emitJump(OP_JMP);
            patchJump(jElse);
            compileNode(n->elseBlock.get());
            patchJump(jEnd);
        } else {
            patchJump(jElse);
        }
        return;
    }
    if (auto* n = dynamic_cast<const WhileStmt*>(node)) {
        size_t loopStart = code.size();
        compileNode(n->cond.get());
        size_t jExit = emitJump(OP_JMP_FALSE);
        compileNode(n->body.get());
        emit(OP_JMP); emitU16((uint16_t)loopStart);
        patchJump(jExit);
        return;
    }
    if (auto* n = dynamic_cast<const Block*>(node)) {
        for (auto& s : n->stmts) compileNode(s.get());
        return;
    }
    if (auto* n = dynamic_cast<const ExprStmt*>(node)) {
        compileNode(n->expr.get());
        emit(OP_POP); // discard the unused value
        return;
    }
    throw std::runtime_error("Compiler: unknown AST node");
}
