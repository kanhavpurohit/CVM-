#include "parser.h"
#include <stdexcept>

Parser::Parser(std::vector<Token> toks) : tokens(std::move(toks)) {}

const Token& Parser::current() const { return tokens[pos]; }
const Token& Parser::peek(int offset) const {
    size_t i = pos + offset;
    return tokens[i < tokens.size() ? i : tokens.size()-1];
}
bool Parser::check(TokenType t) const { return current().type == t; }
bool Parser::match(TokenType t) {
    if (check(t)) { pos++; return true; }
    return false;
}
const Token& Parser::expect(TokenType t, const std::string& msg) {
    if (!check(t)) {
        throw std::runtime_error("Parse error (line " + std::to_string(current().line) +
                                 "): expected " + msg + " but got '" + current().value + "'");
    }
    return tokens[pos++];
}

NodePtr Parser::parseProgram() {
    auto block = std::make_unique<Block>();
    while (!check(TokenType::END_OF_FILE)) {
        block->stmts.push_back(parseStatement());
    }
    return block;
}

NodePtr Parser::parseStatement() {
    if (check(TokenType::LBRACE)) return parseBlock();
    if (check(TokenType::LET))    return parseVarDecl();
    if (check(TokenType::IF))     return parseIf();
    if (check(TokenType::WHILE))  return parseWhile();
    if (check(TokenType::PRINT))  return parsePrint();
    if (check(TokenType::INPUT))  return parseInput();
    return parseAssignOrExprStmt();
}

NodePtr Parser::parseBlock() {
    expect(TokenType::LBRACE, "'{'");
    auto block = std::make_unique<Block>();
    while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE)) {
        block->stmts.push_back(parseStatement());
    }
    expect(TokenType::RBRACE, "'}'");
    return block;
}

NodePtr Parser::parseVarDecl() {
    int ln = current().line;
    expect(TokenType::LET, "'let'");
    auto& nameTok = expect(TokenType::IDENTIFIER, "identifier");
    expect(TokenType::ASSIGN, "'='");
    auto value = parseExpression();
    expect(TokenType::SEMICOLON, "';'");
    auto node = std::make_unique<VarDecl>();
    node->name = nameTok.value;
    node->value = std::move(value);
    node->line = ln;
    return node;
}

NodePtr Parser::parseIf() {
    int ln = current().line;
    expect(TokenType::IF, "'if'");
    expect(TokenType::LPAREN, "'('");
    auto cond = parseExpression();
    expect(TokenType::RPAREN, "')'");
    auto thenB = parseBlock();
    NodePtr elseB = nullptr;
    if (match(TokenType::ELSE)) {
        if (check(TokenType::IF)) elseB = parseIf(); // else if chain
        else                       elseB = parseBlock();
    }
    auto node = std::make_unique<IfStmt>();
    node->cond = std::move(cond);
    node->thenBlock = std::move(thenB);
    node->elseBlock = std::move(elseB);
    node->line = ln;
    return node;
}

NodePtr Parser::parseWhile() {
    int ln = current().line;
    expect(TokenType::WHILE, "'while'");
    expect(TokenType::LPAREN, "'('");
    auto cond = parseExpression();
    expect(TokenType::RPAREN, "')'");
    auto body = parseBlock();
    auto node = std::make_unique<WhileStmt>();
    node->cond = std::move(cond);
    node->body = std::move(body);
    node->line = ln;
    return node;
}

NodePtr Parser::parsePrint() {
    int ln = current().line;
    expect(TokenType::PRINT, "'print'");
    bool paren = match(TokenType::LPAREN);
    auto value = parseExpression();
    if (paren) expect(TokenType::RPAREN, "')'");
    expect(TokenType::SEMICOLON, "';'");
    auto node = std::make_unique<PrintStmt>();
    node->value = std::move(value);
    node->line = ln;
    return node;
}

NodePtr Parser::parseInput() {
    int ln = current().line;
    expect(TokenType::INPUT, "'input'");
    auto& nameTok = expect(TokenType::IDENTIFIER, "identifier");
    expect(TokenType::SEMICOLON, "';'");
    auto node = std::make_unique<InputStmt>();
    node->name = nameTok.value;
    node->line = ln;
    return node;
}

NodePtr Parser::parseAssignOrExprStmt() {
    // assignment: IDENT '=' expr ';'
    if (check(TokenType::IDENTIFIER) && peek().type == TokenType::ASSIGN) {
        int ln = current().line;
        auto& nameTok = tokens[pos++];
        pos++; // consume '='
        auto value = parseExpression();
        expect(TokenType::SEMICOLON, "';'");
        auto node = std::make_unique<Assign>();
        node->name = nameTok.value;
        node->value = std::move(value);
        node->line = ln;
        return node;
    }
    int ln = current().line;
    auto expr = parseExpression();
    expect(TokenType::SEMICOLON, "';'");
    auto stmt = std::make_unique<ExprStmt>();
    stmt->expr = std::move(expr);
    stmt->line = ln;
    return stmt;
}

// ── Expression precedence climbing ─────────────────
NodePtr Parser::parseExpression() { return parseEquality(); }

NodePtr Parser::parseEquality() {
    auto left = parseComparison();
    while (check(TokenType::EQ) || check(TokenType::NEQ)) {
        std::string op = current().value;
        pos++;
        auto right = parseComparison();
        auto bin = std::make_unique<BinaryOp>();
        bin->op = op;
        bin->left = std::move(left);
        bin->right = std::move(right);
        left = std::move(bin);
    }
    return left;
}

NodePtr Parser::parseComparison() {
    auto left = parseTerm();
    while (check(TokenType::LT) || check(TokenType::GT) ||
           check(TokenType::LEQ) || check(TokenType::GEQ)) {
        std::string op = current().value;
        pos++;
        auto right = parseTerm();
        auto bin = std::make_unique<BinaryOp>();
        bin->op = op;
        bin->left = std::move(left);
        bin->right = std::move(right);
        left = std::move(bin);
    }
    return left;
}

NodePtr Parser::parseTerm() {
    auto left = parseFactor();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        std::string op = current().value;
        pos++;
        auto right = parseFactor();
        auto bin = std::make_unique<BinaryOp>();
        bin->op = op;
        bin->left = std::move(left);
        bin->right = std::move(right);
        left = std::move(bin);
    }
    return left;
}

NodePtr Parser::parseFactor() {
    auto left = parseUnary();
    while (check(TokenType::STAR) || check(TokenType::SLASH) || check(TokenType::MOD)) {
        std::string op = current().value;
        pos++;
        auto right = parseUnary();
        auto bin = std::make_unique<BinaryOp>();
        bin->op = op;
        bin->left = std::move(left);
        bin->right = std::move(right);
        left = std::move(bin);
    }
    return left;
}

NodePtr Parser::parseUnary() {
    if (check(TokenType::MINUS)) {
        pos++;
        auto operand = parseUnary();
        auto un = std::make_unique<UnaryOp>();
        un->op = "-";
        un->operand = std::move(operand);
        return un;
    }
    return parsePrimary();
}

NodePtr Parser::parsePrimary() {
    const Token& t = current();
    if (t.type == TokenType::NUMBER) {
        pos++;
        return std::make_unique<NumberLit>(std::stoi(t.value));
    }
    if (t.type == TokenType::TRUE)  { pos++; return std::make_unique<BoolLit>(true);  }
    if (t.type == TokenType::FALSE) { pos++; return std::make_unique<BoolLit>(false); }
    if (t.type == TokenType::IDENTIFIER) {
        pos++;
        return std::make_unique<Identifier>(t.value);
    }
    if (t.type == TokenType::LPAREN) {
        pos++;
        auto e = parseExpression();
        expect(TokenType::RPAREN, "')'");
        return e;
    }
    throw std::runtime_error("Parse error (line " + std::to_string(t.line) +
                             "): unexpected token '" + t.value + "'");
}
