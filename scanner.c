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
    char* last_period = strrchr(fname, '.');
    if (!last_period || strcmp(last_period, ".core") != 0) {
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
    for (int i = 0; i < 128; i++) fprintf(symbol_fp, "_");
    fprintf(symbol_fp, "\n");
    fprintf(symbol_fp, "TOKEN CODE      | TOKEN                    | LINE #          | COLUMN #        | LEXEME\n");
    for (int i = 0; i < 128; i++) fprintf(symbol_fp, "_");
    fprintf(symbol_fp, "\n");

    current_char = get_char(); // Initialize curent_char before the loop

    do {
        lex();

        // Skip writing to symbol table if there's an error
        if (next_token == -1 || next_token == ERROR_INVALID_CHARACTER) {
            continue;
        }

        // Handle TOKEN_EOF separately
        if (next_token == TOKEN_EOF) {
            printf("Next token is: %-30s Next lexeme: is %s\n", token_names[next_token], "EOF");
            fprintf(symbol_fp, "47              | TOKEN_EOF                | %d               | -1              | EOF\n", line_number);
            break;
        }

        // Print to console
        if (next_token >= 0 && next_token < sizeof(token_names) / sizeof(token_names[0])) {
            printf("Next token is: %-30s Next lexeme: is %s\n", token_names[next_token], lexeme);
        }
        else {
            printf("Next token is: Unknown, Next lexeme is %s\n", lexeme);
            continue;
        }

        // Skip comments from symbol table
        if (next_token == COMMENT) {
            continue;
        }

        // Write to symbol_table.txt only valid tokens
        if (next_token >= 0 && next_token < sizeof(token_names) / sizeof(token_names[0])) {
            int token_code = next_token; // Example mapping

            fprintf(symbol_fp, "%-15d | %-24s | %-15d | %-15d | %s\n",
                token_code, token_names[next_token], token_start_line, token_start_column, lexeme);
        }
    } while (next_token != TOKEN_EOF);

    // Design for footer
    for (int i = 0; i < 128; i++) fprintf(symbol_fp, "_");
    fprintf(symbol_fp, "\n");
    fclose(symbol_fp);
    fclose(in_fp);

    return 0;
}

/******************************************************/
/* add_char - a function to add current_char to lexeme */
void add_char() {
    if (lexeme_length <= MAX_LEXEME_LENGTH - 2) {
        lexeme[lexeme_length++] = current_char;
        lexeme[lexeme_length] = '\0';
    }
    else {
        printf("Error - lexeme is too long \n");
    }
}

/******************************************************/
/* get_char - a function to get the next character of input */
char get_char() {
    int ch = fgetc(in_fp);

    if (ch == '\n') {
        line_number++;
        column_number = 0; // Reset column number at the start of a new line
    }
    else if (ch != EOF) {
        column_number++;
    }

    return ch;
}

/******************************************************/
/* get_non_blank - a function to call get_char until it returns a non-whitespace character */
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

    current_char = get_non_blank();
    token_start_line = line_number;
    token_start_column = column_number;

    // Check for EOF before proceeding
    if (current_char == EOF) {
        add_eof();
        return;
    }

    // Parse strings
    if (current_char == '"') {
        string();
        return;
    }

    // Parse number literals
    if (isdigit(current_char) || (current_char == '.' && isdigit(peek()))) {
        number();
        return; // After handling a number, return to avoid redundant checks
    }

    // Parse identifiers
    if (isalpha(current_char) || current_char == '_') {
        identifier();
        return;
    }

    // Parse character literals
    if (current_char == '\'') {
        character_literal();
        return;
    }

    // Parse one- or two-character tokens
    switch (current_char) {
        // Single-character operators
        case '(': add_token(LEFT_PARENTHESIS); break;
        case ')': add_token(RIGHT_PARENTHESIS); break;
        case '[': add_token(LEFT_BRACKET); break;
        case ']': add_token(RIGHT_BRACKET); break;
        case '{': add_token(LEFT_BRACE); break;
        case '}': add_token(RIGHT_BRACE); break;
        case ',': add_token(COMMA); break;
        case ';': add_token(SEMICOLON); break;
        case '+': add_token(PLUS); break;
        case '-': add_token(MINUS); break;
        case '*': add_token(MULTIPLY); break;
        case '^': add_token(EXPONENT); break;
        case '%': add_token(MODULO); break;

        // Multi-character operators handled directly
        case '=':
            add_char(); // Add first '='
            current_char = get_char(); // Advance to next character
            if (current_char == '=') {
                add_char(); // Add second '='
                next_token = EQUAL;
                set_token_end_column();
                current_char = get_char();
            } else {
                next_token = ASSIGN;
                set_token_end_column();
            }
            break;
        case '>':
            add_char();
            current_char = get_char();
            if (current_char == '=') {
                add_char();
                next_token = GREATER_EQUAL;
                set_token_end_column();
                current_char = get_char();
            } else {
                next_token = GREATER;
                set_token_end_column();
            }
            break;
        case '<':
            add_char();
            current_char = get_char();
            if (current_char == '=') {
                add_char();
                next_token = LESS_EQUAL;
                set_token_end_column();
                current_char = get_char();
            } else {
                next_token = LESS;
                set_token_end_column();
            }
            break;
        case '!':
            add_char();
            current_char = get_char();
            if (current_char == '=') {
                add_char();
                next_token = NOT_EQUAL;
                set_token_end_column();
                current_char = get_char();
            } else {
                next_token = NOT;
                set_token_end_column();
            }
            break;
        case '&':
            add_char();
            current_char = get_char();
            if (current_char == '&') {
                add_char();
                next_token = AND;
                set_token_end_column();
                current_char = get_char();
            } else {
                next_token = AMPERSAND;
                set_token_end_column();
            }
            break;
        case '|':
            add_char();
            current_char = get_char();
            if (current_char == '|') {
                add_char();
                next_token = OR;
                set_token_end_column();
                current_char = get_char();
            } else {
                next_token = OR;
                set_token_end_column();
            }
            break;
        case '/':
            current_char = get_char();
            if (current_char == '/') {
                // It's a comment
                next_token = COMMENT;
                token_start_column = column_number;
                lexeme[lexeme_length++] = '/';
                lexeme[lexeme_length++] = '/';
                lexeme[lexeme_length] = '\0';

                // Proceed to read the rest of the comment
                current_char = get_char();
                while (current_char != '\n' && current_char != EOF) {
                    add_char();
                    current_char = get_char();
                }
                set_token_end_column();
                break; // Return after handling the comment
            }
            else {
                // It's a divide operator
                unget_char(current_char);
                current_char = '/';
                add_token(DIVIDE);
                break;
            }
            // If reached this, then must be invalid character
        default:
            printf("ERROR - invalid char %c\n", current_char);
            next_token = ERROR_INVALID_CHARACTER;
            set_token_end_column();
            current_char = get_char();
            return;
    }
}

/******************************************************/
/* add_token - updates next_token and lexeme for printing */
void add_token(TokenType token) {
    add_char();
    next_token = token;
    set_token_end_column();
    current_char = get_char(); // Advance to the next character
}

/******************************************************/
/* number - reads the rest of the number literal */
void number() {
    bool has_decimal = false;
    bool error_occurred = false;

    // Reset lexeme
    lexeme_length = 0;

    // Handle leading decimal point
    if (current_char == '.') {
        add_char();
        has_decimal = true;
        current_char = get_char();
    }

    // Read all digits and valid separators
    while (isdigit(current_char) || current_char == '\'' || current_char == '`' || current_char == '.') {
        if (current_char == '.') {
            if (has_decimal) {
                // Second decimal point encountered
                break;
            }
            has_decimal = true;
            add_char();
            current_char = get_char();
        }
        else if (current_char == '\'' || current_char == '`') {
            // Validate separator: must be followed by exactly three digits
            char separator = current_char;
            add_char(); // Add the separator
            current_char = get_char();
            int digits = 0;
            while (digits < 3 && isdigit(current_char)) {
               add_char();
                digits++;
                current_char = get_char();
            }
            if (digits != 3) {
                // Invalid separator usage
                fprintf(stderr, "ERROR: Invalid noise separators at line %d, col %d\n",
                    line_number, column_number);
                error_occurred = true;
                break;
            }
            // Continue to check if another separator follows
        }
        else {
            add_char();
            current_char = get_char();
        }
    }

    // If an error occurred, consume the rest of the invalid number literal
    if (error_occurred) {
        while (isdigit(current_char) || current_char == '\'' || current_char == '`' || current_char == '.') {
            current_char = get_char();
        }
        next_token = -1;
        lexeme_length = 0; // Reset lexeme to ignore invalid number
        return; // Exit the function after handling the error
    }

    // Proceed with validation if no errors occurred
    if (!error_occurred) {
        int new_length = 0;
        {
            char temp[MAX_LEXEME_LENGTH];
            int temp_len = 0;
            int digit_count = 0;
            bool strict_noise_valid = true;

            // Scan from the end to the beginning
            for (int i = lexeme_length - 1; i >= 0; i--) {
                if (isdigit(lexeme[i])) {
                    temp[temp_len++] = lexeme[i];
                    digit_count++;
                }
                else if (lexeme[i] == '\'' || lexeme[i] == '`') {
                    if (digit_count != 3) {
                        strict_noise_valid = false;
                        break;
                    }
                    digit_count = 0;
                }
                else if (lexeme[i] == '.') {
                    temp[temp_len++] = lexeme[i];
                    digit_count = 0;
                }
                else if (i == 0 && (lexeme[i] == '-' || lexeme[i] == '+')) {
                    temp[temp_len++] = lexeme[i];
                }
                else {
                    strict_noise_valid = false;
                    break;
                }
            }

            // Handle misaligned separators
            if (!strict_noise_valid) {
                fprintf(stderr, "ERROR: Invalid noise separators at line %d, col %d\n",
                    line_number, column_number);
                next_token = -1;
                lexeme_length = 0;
                return; // Exit the function after handling the error
            }

            // Reverse temp into lexeme
            new_length = 0;
            for (int i = temp_len - 1; i >= 0; i--) {
                lexeme[new_length++] = temp[i];
            }

            // Add 0 to leading decimal if necessary
            if (lexeme[0] == '.') {
                // Shift right by 1
                for (int i = new_length; i >= 0; i--) {
                    lexeme[i + 1] = lexeme[i];
                }
                lexeme[0] = '0';
                new_length++;
            }

            // Add 0 to trailing decimal if necessary
            if (lexeme[new_length - 1] == '.') {
                lexeme[new_length] = '0';
                new_length++;
                lexeme[new_length] = '\0';
            }
            else {
                lexeme[new_length] = '\0';
            }

            lexeme_length = new_length;
        }
    }

    next_token = has_decimal ? FLOAT_LITERAL : INTEGER_LITERAL;
    set_token_end_column();
    // current_char is already at the next character after the number
}

/******************************************************/
/* identifier - reads the rest of the identifier and checks length <= 31 */
void identifier() {
    char local_buffer[256];
    int local_length = 0;

    // First character is already known to be valid for an identifier
    local_buffer[local_length++] = current_char;
    local_buffer[local_length] = '\0';

    current_char = get_char();

    // Read additional valid identifier characters
    while (isalnum(current_char) || current_char == '_') {
        // Append to local_buffer, avoiding overflow
        if (local_length < (int)(sizeof(local_buffer) - 1)) {
            local_buffer[local_length++] = current_char;
            local_buffer[local_length] = '\0';
        }
        current_char = get_char();
    }

    // Now decide if valid or invalid based on length <= 31
    if (local_length > 31) {
        // Report the entire invalid identifier
        printf("ERROR - invalid identifier: %s\n", local_buffer);
        // Set next_token to -1 so it won't appear as a separate token
        next_token = -1; 
        lexeme_length = 0;
        lexeme[0] = '\0';
        return; 
    } else {
        // Copy to lexeme (safe because local_length <= 31)
        strncpy(lexeme, local_buffer, local_length);
        lexeme[local_length] = '\0';
        lexeme_length = local_length;
        // Check if it's a keyword or just an identifier
        next_token = keywords();
    }

    set_token_end_column();
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
        // check for void
        case 'v':
            if (lexeme[1] == 'o' && lexeme[2] == 'i' && lexeme[3] == 'd' && lexeme_length == 4) 
                return VOID;
            break;
        // check if while
        case 'w':
            if (lexeme[1] == 'h' && lexeme[2] == 'i' && lexeme[3] == 'l' && lexeme[4] == 'e' && lexeme_length == 5) 
                return WHILE;
            break;
    }

    return IDENTIFIER;
}

/******************************************************/
/* character_literal - reads the character literal */
void character_literal()
{
    add_char(); // Add the opening single quote

    // Read the character or escape sequence
    current_char = get_char();

    if (current_char == '\\') { // Handle escape sequences like '\n' or '\t'
        add_char(); // Add the backslash
        current_char = get_char(); // Get the actual escaped character
        if (current_char != '\'' && current_char != EOF) {
            add_char(); // Add the escaped character
        }
        else {
            next_token = ERROR_INVALID_CHARACTER;
            printf("Error - unterminated character literal\n");
            return;
        }
    }
    else if (current_char == '\'' || current_char == EOF) { // Handle empty or malformed character literals
        next_token = ERROR_INVALID_CHARACTER;
        printf("Error - invalid or unterminated character literal\n");
        return;
    }
    else {
        add_char(); // Add the character
    }

    // Read the closing single quote
    current_char = get_char();
    if (current_char == '\'') {
        add_char();
        next_token = CHARACTER_LITERAL;
        set_token_end_column();
        current_char = get_char();
    }
    else { // Handle missing closing single quote
        next_token = ERROR_INVALID_CHARACTER;
        printf("Error - unterminated character literal\n");
    }
}

/******************************************************/
/* add_eof - adds the EOF token */
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
    current_char = get_char();

    while (current_char != '"' && current_char != EOF) {
        if (current_char == '\\') {  // Handle escape sequences
            add_char(); // Add backslash to lexeme
            current_char = get_char();
            // Handle escape character
            add_char();
        }
        else {
            add_char();
        }
        current_char = get_char();
    }

    if (current_char == '"') {
        add_char();  // Add the closing quote to lexeme
        next_token = STRING;
        set_token_end_column();
        current_char = get_char();  // Advance to the next character
    }
    else {
        // Handle error for unterminated string
        printf("Error - unterminated string literal\n");
        next_token = ERROR_INVALID_CHARACTER;
    }
}

/******************************************************/
/* peek - a function to peek at the next character without consuming it */
int peek() {
    int ch = fgetc(in_fp);
    ungetc(ch, in_fp);
    return ch;
}

/******************************************************/
/* unget_char - ungets a character */
void unget_char(int ch) {
    if (ch == EOF) return; // Do nothing for EOF

    ungetc(ch, in_fp);

    if (ch == '\n') {
        line_number--;
        // Column number cannot be accurately adjusted here unless you track line lengths
        // You may need to handle this if your lexer allows for multi-line ungets
    }
    else {
        column_number--;
    }
}

/******************************************************/
/* set_token_end_column - sets the end column of the token */
void set_token_end_column() {
    token_end_column = token_start_column + lexeme_length;
}