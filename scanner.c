/* Sebesta simple Lexical Analyzer example */

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

#include "token.h"

/* Global declarations */

/* Variables */
char lexeme[MAX_LEXEME_LENGTH];
int current_char;
int lexeme_length;
int token;
int next_token;
int line_number = 1;
int column_number = 0;
int token_start_line;
int token_start_column;
int token_end_column;

FILE *in_fp;
FILE *symbol_fp;


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
int peek();
void unget_char(int ch);
void set_token_end_column();

/******************************************************/
/* main driver */
int main(int argc, char* argv[argc + 1]) {
    if (argc != 2) {
        printf("Usage: ./%s <filename>\n", argv[0]);
        return 1;
    }

    const char* fname = argv[1];
    char *last_period = strrchr(fname, '.');
    if (strcmp(last_period, ".core") != 0) {
        printf("Input file passed must have .core extension\n");
        return 1;
    }

    in_fp = fopen(fname, "rb");
    if (in_fp == NULL) {
        printf("ERROR - cannot open file\n");
        return 1;
    }

    symbol_fp = fopen("symbol_table.txt", "w");
    if (symbol_fp == NULL) {
        printf("ERROR - cannot open output file\n");
        fclose(in_fp);
        return 1;
    }

    // Design for header
    for(int i=0; i<128; i++) fprintf(symbol_fp, "_");
    fprintf(symbol_fp, "\n");
    fprintf(symbol_fp, "TOKEN CODE      | TOKEN                    | LINE #          | COLUMN #        | LEXEME\n");
    for(int i=0; i<128; i++) fprintf(symbol_fp, "_");
    fprintf(symbol_fp, "\n");

    do {
        lex();

        // Print error messages based on the error type, also not write into symbol_table.txt
        if (next_token == ERROR_INVALID_CHARACTER) {
            printf("ERROR - invalid char %c\n", lexeme[0]);
            continue;
        } else if (next_token == ERROR_INVALID_IDENTIFIER) {
            printf("ERROR - invalid identifier: %s\n", lexeme);
            continue;
        }

        // Print to console
        if (next_token >= 0 && next_token < sizeof(token_names)/sizeof(token_names[0])) {
            printf("Next token is: %-30s Next lexeme: is %s\n", token_names[next_token], lexeme);
        } else {
            printf("Next token is: Unknown, Next lexeme is %s\n", lexeme);
            continue;
        }

        // Try not writing comment tokens into the symbol table
        if (next_token == COMMENT) {
            continue;
        }

        // Write to symbol_table.txt
        if (next_token >= 0 && next_token < sizeof(token_names)/sizeof(token_names[0])) {
        fprintf(symbol_fp, "%-15d | %-24s | %-15d | %-15d | %s\n",
                next_token, token_names[next_token], token_start_line, token_start_column, lexeme);
        } else {
            fprintf(symbol_fp, "Unknown            | %-15d | %-15d | %s\n",
                    token_start_column, token_start_line, lexeme);
        }
    } while (next_token != TOKEN_EOF);

    // Design for footer
    for(int i=0; i<128; i++) fprintf(symbol_fp, "_");
    fprintf(symbol_fp, "\n");
    fclose(symbol_fp);
    fclose(in_fp);

    return 0;
}

/******************************************************/
/* add_char - a function to add next_char to lexeme */
void add_char() {
    if (lexeme_length <= MAX_LEXEME_LENGTH - 2) {
        lexeme[lexeme_length++] = current_char;
        lexeme[lexeme_length] = '\0';
    } else {
        printf("Error - lexeme is too long \n");
    }
}

/******************************************************/
/* getChar - a function to get the next character of input */
char get_char() {
    int ch = fgetc(in_fp);

    if (ch == '\n') {
        line_number++;
        column_number = 0; // Reset column number at the start of a new line
    } else if (ch != EOF) {
        column_number++;
    }

    return ch;
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
    current_char = get_char();
    current_char = get_non_blank();
    lexeme_length = 0;
    token_start_line = line_number;
    // this is for the global tokens to retain their column number
    token_start_column = column_number;
    
    // End if end of file
    if (current_char == EOF) return add_eof();

    // Parse strings
    if (current_char == '"') return string();

    // Parse number literals
    if (isdigit(current_char) ||
        (current_char == '.' && isdigit(peek()))) {
            return number();
        }
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
        case '+': return add_token(PLUS);
        case '-': return add_token(MINUS);
        case '*': return add_token(MULTIPLY);
        case '^': return add_token(EXPONENT);
        case '%': return add_token(MODULO);

        // Multi-character operators
        case '=': return add_token(match('=') ? EQUAL : ASSIGN);
        case '>': return add_token(match('=') ? GREATER_EQUAL : GREATER);
        case '<': return add_token(match('=') ? LESS_EQUAL : LESS);
        case '!': return add_token(match('=') ? NOT_EQUAL : NOT);
        case '&': return add_token(match('&') ? AND : AMPERSAND);
        case '|': return add_token(match('|') ? OR : OR);
        case '/':
            if (peek() == '/') {
                // It's a comment
                next_token = COMMENT;

                // Set token_start_column before consuming any characters
                token_start_column = column_number;

                // Add the first '/'
                add_char();

                // Consume and add the second '/'
                current_char = get_char(); // Get the second '/'
                add_char();

                // Proceed to read the rest of the comment
                current_char = get_char();
                while (current_char != '\n' && current_char != EOF) {
                    add_char();
                    current_char = get_char();
                }
                set_token_end_column();
                break;
            } else {
                return add_token(DIVIDE);
            }
        // If reached this, then must be invalid character
        default:
            add_char();
            next_token = ERROR_INVALID_CHARACTER;
            set_token_end_column();
            break;
    }
}

/******************************************************/
/* add_token - updates next_token and lexeme for printing */
void add_token(TokenType token) {
    add_char();
    next_token = token;
    set_token_end_column();
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
    int ch = fgetc(in_fp);
    ungetc(ch, in_fp);
    return ch;
}

/******************************************************/
/* number - reads the rest of the number literal */
void number() {
    // Handle leading decimal point
    if (current_char == '.' && isdigit(peek())) { 
        lexeme[0] = '0'; // Adds 0 
        lexeme[1] = '.';
        lexeme_length = 2;

        // Read subsequent digits
        while (isdigit(peek())) {
            current_char = get_char();
            lexeme[lexeme_length++] = current_char;
        }
    } else {
        //Handle regular numbers
        add_char();

        while (isdigit(peek())) {
            current_char = get_char();
            add_char();
        }

        //Handle fractional parts
        if (peek() == '.') {
            current_char = get_char();
            add_char();

            while (isdigit(peek())) {
                current_char = get_char();
                add_char();
            }
        }
    }

    // Handle trailing decimal points
    if (lexeme[lexeme_length - 1] == '.') { 
        lexeme[lexeme_length++] = '0';
        lexeme[lexeme_length] = '\0';
    } else {
        lexeme[lexeme_length] = '\0';
    }

    next_token = NUMBER;
    set_token_end_column();
}

/******************************************************/
/* identifier - reads the rest of the identifier */
void identifier() {
    // Add the first character we found
    add_char();

    char next_char = get_char();
    bool too_long = false;

    while (isalnum(next_char) || next_char == '_') {  // check if alphanumeric or underscore
        current_char = next_char;

        // Always add the character to preserve the full lexeme
        add_char();

        // Check if the length exceeds the limit
        if (lexeme_length > 31) {
            too_long = true;
        }

        next_char = get_char();
    }

    // Put back the last non-alphanumeric character we found
    unget_char(next_char);

    // If the identifier is too long, set the token type but delay error reporting
    if (too_long) {
        next_token = ERROR_INVALID_IDENTIFIER;
    } else {
        next_token = keywords();
    }
    set_token_end_column(); // Moved outside the if-else block

    // Now report errors if needed
    if (too_long) {
        next_token = ERROR_INVALID_IDENTIFIER;
    }
}

/******************************************************/
/* keywords - determine if the identifier is a keyword */
TokenType keywords() {
    switch (lexeme[0]) {
        // check if bool
        case 'b':
            if (lexeme[1] == 'o' && lexeme[2] == 'o' && lexeme[3] == 'l' && lexeme_length == 4) 
                return BOOL;
            break;
        // check if char
        case 'c':
            if (lexeme[1] == 'h' && lexeme[2] == 'a' && lexeme[3] == 'r' && lexeme_length == 4) 
                return CHAR;
            break;
        // check if else
        case 'e':
            if (lexeme[1] == 'l' && lexeme[2] == 's' && lexeme[3] == 'e' && lexeme_length == 4) 
                return ELSE;
            break;
        // check if false, float, or for
        case 'f':
            if (lexeme[1] == 'a' && lexeme[2] == 'l' && lexeme[3] == 's' && lexeme[4] == 'e' && lexeme_length == 5)
                return FALSE;
            if (lexeme[1] == 'l' && lexeme[2] == 'o' && lexeme[3] == 'a' && lexeme[4] == 't' && lexeme_length == 5)
                return FLOAT;
            if (lexeme[1] == 'o' && lexeme[2] == 'r' && lexeme_length == 3) 
                return FOR;
            break;
        // check if if or int
        case 'i':
            if (lexeme[1] == 'f' && lexeme_length == 2)
                return IF;
            if (lexeme[1] == 'n' && lexeme[2] == 't' && lexeme_length == 3)
                return INT;
            break;
        // check if printf
        case 'p':
            if (lexeme[1] == 'r' && lexeme[2] == 'i' && lexeme[3] == 'n' && lexeme[4] == 't' && lexeme[5] == 'f' && lexeme_length == 6) 
                return PRINTF;
            break;
        // check if return
        case 'r':
            if (lexeme[1] == 'e' && lexeme[2] == 't' && lexeme[3] == 'u' && lexeme[4] == 'r' && lexeme[5] == 'n' && lexeme_length == 6)
                return RETURN;
            break;
        // check if scanf
        case 's':
            if (lexeme[1] == 'c' && lexeme[2] == 'a' && lexeme[3] == 'n' && lexeme[4] == 'f' && lexeme_length == 5) 
                return SCANF;
            break;
        // check if true
        case 't':
            if (lexeme[1] == 'r' && lexeme[2] == 'u' && lexeme[3] == 'e' && lexeme_length == 4) 
                return TRUE;
            break;
        // check if while
        case 'w':
            if (lexeme[1] == 'h' && lexeme[2] == 'i' && lexeme[3] == 'l' && lexeme[4] == 'e' && lexeme_length == 5) 
                return WHILE;
            break;
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
        set_token_end_column();
    } else { // Handle missing closing single quote
        next_token = ERROR_INVALID_CHARACTER;
        printf("Error - unterminated character literal\n");
    }
}

void add_eof() {
    lexeme[0] = 'E';
    lexeme[1] = 'O';
    lexeme[2] = 'F';
    lexeme[3] = '\0';
    next_token = TOKEN_EOF;
    token_start_line = line_number;
    token_start_column = -1;
    token_end_column = -1;
}

/******************************************************/
/* string - reads the rest of the string literal */
void string()
{
    add_char();  // Add the opening quote to lexeme
    char nextChar = get_char();

    while (nextChar != '"' && nextChar != EOF) {
        if (nextChar == '\\') {  // Handle escape sequences
            current_char = nextChar;
            add_char(); // Add backslash to lexeme
            nextChar = get_char();
            // Increment column_number inside get_char()
            // Handle escape character
            current_char = nextChar;
            add_char();
        } else {
            current_char = nextChar;
            add_char();
        }
        nextChar = get_char();
    }

    if (nextChar == '"') {
        current_char = nextChar;
        add_char();  // Add the closing quote to lexeme
        next_token = STRING;
        set_token_end_column();  // Successfully parsed a string literal
    } else {
        // Handle error for unterminated string
        printf("Error - unterminated string literal\n");
        next_token = ERROR_INVALID_CHARACTER;
    }
}

void unget_char(int ch) {
    if (ch == EOF) return; // Do nothing for EOF

    ungetc(ch, in_fp);

    if (ch == '\n') {
        line_number--;
        // Column number cannot be accurately adjusted here unless you track line lengths
        // You may need to handle this if your lexer allows for multi-line ungets
    } else {
        column_number--;
    }
}

/******************************************************/
/* helper function to set token_end_column */
void set_token_end_column() {
    token_end_column = token_start_column + lexeme_length;
}