#ifndef TOKEN_H
#define TOKEN_H

// Just set some arbitrary limits
#define MAX_LEXEME_LENGTH 500
#define MAX_TOKENS 500

// Moved from keywords_fsm.h
typedef enum {
    s,        // Start
    b, bo, boo, final_bool, // States for 'bool'
    c, ch, cha, final_char, // States for 'char'
    e, el, els, final_else, // States for 'else'
    f, fa, fal, fals, final_false, fl, flo, floa, final_float, fo, final_for, // States for 'false', 'float', 'for'
    i, in, final_if, final_int, // States for 'if', 'int'
    p, pr, pri, prin, print, final_printf, // States for 'printf'
    r, re, ret, retu, retur, final_return, // States for 'return'
    s1, sc, sca, scan, final_scanf, // States for 'scanf'
    t, tr, tru, final_true, // States for 'true'
    w, wh, whi, whil, final_while, // States for 'while'
    v, vo, voi, final_void // States for 'void'
} KeywordState;

typedef enum
{
    // Tokens composed of characters exclusively part of single-character tokens
    left_parenthesis,
    right_parenthesis,
    left_bracket,
    right_bracket,
    left_brace,
    right_brace,
    comma,
    semicolon,
    multiply,
    exponent,
    ampersand,

    // Possible multi-character tokens
    plus,
    minus,
    divide,
    equal,
    not_equal,
    assign,
    less,
    less_equal,
    greater,
    greater_equal,
    not,
    or,
    and,
    comment,

    // Tokens using the same characters
    modulo,

    // Literals
    identifier,
    string,
    integer_literal,
    float_literal,
    character_literal,
    
    // Keywords
    char_kw,
    int_kw,
    float_kw,
    bool_kw,
    if_kw,
    else_kw,
    for_kw,
    while_kw,
    return_kw,
    printf_kw,
    scanf_kw,
    true_kw,
    false_kw,
    void_kw,

    // Error tokens, not part of the language just for the scanner
    error_invalid_character,
    error_invalid_identifier,

    // End of file token
    token_eof
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
    "left_brace",
    "right_brace",
    "comma",
    "semicolon",
    "multiply",
    "exponent",
    "ampersand",
    "plus",
    "minus",
    "divide",
    "equal",
    "not_equal",
    "assign",
    "less",
    "less_equal",
    "greater",
    "greather_equal",
    "not",
    "or",
    "and",
    "comment",
    "modulo",
    "identifier",
    "string",
    "integer_literal",
    "float_literal",
    "character_literal",
    "char",
    "int",
    "float",
    "bool",
    "if",
    "else",
    "for",
    "while",
    "return",
    "printf",
    "scanf",
    "true",
    "false",
    "void",
    "error_invalid_character",
    "error_invalid_identifier",
    "token_eof"
};

#endif //TOKEN_H