#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include "token.h"
#include "scanner.h"


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
                if (isalpha(current_char)) {
                    identifier();
                    next_token = keywords(lexeme);
                } else if (isdigit(current_char)) {
                    number();
                } else {
                    printf("ERROR - invalid char %c\n", current_char);
                    next_token = ERROR_INVALID_CHARACTER;
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
        next_token = keywords(lexeme);
    }

    set_token_end_column();
}

/******************************************************/
/* keywords - a function to check if the lexeme is a keyword */
TokenType keywords(char *lexeme) {
    char *current = lexeme;
    KeywordState state = S;

    while (*current != '\0') {
        switch (state) {
            case S:
                switch (*current) {
                    case 'b': state = B; break; // Possible 'bool'
                    case 'c': state = C; break; // Possible 'char'
                    case 'e': state = E; break; // Possible 'else'
                    case 'f': state = F; break; // Possible 'false', 'float', 'for'
                    case 'i': state = I; break; // Possible 'if', 'int'
                    case 'p': state = P; break; // Possible 'printf'
                    case 'r': state = R; break; // Possible 'return'
                    case 's': state = S1; break; // Possible 'scanf'
                    case 't': state = T; break; // Possible 'true'
                    case 'w': state = W; break; // Possible 'while'
                    case 'v': state = V; break; // Possible 'void'
                    default: return IDENTIFIER; // Not a keyword
                }
                break;

            // States for 'bool'
            case B:
                if (*current == 'o') state = BO; else return IDENTIFIER;
                break;
            case BO:
                if (*current == 'o') state = BOO; else return IDENTIFIER;
                break;
            case BOO:
                if (*current == 'l') state = FINAL_BOOL; else return IDENTIFIER;
                break;
            case FINAL_BOOL:
                return (*current == '\0') ? BOOL : IDENTIFIER;

            // States for 'char'
            case C:
                if (*current == 'h') state = CH; else return IDENTIFIER;
                break;
            case CH:
                if (*current == 'a') state = CHA; else return IDENTIFIER;
                break;
            case CHA:
                if (*current == 'r') state = FINAL_CHAR; else return IDENTIFIER;
                break;
            case FINAL_CHAR:
                return (*current == '\0') ? CHAR : IDENTIFIER;

            // States for 'else'
            case E:
                if (*current == 'l') state = EL; else return IDENTIFIER;
                break;
            case EL:
                if (*current == 's') state = ELS; else return IDENTIFIER;
                break;
            case ELS:
                if (*current == 'e') state = FINAL_ELSE; else return IDENTIFIER;
                break;
            case FINAL_ELSE:
                return (*current == '\0') ? ELSE : IDENTIFIER;

            // States for 'false', 'float', 'for'
            case F:
                if (*current == 'a') state = FA;  // Possible 'false'
                else if (*current == 'l') state = FL; // Possible 'float'
                else if (*current == 'o') state = FO; // Possible 'for'
                else return IDENTIFIER;
                break;
            case FA:
                if (*current == 'l') state = FAL; else return IDENTIFIER;
                break;
            case FAL:
                if (*current == 's') state = FALS; else return IDENTIFIER;
                break;
            case FALS:
                if (*current == 'e') state = FINAL_FALSE; else return IDENTIFIER;
                break;
            case FINAL_FALSE:
                return (*current == '\0') ? FALSE : IDENTIFIER;

            case FL:
                if (*current == 'o') state = FLO; else return IDENTIFIER;
                break;
            case FLO:
                if (*current == 'a') state = FLOA; else return IDENTIFIER;
                break;
            case FLOA:
                if (*current == 't') state = FINAL_FLOAT; else return IDENTIFIER;
                break;
            case FINAL_FLOAT:
                return (*current == '\0') ? FLOAT : IDENTIFIER;

            case FO:
                if (*current == 'r') state = FINAL_FOR; else return IDENTIFIER;
                break;
            case FINAL_FOR:
                return (*current == '\0') ? FOR : IDENTIFIER;

            // States for 'if', 'int'
            case I:
                if (*current == 'f') state = FINAL_IF;
                else if (*current == 'n') state = IN;
                else return IDENTIFIER;
                break;
            case FINAL_IF:
                return (*current == '\0') ? IF : IDENTIFIER;

            case IN:
                if (*current == 't') state = FINAL_INT; else return IDENTIFIER;
                break;
            case FINAL_INT:
                return (*current == '\0') ? INT : IDENTIFIER;

            // States for 'printf'
            case P:
                if (*current == 'r') state = PR; else return IDENTIFIER;
                break;
            case PR:
                if (*current == 'i') state = PRI; else return IDENTIFIER;
                break;
            case PRI:
                if (*current == 'n') state = PRIN; else return IDENTIFIER;
                break;
            case PRIN:
                if (*current == 't') state = PRINT; else return IDENTIFIER;
                break;
            case PRINT:
                if (*current == 'f') state = FINAL_PRINTF; else return IDENTIFIER;
                break;
            case FINAL_PRINTF:
                return (*current == '\0') ? PRINTF : IDENTIFIER;

            // States for 'return'
            case R:
                if (*current == 'e') state = RE; else return IDENTIFIER;
                break;
            case RE:
                if (*current == 't') state = RET; else return IDENTIFIER;
                break;
            case RET:
                if (*current == 'u') state = RETU; else return IDENTIFIER;
                break;
            case RETU:
                if (*current == 'r') state = RETUR; else return IDENTIFIER;
                break;
            case RETUR:
                if (*current == 'n') state = FINAL_RETURN; else return IDENTIFIER;
                break;
            case FINAL_RETURN:
                return (*current == '\0') ? RETURN : IDENTIFIER;

            // States for 'scanf'
            case S1:
                if (*current == 'c') state = SC; else return IDENTIFIER;
                break;
            case SC:
                if (*current == 'a') state = SCA; else return IDENTIFIER;
                break;
            case SCA:
                if (*current == 'n') state = SCAN; else return IDENTIFIER;
                break;
            case SCAN:
                if (*current == 'f') state = FINAL_SCANF; else return IDENTIFIER;
                break;
            case FINAL_SCANF:
                return (*current == '\0') ? SCANF : IDENTIFIER;

            // States for 'true'
            case T:
                if (*current == 'r') state = TR; else return IDENTIFIER;
                break;
            case TR:
                if (*current == 'u') state = TRU; else return IDENTIFIER;
                break;
            case TRU:
                if (*current == 'e') state = FINAL_TRUE; else return IDENTIFIER;
                break;
            case FINAL_TRUE:
                return (*current == '\0') ? TRUE : IDENTIFIER;

            // States for 'while'
            case W:
                if (*current == 'h') state = WH; else return IDENTIFIER;
                break;
            case WH:
                if (*current == 'i') state = WHI; else return IDENTIFIER;
                break;
            case WHI:
                if (*current == 'l') state = WHIL; else return IDENTIFIER;
                break;
            case WHIL:
                if (*current == 'e') state = FINAL_WHILE; else return IDENTIFIER;
                break;
            case FINAL_WHILE:
                return (*current == '\0') ? WHILE : IDENTIFIER;

            // States for 'void'
            case V:
                if (*current == 'o') state = VO; 
                else return IDENTIFIER;
                break;
            case VO:
                if (*current == 'i') state = VOI; 
                else return IDENTIFIER;
                break;
            case VOI:
                if (*current == 'd') state = FINAL_VOID; 
                else return IDENTIFIER;
                break;
            case FINAL_VOID:
                return (*current == '\0') ? VOID : IDENTIFIER;

            default:
                return IDENTIFIER;
        }
        current++;
    }

    // Handle end of string for each final state
    switch (state) {
        case FINAL_BOOL: return BOOL;
        case FINAL_CHAR: return CHAR;
        case FINAL_ELSE: return ELSE;
        case FINAL_FALSE: return FALSE;
        case FINAL_FLOAT: return FLOAT;
        case FINAL_FOR: return FOR;
        case FINAL_IF: return IF;
        case FINAL_INT: return INT;
        case FINAL_PRINTF: return PRINTF;
        case FINAL_RETURN: return RETURN;
        case FINAL_SCANF: return SCANF;
        case FINAL_TRUE: return TRUE;
        case FINAL_WHILE: return WHILE;
        case FINAL_VOID: return VOID;
        default: return IDENTIFIER;
    }
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