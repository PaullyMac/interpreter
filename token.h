#ifndef TOKEN_H
#define TOKEN_H

#endif //TOKEN_H

typedef enum
{
    // Tokens composed of characters exclusively part of single-character tokens
    LEFT_PARENTHESIS,
    RIGHT_PARENTHESIS,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    LEFT_BRACE,
    RIGHT_BRACE,
    COMMA,
    SEMICOLON,

    // Possible multi-character tokens
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    EXPONENT,
    INCREMENT,
    DECREMENT,
    EQUAL,
    NOT_EQUAL,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
    BANG,
    OR,
    AND,
    COMMENT,

    // Tokens using the same characters
    MODULO,
    FORMAT,

    // Literals
    IDENTIFIER,
    STRING,
    INTEGER_NUMBER,
    FLOAT_NUMBER,

    // Keywords
    CHAR,
    INT,
    FLOAT,
    BOOL,
    IF,
    ELSE,
    FOR,
    WHILE,
    RETURN,
    PRINTF,
    SCANF,
    TRUE,
    FALSE
} TokenType;