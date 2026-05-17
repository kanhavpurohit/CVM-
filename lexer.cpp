#include "lexer.h"
#include <stdexcept>
#include <unordered_map>

static const std::unordered_map<std::string, TokenType> KEYWORDS = {
    {"let",    TokenType::LET},    {"if",     TokenType::IF},
    {"else",   TokenType::ELSE},   {"while",  TokenType::WHILE},
    {"fn",     TokenType::FN},     {"return", TokenType::RETURN},
    {"print",  TokenType::PRINT},  {"input",  TokenType::INPUT},
    {"true",   TokenType::TRUE},   {"false",  TokenType::FALSE}
};

const char* tokenTypeName(TokenType t) {
    switch (t) {
        case TokenType::NUMBER:      return "NUMBER";
        case TokenType::STRING:      return "STRING";
        case TokenType::IDENTIFIER:  return "IDENTIFIER";
        case TokenType::LET:         return "LET";
        case TokenType::IF:          return "IF";
        case TokenType::ELSE:        return "ELSE";
        case TokenType::WHILE:       return "WHILE";
        case TokenType::FN:          return "FN";
        case TokenType::RETURN:      return "RETURN";
        case TokenType::PRINT:       return "PRINT";
        case TokenType::INPUT:       return "INPUT";
        case TokenType::TRUE:        return "TRUE";
        case TokenType::FALSE:       return "FALSE";
        case TokenType::PLUS:        return "PLUS";
        case TokenType::MINUS:       return "MINUS";
        case TokenType::STAR:        return "STAR";
        case TokenType::SLASH:       return "SLASH";
        case TokenType::MOD:         return "MOD";
        case TokenType::ASSIGN:      return "ASSIGN";
        case TokenType::EQ:          return "EQ";
        case TokenType::NEQ:         return "NEQ";
        case TokenType::LT:          return "LT";
        case TokenType::GT:          return "GT";
        case TokenType::LEQ:         return "LEQ";
        case TokenType::GEQ:         return "GEQ";
        case TokenType::LPAREN:      return "LPAREN";
        case TokenType::RPAREN:      return "RPAREN";
        case TokenType::LBRACE:      return "LBRACE";
        case TokenType::RBRACE:      return "RBRACE";
        case TokenType::SEMICOLON:   return "SEMICOLON";
        case TokenType::COMMA:       return "COMMA";
        case TokenType::END_OF_FILE: return "EOF";
    }
    return "?";
}

Lexer::Lexer(const std::string& src) : source(src), pos(0), line(1) {}

char Lexer::current() const { return pos < source.size() ? source[pos] : '\0'; }
char Lexer::peek()    const { return pos+1 < source.size() ? source[pos+1] : '\0'; }
void Lexer::advance() {
    if (current() == '\n') line++;
    pos++;
}

void Lexer::skipWhitespaceAndComments() {
    while (true) {
        while (isspace(current())) advance();
        // Skip // line comments
        if (current() == '/' && peek() == '/') {
            while (current() != '\n' && current() != '\0') advance();
        } else break;
    }
}

Token Lexer::readNumber() {
    std::string val;
    while (isdigit(current()) || current() == '.') { val += current(); advance(); }
    return {TokenType::NUMBER, val, line};
}

Token Lexer::readString() {
    advance(); // skip opening "
    std::string val;
    while (current() != '"' && current() != '\0') { val += current(); advance(); }
    advance(); // skip closing "
    return {TokenType::STRING, val, line};
}

Token Lexer::readIdentifierOrKeyword() {
    std::string val;
    while (isalnum(current()) || current() == '_') { val += current(); advance(); }
    auto it = KEYWORDS.find(val);
    TokenType t = (it != KEYWORDS.end()) ? it->second : TokenType::IDENTIFIER;
    return {t, val, line};
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        skipWhitespaceAndComments();
        char c = current();
        if (c == '\0') { tokens.push_back({TokenType::END_OF_FILE, "", line}); break; }
        if (isdigit(c))           { tokens.push_back(readNumber()); continue; }
        if (c == '"')             { tokens.push_back(readString()); continue; }
        if (isalpha(c) || c=='_'){ tokens.push_back(readIdentifierOrKeyword()); continue; }
        Token t{{}, std::string(1,c), line};
        switch (c) {
            case '+': t.type=TokenType::PLUS;      break;
            case '-': t.type=TokenType::MINUS;     break;
            case '*': t.type=TokenType::STAR;      break;
            case '/': t.type=TokenType::SLASH;     break;
            case '%': t.type=TokenType::MOD;       break;
            case '(': t.type=TokenType::LPAREN;    break;
            case ')': t.type=TokenType::RPAREN;    break;
            case '{': t.type=TokenType::LBRACE;    break;
            case '}': t.type=TokenType::RBRACE;    break;
            case ';': t.type=TokenType::SEMICOLON; break;
            case ',': t.type=TokenType::COMMA;     break;
            case '=': if(peek()=='='){ t={TokenType::EQ,"==",line}; advance(); }
                      else t.type=TokenType::ASSIGN; break;
            case '!': if(peek()=='='){ t={TokenType::NEQ,"!=",line}; advance(); }
                      else throw std::runtime_error("Unexpected '!'"); break;
            case '<': if(peek()=='='){ t={TokenType::LEQ,"<=",line}; advance(); }
                      else t.type=TokenType::LT; break;
            case '>': if(peek()=='='){ t={TokenType::GEQ,">=",line}; advance(); }
                      else t.type=TokenType::GT; break;
            default: throw std::runtime_error(std::string("Unknown char: ")+c);
        }
        advance();
        tokens.push_back(t);
    }
    return tokens;
}