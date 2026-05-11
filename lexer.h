#pragma once
#include <string>
#include <vector>

// ── Token Types ────────────────────────────────────
// Every token the Lexer can produce.
// The Parser (Part 2) will match on these.
enum class TokenType {
    NUMBER, STRING, IDENTIFIER,
    LET, IF, ELSE, WHILE, FN, RETURN, PRINT, INPUT,
    TRUE, FALSE,
    PLUS, MINUS, STAR, SLASH, MOD,
    ASSIGN, EQ, NEQ, LT, GT, LEQ, GEQ,
    LPAREN, RPAREN, LBRACE, RBRACE,
    SEMICOLON, COMMA,
    END_OF_FILE
};

// ── Token ───────────────────────────────────────────
// This struct is the JIGSAW PIECE that connects
// Part 1 (Lexer) → Part 2 (Parser).
struct Token {
    TokenType type;
    std::string value;
    int line;
};

// ── Lexer class ─────────────────────────────────────
class Lexer {
public:
    explicit Lexer(const std::string& source);
    std::vector<Token> tokenize(); // ← Parser calls this

private:
    std::string source;
    size_t pos;
    int    line;

    char current() const;
    char peek()    const;
    void advance();
    void skipWhitespaceAndComments();
    Token readNumber();
    Token readString();
    Token readIdentifierOrKeyword();
};