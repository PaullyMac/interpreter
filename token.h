#ifndef TOKEN_H
#define TOKEN_H

// Just set some arbitrary limits
#define MAX_LEXEME_LENGTH 500
#define MAX_TOKENS 500

// Moved from keywords_fsm.h
typedef enum {
    S,        // Start
    B, BO, BOO, FINAL_BOOL, // States for 'bool'
    C, CH, CHA, FINAL_CHAR, // States for 'char'
    E, EL, ELS, FINAL_ELSE, // States for 'else'
    F, FA, FAL, FALS, FINAL_FALSE, FL, FLO, FLOA, FINAL_FLOAT, FO, FINAL_FOR, // States for 'false', 'float', 'for'
    I, IN, FINAL_IF, FINAL_INT, // States for 'if', 'int'
    P, PR, PRI, PRIN, PRINT, FINAL_PRINTF, // States for 'printf'
    R, RE, RET, RETU, RETUR, FINAL_RETURN, // States for 'return'
    S1, SC, SCA, SCAN, FINAL_SCANF, // States for 'scanf'
    T, TR, TRU, FINAL_TRUE, // States for 'true'
    W, WH, WHI, WHIL, FINAL_WHILE, // States for 'while'
    V, VO, VOI, FINAL_VOID // States for 'void'
} KeywordState;

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
    "left_parenthesis",
    "right_parenthesis",
    "left_bracket",
    "right_bracket",
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