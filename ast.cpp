#include "ast.h"
#include <sstream>
#include <cstdio>

std::string jsonEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            case '\b': out += "\\b";  break;
            case '\f': out += "\\f";  break;
            default:
                if ((unsigned char)c < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof buf, "\\u%04x", (unsigned char)c);
                    out += buf;
                } else {
                    out += c;
                }
        }
    }
    return out;
}

static void jstr(std::ostream& o, const std::string& s) {
    o << '"' << jsonEscape(s) << '"';
}

void astToJson(const Node* node, std::ostream& o) {
    if (!node) { o << "null"; return; }
    if (auto* n = dynamic_cast<const NumberLit*>(node)) {
        o << "{\"type\":\"NumberLit\",\"value\":" << n->value << "}";
        return;
    }
    if (auto* n = dynamic_cast<const BoolLit*>(node)) {
        o << "{\"type\":\"BoolLit\",\"value\":" << (n->value ? "true" : "false") << "}";
        return;
    }
    if (auto* n = dynamic_cast<const Identifier*>(node)) {
        o << "{\"type\":\"Identifier\",\"name\":"; jstr(o, n->name); o << "}";
        return;
    }
    if (auto* n = dynamic_cast<const BinaryOp*>(node)) {
        o << "{\"type\":\"BinaryOp\",\"op\":"; jstr(o, n->op);
        o << ",\"children\":[";
        astToJson(n->left.get(), o); o << ",";
        astToJson(n->right.get(), o);
        o << "]}";
        return;
    }
    if (auto* n = dynamic_cast<const UnaryOp*>(node)) {
        o << "{\"type\":\"UnaryOp\",\"op\":"; jstr(o, n->op);
        o << ",\"children\":[";
        astToJson(n->operand.get(), o);
        o << "]}";
        return;
    }
    if (auto* n = dynamic_cast<const VarDecl*>(node)) {
        o << "{\"type\":\"VarDecl\",\"name\":"; jstr(o, n->name);
        o << ",\"children\":[";
        astToJson(n->value.get(), o);
        o << "]}";
        return;
    }
    if (auto* n = dynamic_cast<const Assign*>(node)) {
        o << "{\"type\":\"Assign\",\"name\":"; jstr(o, n->name);
        o << ",\"children\":[";
        astToJson(n->value.get(), o);
        o << "]}";
        return;
    }
    if (auto* n = dynamic_cast<const PrintStmt*>(node)) {
        o << "{\"type\":\"Print\",\"children\":[";
        astToJson(n->value.get(), o);
        o << "]}";
        return;
    }
    if (auto* n = dynamic_cast<const InputStmt*>(node)) {
        o << "{\"type\":\"Input\",\"name\":"; jstr(o, n->name); o << "}";
        return;
    }
    if (auto* n = dynamic_cast<const IfStmt*>(node)) {
        o << "{\"type\":\"If\",\"children\":[";
        astToJson(n->cond.get(), o); o << ",";
        astToJson(n->thenBlock.get(), o);
        if (n->elseBlock) { o << ","; astToJson(n->elseBlock.get(), o); }
        o << "]}";
        return;
    }
    if (auto* n = dynamic_cast<const WhileStmt*>(node)) {
        o << "{\"type\":\"While\",\"children\":[";
        astToJson(n->cond.get(), o); o << ",";
        astToJson(n->body.get(), o);
        o << "]}";
        return;
    }
    if (auto* n = dynamic_cast<const Block*>(node)) {
        o << "{\"type\":\"Block\",\"children\":[";
        for (size_t i = 0; i < n->stmts.size(); ++i) {
            if (i) o << ",";
            astToJson(n->stmts[i].get(), o);
        }
        o << "]}";
        return;
    }
    if (auto* n = dynamic_cast<const ExprStmt*>(node)) {
        o << "{\"type\":\"ExprStmt\",\"children\":[";
        astToJson(n->expr.get(), o);
        o << "]}";
        return;
    }
    o << "null";
}

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
