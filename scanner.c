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
void identifier_function();
void string_function();
void add_eof();
void character_literal_function();
TokenType keywords(char *lexeme);
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
        if (next_token == -1 || next_token == error_invalid_character) {
            continue;
        }

        // Handle TOKEN_EOF separately
        if (next_token == token_eof) {
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
        if (next_token == comment) {
            continue;
        }

        // Write to symbol_table.txt only valid tokens
        if (next_token >= 0 && next_token < sizeof(token_names) / sizeof(token_names[0])) {
            int token_code = next_token; // Example mapping

            fprintf(symbol_fp, "%-15d | %-24s | %-15d | %-15d | %s\n",
                token_code, token_names[next_token], token_start_line, token_start_column, lexeme);
        }
    } while (next_token != token_eof);

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
        string_function();
        return;
    }

    // Parse number literals
    if (isdigit(current_char) || (current_char == '.' && isdigit(peek()))) {
        number();
        return; // After handling a number, return to avoid redundant checks
    }

    // Parse identifiers
    if (isalpha(current_char) || current_char == '_') {
        identifier_function();
        return;
    }

    // Parse character literals
    if (current_char == '\'') {
        character_literal_function();
        return;
    }

    // Parse one- or two-character tokens
    switch (current_char) {
        // Single-character operators
        case '(': add_token(left_parenthesis); break;
        case ')': add_token(right_parenthesis); break;
        case '[': add_token(left_bracket); break;
        case ']': add_token(right_bracket); break;
        case '{': add_token(left_brace); break;
        case '}': add_token(right_brace); break;
        case ',': add_token(comma); break;
        case ';': add_token(semicolon); break;
        case '+': add_token(plus); break;
        case '-': add_token(minus); break;
        case '*': add_token(multiply); break;
        case '^': add_token(exponent); break;
        case '%': add_token(modulo); break;

        // Multi-character operators handled directly
        case '=':
            add_char(); // Add first '='
            current_char = get_char(); // Advance to next character
            if (current_char == '=') {
                add_char(); // Add second '='
                next_token = equal;
                set_token_end_column();
                current_char = get_char();
            } else {
                next_token = assign;
                set_token_end_column();
            }
            break;
        case '>':
            add_char();
            current_char = get_char();
            if (current_char == '=') {
                add_char();
                next_token = greater_equal;
                set_token_end_column();
                current_char = get_char();
            } else {
                next_token = greater;
                set_token_end_column();
            }
            break;
        case '<':
            add_char();
            current_char = get_char();
            if (current_char == '=') {
                add_char();
                next_token = less_equal;
                set_token_end_column();
                current_char = get_char();
            } else {
                next_token = less;
                set_token_end_column();
            }
            break;
        case '!':
            add_char();
            current_char = get_char();
            if (current_char == '=') {
                add_char();
                next_token = not_equal;
                set_token_end_column();
                current_char = get_char();
            } else {
                next_token = not;
                set_token_end_column();
            }
            break;
        case '&':
            add_char();
            current_char = get_char();
            if (current_char == '&') {
                add_char();
                next_token = and;
                set_token_end_column();
                current_char = get_char();
            } else {
                next_token = ampersand;
                set_token_end_column();
            }
            break;
        case '|':
            add_char();
            current_char = get_char();
            if (current_char == '|') {
                add_char();
                next_token = or;
                set_token_end_column();
                current_char = get_char();
            } else {
                next_token = or;
                set_token_end_column();
            }
            break;
        case '/':
            current_char = get_char();
            if (current_char == '/') {
                // It's a comment
                next_token = comment;
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
                add_token(divide);
                break;
            }
            // If reached this, then must be invalid character
            default:
                if (isalpha(current_char)) {
                    identifier_function();
                    next_token = keywords(lexeme);
                } else if (isdigit(current_char)) {
                    number();
                } else {
                    printf("ERROR - invalid char %c\n", current_char);
                    next_token = error_invalid_character;
                    set_token_end_column();
                    current_char = get_char();
                }
                break;
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

    next_token = has_decimal ? float_literal : integer_literal;
    set_token_end_column();
    // current_char is already at the next character after the number
}

/******************************************************/
/* identifier - reads the rest of the identifier and checks length <= 31 */
void identifier_function() {
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
        next_token = keywords(lexeme);
    }

    set_token_end_column();
}

/******************************************************/
/* keywords - a function to check if the lexeme is a keyword */
TokenType keywords(char *lexeme) {
    char *current = lexeme;
    KeywordState state = s;

    while (*current != '\0') {
        switch (state) {
            case s:
                switch (*current) {
                    case 'b': state = b; break; // Possible 'bool'
                    case 'c': state = c; break; // Possible 'char'
                    case 'e': state = e; break; // Possible 'else'
                    case 'f': state = f; break; // Possible 'false', 'float', 'for'
                    case 'i': state = i; break; // Possible 'if', 'int'
                    case 'p': state = p; break; // Possible 'printf'
                    case 'r': state = r; break; // Possible 'return'
                    case 's': state = s1; break; // Possible 'scanf'
                    case 't': state = t; break; // Possible 'true'
                    case 'w': state = w; break; // Possible 'while'
                    case 'v': state = v; break; // Possible 'void'
                    default: return identifier; // Not a keyword
                }
                break;

            // States for 'bool'
            case b:
                if (*current == 'o') state = bo; else return identifier;
                break;
            case bo:
                if (*current == 'o') state = boo; else return identifier;
                break;
            case boo:
                if (*current == 'l') state = final_bool; else return identifier;
                break;
            case final_bool:
                return (*current == '\0') ? bool_kw : identifier;

            // States for 'char'
            case c:
                if (*current == 'h') state = ch; else return identifier;
                break;
            case ch:
                if (*current == 'a') state = cha; else return identifier;
                break;
            case cha:
                if (*current == 'r') state = final_char; else return identifier;
                break;
            case final_char:
                return (*current == '\0') ? char_kw : identifier;

            // States for 'else'
            case e:
                if (*current == 'l') state = el; else return identifier;
                break;
            case el:
                if (*current == 's') state = els; else return identifier;
                break;
            case els:
                if (*current == 'e') state = final_else; else return identifier;
                break;
            case final_else:
                return (*current == '\0') ? else_kw : identifier;

            // States for 'false', 'float', 'for'
            case f:
                if (*current == 'a') state = fa;  // Possible 'false'
                else if (*current == 'l') state = fl; // Possible 'float'
                else if (*current == 'o') state = fo; // Possible 'for'
                else return identifier;
                break;
            case fa:
                if (*current == 'l') state = fal; else return identifier;
                break;
            case fal:
                if (*current == 's') state = fals; else return identifier;
                break;
            case fals:
                if (*current == 'e') state = final_false; else return identifier;
                break;
            case final_false:
                return (*current == '\0') ? false_kw : identifier;

            case fl:
                if (*current == 'o') state = flo; else return identifier;
                break;
            case flo:
                if (*current == 'a') state = floa; else return identifier;
                break;
            case floa:
                if (*current == 't') state = final_float; else return identifier;
                break;
            case final_float:
                return (*current == '\0') ? float_kw : identifier;

            case fo:
                if (*current == 'r') state = final_for; else return identifier;
                break;
            case final_for:
                return (*current == '\0') ? for_kw : identifier;

            // States for 'if', 'int'
            case i:
                if (*current == 'f') state = final_if;
                else if (*current == 'n') state = in;
                else return identifier;
                break;
            case final_if:
                return (*current == '\0') ? if_kw : identifier;

            case in:
                if (*current == 't') state = final_int; else return identifier;
                break;
            case final_int:
                return (*current == '\0') ? int_kw : identifier;

            // States for 'printf'
            case p:
                if (*current == 'r') state = pr; else return identifier;
                break;
            case pr:
                if (*current == 'i') state = pri; else return identifier;
                break;
            case pri:
                if (*current == 'n') state = prin; else return identifier;
                break;
            case prin:
                if (*current == 't') state = print; else return identifier;
                break;
            case print:
                if (*current == 'f') state = final_printf; else return identifier;
                break;
            case final_printf:
                return (*current == '\0') ? printf_kw : identifier;

            // States for 'return'
            case r:
                if (*current == 'e') state = re; else return identifier;
                break;
            case re:
                if (*current == 't') state = ret; else return identifier;
                break;
            case ret:
                if (*current == 'u') state = retu; else return identifier;
                break;
            case retu:
                if (*current == 'r') state = retur; else return identifier;
                break;
            case retur:
                if (*current == 'n') state = final_return; else return identifier;
                break;
            case final_return:
                return (*current == '\0') ? return_kw : identifier;

            // States for 'scanf'
            case s1:
                if (*current == 'c') state = sc; else return identifier;
                break;
            case sc:
                if (*current == 'a') state = sca; else return identifier;
                break;
            case sca:
                if (*current == 'n') state = scan; else return identifier;
                break;
            case scan:
                if (*current == 'f') state = final_scanf; else return identifier;
                break;
            case final_scanf:
                return (*current == '\0') ? scanf_kw : identifier;

            // States for 'true'
            case t:
                if (*current == 'r') state = tr; else return identifier;
                break;
            case tr:
                if (*current == 'u') state = tru; else return identifier;
                break;
            case tru:
                if (*current == 'e') state = final_true; else return identifier;
                break;
            case final_true:
                return (*current == '\0') ? true_kw : identifier;

            // States for 'while'
            case w:
                if (*current == 'h') state = wh; else return identifier;
                break;
            case wh:
                if (*current == 'i') state = whi; else return identifier;
                break;
            case whi:
                if (*current == 'l') state = whil; else return identifier;
                break;
            case whil:
                if (*current == 'e') state = final_while; else return identifier;
                break;
            case final_while:
                return (*current == '\0') ? while_kw : identifier;

            // States for 'void'
            case v:
                if (*current == 'o') state = vo; 
                else return identifier;
                break;
            case vo:
                if (*current == 'i') state = voi; 
                else return identifier;
                break;
            case voi:
                if (*current == 'd') state = final_void; 
                else return identifier;
                break;
            case final_void:
                return (*current == '\0') ? void_kw : identifier;

            default:
                return identifier;
        }
        current++;
    }

    // Handle end of string for each final state
    switch (state) {
        case final_bool: return bool_kw;
        case final_char: return char_kw;
        case final_else: return else_kw;
        case final_false: return false_kw;
        case final_float: return float_kw;
        case final_for: return for_kw;
        case final_if: return if_kw;
        case final_int: return int_kw;
        case final_printf: return printf_kw;
        case final_return: return return_kw;
        case final_scanf: return scanf_kw;
        case final_true: return true_kw;
        case final_while: return while_kw;
        case final_void: return void_kw;
        default: return identifier;
    }
}

/******************************************************/
/* character_literal - reads the character literal */
void character_literal_function()
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
            next_token = error_invalid_character;
            printf("Error - unterminated character literal\n");
            return;
        }
    }
    else if (current_char == '\'' || current_char == EOF) { // Handle empty or malformed character literals
        next_token = error_invalid_character;
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
        next_token = character_literal;
        set_token_end_column();
        current_char = get_char();
    }
    else { // Handle missing closing single quote
        next_token = error_invalid_character;
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
    next_token = token_eof;
    token_start_line = line_number;
    token_start_column = -1;
    token_end_column = -1;
}

/******************************************************/
/* string - reads the rest of the string literal */
void string_function()
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
        next_token = string;
        set_token_end_column();
        current_char = get_char();  // Advance to the next character
    }
    else {
        // Handle error for unterminated string
        printf("Error - unterminated string literal\n");
        next_token = error_invalid_character;
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