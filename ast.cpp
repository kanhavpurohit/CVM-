#include "ast.h"

static void pad(int n) {
    for (int i = 0; i < n; ++i) std::cout << "  ";
}

void printAst(const Node* node, int indent) {
    if (!node) { pad(indent); std::cout << "<null>\n"; return; }

    if (auto* n = dynamic_cast<const NumberLit*>(node))   { pad(indent); std::cout << "Number(" << n->value << ")\n"; return; }
    if (auto* n = dynamic_cast<const BoolLit*>(node))     { pad(indent); std::cout << "Bool("  << (n->value?"true":"false") << ")\n"; return; }
    if (auto* n = dynamic_cast<const Identifier*>(node))  { pad(indent); std::cout << "Ident(" << n->name << ")\n"; return; }

    if (auto* n = dynamic_cast<const BinaryOp*>(node)) {
        pad(indent); std::cout << "BinaryOp(" << n->op << ")\n";
        printAst(n->left.get(),  indent+1);
        printAst(n->right.get(), indent+1);
        return;
    }
    if (auto* n = dynamic_cast<const UnaryOp*>(node)) {
        pad(indent); std::cout << "UnaryOp(" << n->op << ")\n";
        printAst(n->operand.get(), indent+1);
        return;
    }
    if (auto* n = dynamic_cast<const VarDecl*>(node)) {
        pad(indent); std::cout << "VarDecl(" << n->name << ")\n";
        printAst(n->value.get(), indent+1);
        return;
    }
    if (auto* n = dynamic_cast<const Assign*>(node)) {
        pad(indent); std::cout << "Assign(" << n->name << ")\n";
        printAst(n->value.get(), indent+1);
        return;
    }
    if (auto* n = dynamic_cast<const PrintStmt*>(node)) {
        pad(indent); std::cout << "Print\n";
        printAst(n->value.get(), indent+1);
        return;
    }
    if (auto* n = dynamic_cast<const InputStmt*>(node)) {
        pad(indent); std::cout << "Input(" << n->name << ")\n";
        return;
    }
    if (auto* n = dynamic_cast<const IfStmt*>(node)) {
        pad(indent); std::cout << "If\n";
        pad(indent+1); std::cout << "cond:\n"; printAst(n->cond.get(), indent+2);
        pad(indent+1); std::cout << "then:\n"; printAst(n->thenBlock.get(), indent+2);
        if (n->elseBlock) { pad(indent+1); std::cout << "else:\n"; printAst(n->elseBlock.get(), indent+2); }
        return;
    }
    if (auto* n = dynamic_cast<const WhileStmt*>(node)) {
        pad(indent); std::cout << "While\n";
        pad(indent+1); std::cout << "cond:\n"; printAst(n->cond.get(), indent+2);
        pad(indent+1); std::cout << "body:\n"; printAst(n->body.get(), indent+2);
        return;
    }
    if (auto* n = dynamic_cast<const Block*>(node)) {
        pad(indent); std::cout << "Block\n";
        for (auto& s : n->stmts) printAst(s.get(), indent+1);
        return;
    }
    if (auto* n = dynamic_cast<const ExprStmt*>(node)) {
        pad(indent); std::cout << "ExprStmt\n";
        printAst(n->expr.get(), indent+1);
        return;
    }
    pad(indent); std::cout << "<unknown node>\n";
}
