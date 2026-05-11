#pragma once
#include "lexer.h"
#include "ast.h"

class Parser {
public:
    explicit Parser(std::vector<Token> toks);
    NodePtr parseProgram(); // returns a Block

private:
    std::vector<Token> tokens;
    size_t pos = 0;

    const Token& current() const;
    const Token& peek(int offset = 1) const;
    bool check(TokenType t) const;
    bool match(TokenType t);
    const Token& expect(TokenType t, const std::string& msg);

    // statements
    NodePtr parseStatement();
    NodePtr parseBlock();
    NodePtr parseVarDecl();
    NodePtr parseIf();
    NodePtr parseWhile();
    NodePtr parsePrint();
    NodePtr parseInput();
    NodePtr parseAssignOrExprStmt();

    // expressions (precedence climbing)
    NodePtr parseExpression();
    NodePtr parseEquality();
    NodePtr parseComparison();
    NodePtr parseTerm();      // + -
    NodePtr parseFactor();    // * / %
    NodePtr parseUnary();
    NodePtr parsePrimary();
};
