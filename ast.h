#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>

// ── AST Node Hierarchy ─────────────────────────────
// The Parser produces these; the Compiler walks them.

struct Node {
    int line = 0;
    virtual ~Node() = default;
};
using NodePtr = std::unique_ptr<Node>;

// ── Expressions ────────────────────────────────────
struct NumberLit : Node {
    int value;
    explicit NumberLit(int v) : value(v) {}
};

struct BoolLit : Node {
    bool value;
    explicit BoolLit(bool v) : value(v) {}
};

struct Identifier : Node {
    std::string name;
    explicit Identifier(std::string n) : name(std::move(n)) {}
};

struct BinaryOp : Node {
    std::string op;
    NodePtr left, right;
};

struct UnaryOp : Node {
    std::string op;
    NodePtr operand;
};

// ── Statements ─────────────────────────────────────
struct VarDecl : Node {
    std::string name;
    NodePtr value;
};

struct Assign : Node {
    std::string name;
    NodePtr value;
};

struct PrintStmt : Node {
    NodePtr value;
};

struct InputStmt : Node {
    std::string name;
};

struct IfStmt : Node {
    NodePtr cond;
    NodePtr thenBlock;
    NodePtr elseBlock; // may be null
};

struct WhileStmt : Node {
    NodePtr cond;
    NodePtr body;
};

struct Block : Node {
    std::vector<NodePtr> stmts;
};

struct ExprStmt : Node {
    NodePtr expr;
};

// ── Debug printer ──────────────────────────────────
void printAst(const Node* node, int indent = 0);
