#ifndef TOKEN_H
#define TOKEN_H

// Just set some arbitrary limits
#define MAX_LEXEME_LENGTH 500
#define MAX_TOKENS 500

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
    MULTIPLY,
    EXPONENT,
    AMPERSAND,

    // Possible multi-character tokens
    PLUS,
    MINUS,
    DIVIDE,
    EQUAL,
    NOT_EQUAL,
    ASSIGN,
    LESS,
    LESS_EQUAL,
    GREATER,
    GREATER_EQUAL,
    NOT,
    OR,
    AND,
    COMMENT,

    // Tokens using the same characters
    MODULO,

    // Literals
    IDENTIFIER,
    STRING,
    INTEGER_LITERAL,
    FLOAT_LITERAL,
    CHARACTER_LITERAL,
    
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
    FALSE,
    VOID,

    // Error tokens, not part of the language just for the scanner
    ERROR_INVALID_CHARACTER,
    ERROR_INVALID_IDENTIFIER,

    // End of file token
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[MAX_LEXEME_LENGTH];
    int line_number;
    int column_number;
} Token;

// Token names array
char* token_names[] = {
    "LEFT_PARENTHESIS",
    "RIGHT_PARENTHESIS",
    "LEFT_BRACKET",
    "RIGHT_BRACKET",
    "LEFT_BRACE",
    "RIGHT_BRACE",
    "COMMA",
    "SEMICOLON",
    "MULTIPLY",
    "EXPONENT",
    "AMPERSAND",
    "PLUS",
    "MINUS",
    "DIVIDE",
    "EQUAL",
    "NOT_EQUAL",
    "ASSIGN",
    "LESS",
    "LESS_EQUAL",
    "GREATER",
    "GREATER_EQUAL",
    "NOT",
    "OR",
    "AND",
    "COMMENT",
    "MODULO",
    "IDENTIFIER",
    "STRING",
    "INTEGER_LITERAL",
    "FLOAT_LITERAL",
    "CHARACTER_LITERAL",
    "CHAR",
    "INT",
    "FLOAT",
    "BOOL",
    "IF",
    "ELSE",
    "FOR",
    "WHILE",
    "RETURN",
    "PRINTF",
    "SCANF",
    "TRUE",
    "FALSE",
    "VOID",
    "ERROR_INVALID_CHARACTER",
    "ERROR_INVALID_IDENTIFIER",
    "TOKEN_EOF"
};

#endif //TOKEN_H