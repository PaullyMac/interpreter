/* Sebesta simple Lexical Analyzer example */

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

#include "token.h"

/* Global declarations */

/* Variables */
char lexeme[100];
int current_char;
int lexeme_length;
int token;
int next_token;
FILE *in_fp;

/* Function declarations */
void add_char();
char get_char();
char get_non_blank();
void lex();
void add_token(TokenType token);
void number();
void identifier();
bool match(char expected);
void string();
void add_eof();
void character_literal(); 
TokenType keywords();
TokenType check_keyword(int start, int length, const char* rest, TokenType type);
int peek();

/******************************************************/
/* main driver */
int main() {
    /* Open the input data file and process its contents */
    const char* fname = "file.txt";
    in_fp = fopen(fname, "rb");

    if (in_fp == NULL) {
        printf("ERROR - cannot open file \n");
        return 1;
    }

    // Scanning
    do {
        lex();
        if (next_token == ERROR_INVALID_CHARACTER) {
            printf("ERROR - invalid char %c\n", current_char);
            continue;
        }

        printf("Next token is: %d, Next lexeme is %s\n", next_token, lexeme);
    } while (next_token != EOF);

    return 0;
}

/******************************************************/
/* add_char - a function to add next_char to lexeme */
void add_char() {
    if (lexeme_length <= 98) {
        lexeme[lexeme_length++] = current_char;
        lexeme[lexeme_length] = '\0';
    } else {
        printf("Error - lexeme is too long \n");
    }
}

/******************************************************/
/* getChar - a function to get the next character of input */
char get_char() {
    return getc(in_fp);
}

/******************************************************/
/* getNonBlank - a function to call getChar until it returns a non-whitespace character */
char get_non_blank() {
    while (isspace(current_char)) {
        current_char = get_char();
    }
    return current_char;
}

/******************************************************/
/* lex - a simple lexical analyzer for arithmetic expressions */
void lex() {
    lexeme_length = 0;
    current_char = get_char();
    current_char = get_non_blank();

    // End if end of file
    if (current_char == EOF) return add_eof();

    // Parse strings
    if (current_char == '"') return string();

    // Parse number literals
    if (isdigit(current_char)) return number();

    // Parse identifiers
    if (isalpha(current_char)) return identifier();

    // Parse character literals
    if (current_char == '\'') return character_literal();

    // Parse one- or two-character tokens
    switch (current_char) {
        // Single-character operators
        case '(': return add_token(LEFT_PARENTHESIS);
        case ')': return add_token(RIGHT_PARENTHESIS);
        case '[': return add_token(LEFT_BRACKET);
        case ']': return add_token(RIGHT_BRACKET);
        case '{': return add_token(LEFT_BRACE);
        case '}': return add_token(RIGHT_BRACE);
        case ',': return add_token(COMMA);
        case ';': return add_token(SEMICOLON);
        case '*': return add_token(MULTIPLY);
        case '^': return add_token(EXPONENT);
        case '%': return add_token(MODULO);

        // Multi-character operators
        case '+': return add_token(match('+') ?  INCREMENT : PLUS);
        case '-': return add_token(match('-') ?  DECREMENT : MINUS);
        case '=': return add_token(match('=') ? EQUAL : ASSIGN);
        case '>': return add_token(match('=') ? GREATER_EQUAL : EQUAL);
        case '<': return add_token(match('=') ? LESS_EQUAL : EQUAL);
        case '!': return add_token(match('=') ? NOT_EQUAL : EQUAL);
        case '|': return add_token(match('|') ? OR : OR);
        case '&': return add_token(match('&') ? OR : OR);
        case '/':
            if (match('/')) {
                add_token(COMMENT);
                // Read comment until newline
                char tmp;
                while ((tmp = get_char()) != '\n' && current_char != EOF)
                {
                    current_char = tmp;
                    add_char();
                }
                return;
            } else {
                return add_token(DIVIDE);
            }

        // If reached this, then must be invalid character
        default:
            next_token = ERROR_INVALID_CHARACTER;
            add_char();
    }
}

/******************************************************/
/* add_token - updates next_token and lexeme for printing */
void add_token(TokenType token) {
    add_char();
    next_token = token;
}

/******************************************************/
/* match - helper function for multi-character operators */
bool match(char expected) {
    if (peek() != expected) {
        return false;
    }

    // Two-character token and we get second character
    add_char(); // Adds the first character of the token, the next add_char is handled by the addToken call
    current_char = get_char();
    return true;
}

/******************************************************/
/* peek - a function to peek at the next character without consuming it */
int peek() {
    int next = get_char();
    ungetc(next, in_fp);
    return next;
}

/******************************************************/
/* number - reads the rest of the number literal */
void number() {
    // Add the first digit found
    add_char();

    // Read subsequent digits
    while (isdigit(peek())) {
        current_char = get_char();
        add_char();
    }

    // Check if decimal point, hanging decimals are valid in C
    if (peek() == '.') {
        // Consume and add
        current_char = get_char();
        add_char();

        // Fractional part
        while (isdigit(peek())) {
            current_char = get_char();
            add_char();
        }
    }

    next_token = NUMBER;
}

/******************************************************/
/* identifier - reads the rest of the identifier */
void identifier() {
    // Add the first character we found
    add_char();

    char next_char = get_char();
    while (isalnum(next_char) || next_char == '_') {  // isalnum checks for letters or digits
        current_char = next_char;
        add_char();
        next_char = get_char();
    }

    // Put back the last non-alphanumeric character we found
    ungetc(next_char, in_fp);

    // Check if the identifier is a keyword or not 
    next_token = keywords();
}

/******************************************************/
/* keywords - determine if the identifier is a keyword */
TokenType keywords() { 
    switch(lexeme[0]) { 
        case 'b': return check_keyword(1, 3, "ool", BOOL);
        case 'c': return check_keyword(1, 3, "har", CHAR);
        case 'e': return check_keyword(1, 3, "lse", ELSE);
        case 'f': 
            if (lexeme_length > 1) {
                switch (lexeme[1]) { 
                    case 'a': return check_keyword(2, 3, "lse", FALSE);
                    case 'l': return check_keyword(2, 3, "oat", FLOAT);
                    case 'o': return check_keyword(2, 1, "r", FOR);
                }
            }
        case 'i': 
            if (lexeme_length > 1) {
                switch (lexeme[1]) { 
                    case 'f': return check_keyword(2, 0, "", IF);
                    case 'n': return check_keyword(2, 1, "t", INT);
                }
            }
        case 'p': return check_keyword(1, 5, "rintf", PRINTF);
        case 'r': return check_keyword(1, 5, "eturn", RETURN);
        case 's': return check_keyword(1, 4, "canf", SCANF);
        case 't': return check_keyword(1, 3, "rue", TRUE);
        case 'w': return check_keyword(1, 4, "hile", WHILE);
    }

    return IDENTIFIER;
}

/******************************************************/
/* checkKeyword - helper function to check if the identifier matches a keyword */
TokenType check_keyword(int start, int length, const char* rest, TokenType type) {
    if (lexeme_length == start + length && memcmp(lexeme + start, rest, length) == 0) {
        return type;
    }
    return IDENTIFIER;
}

void character_literal()
{
    add_char(); // Add the opening single quote

    // Read the character or escape sequence
    current_char = get_char();

    if (current_char == '\\') { // Handle escape sequences like '\n' or '\t'
        add_char();
        current_char = get_char(); // Add the actual escaped character
        if (current_char != '\'' && current_char != EOF) {
            add_char();
        } else {
            next_token = ERROR_INVALID_CHARACTER;
            printf("Error - unterminated character literal\n");
            return;
        }
    } else if (current_char == '\'' || current_char == EOF) { // Handle empty or malformed character literals
        next_token = ERROR_INVALID_CHARACTER;
        printf("Error - invalid or unterminated character literal\n");
        return;
    } else {
        add_char(); // Add the character
    }

    // Read the closing single quote
    current_char = get_char();
    if (current_char == '\'') {
        add_char();
        next_token = CHARACTER_LITERAL;
    } else { // Handle missing closing single quote
        next_token = ERROR_INVALID_CHARACTER;
        printf("Error - unterminated character literal\n");
    }
}

void add_eof()
{
    lexeme[0] = 'E';
    lexeme[1] = 'O';
    lexeme[2] = 'F';
    lexeme[3] = '\0';
    next_token = EOF;
}

/******************************************************/
/* string - reads the rest of the string literal */
void string()
{
    add_char();  // Add the opening quote to lexeme
    char nextChar = get_char();

    while (nextChar != '"' && nextChar != EOF) {
        if (nextChar == '\\') {  // Handle escape sequences
            nextChar = get_char();  // Get the next character after the backslash
            if (nextChar == EOF) break;  // Stop if EOF is reached

            // Handle specific escape sequences
            switch (nextChar) {
                case 'n': current_char = '\n'; break;  // Newline
                case 't': current_char = '\t'; break;  // Tab
                case '\\': current_char = '\\'; break;  // Backslash
                case '"': current_char = '"'; break;   // Double quote
                default:
                    // For unrecognized escape sequences, add the backslash and character as-is
                    printf("Warning - unrecognized escape sequence \\%c\n", nextChar);
                    current_char = '\\';
                    add_char();
                    current_char = nextChar;
            }
        } else {
            current_char = nextChar;
        }

        add_char();  // Add the resolved character to the lexeme
        nextChar = get_char();
    }

    if (nextChar == '"') {
        current_char = nextChar;
        add_char();  // Add the closing quote to lexeme
        next_token = STRING;  // Successfully parsed a string literal
    } else {
        // Handle error for unterminated string
        printf("Error - unterminated string literal\n");
        next_token = ERROR_INVALID_CHARACTER;
    }
}
