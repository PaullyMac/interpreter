#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"

// Global variables to store the token list and the current token index
Token *tokens;
int current_token = 0;
int num_tokens;
int indent_level = 0;
FILE *output_file;

// Function prototypes
Token *load_tokens(const char *filename, int *num_tokens);
void parse_program();
void parse_declaration();
void parse_function_declaration();
void parse_variable_declaration();
void parse_array_declaration();
void parse_data_type();
void parse_identifier();
void parse_parameter_list();
void parse_argument_list();
void parse_block();
void parse_block_item_list();
void parse_block_item();
void parse_statement();
void parse_return_statement();
void parse_const_statement();
void parse_while_statement();
void parse_for_statement();
void parse_exp();
void parse_const();
void match(TokenType type);
void print_indent();
char* get_data_type_string(TokenType type);
char* get_token_string(TokenType type);

int main(void) {
    tokens = load_tokens("symbol_table.txt", &num_tokens);
    if (tokens == NULL) {
        return 1;
    }

    output_file = fopen("parse_tree_output.ebnf", "w");
    if (output_file == NULL) {
        fprintf(stderr, "Error opening output file.\n");
        return 1;
    }

    parse_program();
    printf("Parsing successful!\n");

    fclose(output_file);
    free(tokens);
    return 0;
}

// Helper function to match the current token with the expected type
void match(TokenType type) {
    if (current_token < num_tokens && tokens[current_token].type == type) {
        current_token++;
    } else {
        fprintf(stderr, "Error: Expected token type %s but found %s at line %d\n",
               token_names[type], token_names[tokens[current_token].type],
               tokens[current_token].line_number);
        exit(1);
    }
}

void print_indent() {
    for (int i = 0; i < indent_level; i++) {
        fprintf(output_file, "  ");
    }
}

// <program> ::= { <declaration> }
void parse_program() {
    fprintf(output_file, "Program(\n");
    indent_level++;
    while (current_token < num_tokens && tokens[current_token].type != TOKEN_EOF) {
        parse_declaration();
        if (current_token < num_tokens && tokens[current_token].type != TOKEN_EOF) {
            fprintf(output_file, ",\n");
        } else {
            fprintf(output_file, "\n");
        }
    }
    indent_level--;
    fprintf(output_file, ")\n");
}

// <declaration> ::= <variable_declaration> | <array_declaration> | <function_declaration>
void parse_declaration() {
    print_indent();
    fprintf(output_file, "Declaration(\n");
    indent_level++;
    
    if (current_token < num_tokens && (tokens[current_token].type == INT || tokens[current_token].type == FLOAT ||
                                       tokens[current_token].type == CHAR || tokens[current_token].type == BOOL)) {
        if (current_token + 1 < num_tokens && tokens[current_token+1].type == IDENTIFIER && current_token + 2 < num_tokens && tokens[current_token+2].type == LEFT_PARENTHESIS)
        {
            parse_function_declaration();
        }
        
        else if(current_token + 1 < num_tokens && tokens[current_token+1].type == IDENTIFIER && current_token + 2 < num_tokens && tokens[current_token+2].type == LEFT_BRACKET)
        {
            parse_array_declaration();
        }
        else{
            parse_variable_declaration();
            fprintf(output_file, "\n");
        }
        
    } else {
        fprintf(output_file, "Error: Invalid declaration at line %d\n", tokens[current_token].line_number);
        exit(1);
    }
    indent_level--;
    print_indent();
    fprintf(output_file, ")");
}

// <variable_declaration> ::= <data_type> <identifier> [ “=” <exp> ] “;”
//                         | <data_type> <identifier> { “,” <identifier> } “;”
void parse_variable_declaration(){
    print_indent();
    fprintf(output_file, "Variable_Declaration(\n");
    indent_level++;
    parse_data_type();
    fprintf(output_file, ",\n");

    parse_identifier();
    if (current_token < num_tokens && tokens[current_token].type == SEMICOLON) {
        fprintf(output_file, ",\n");
        print_indent();

        fprintf(output_file, "%s\n", get_token_string(tokens[current_token].type));
        match(SEMICOLON);
    } else if (current_token < num_tokens && tokens[current_token].type == ASSIGN) {
        fprintf(output_file, ",\n");
        print_indent();
        fprintf(output_file, "%s,\n", get_token_string(tokens[current_token].type));
        match(ASSIGN);
        parse_const();
        fprintf(output_file, ",\n");
        print_indent();
        fprintf(output_file, "%s\n", get_token_string(tokens[current_token].type));
        match(SEMICOLON);
    } else if (current_token < num_tokens && tokens[current_token].type == COMMA) {
        while (current_token < num_tokens && tokens[current_token].type == COMMA) {
            fprintf(output_file, ",\n");
            print_indent();

            fprintf(output_file, "%s,\n", get_token_string(tokens[current_token].type));
            match(COMMA);
            parse_identifier();
        }
        fprintf(output_file, ",\n");
        print_indent();
        fprintf(output_file, "%s\n", get_token_string(tokens[current_token].type));
        match(SEMICOLON);
    } else {
        fprintf(output_file, "Error: Invalid variable declaration at line %d\n", tokens[current_token].line_number);
        exit(1);
    }
    indent_level--;
    print_indent();
    fprintf(output_file, ")");
}

// <array_declaration> ::= <data_type> <identifier> “[“ [<const>]  “]”  [ “=” “{“ <argument_list>“}” ] “;”
void parse_array_declaration(){
    print_indent();
    fprintf(output_file, "Array_Declaration(\n");
    indent_level++;
    parse_data_type();
    fprintf(output_file, ",\n");
    parse_identifier();
    fprintf(output_file, ",\n");
    print_indent();

    fprintf(output_file, "%s", get_token_string(tokens[current_token].type));
    match(LEFT_BRACKET);
    fprintf(output_file, ",\n");

    // Use parse_const to handle the optional <const>
    if (current_token < num_tokens && (tokens[current_token].type == NUMBER || 
                                       tokens[current_token].type == CHARACTER_LITERAL ||
                                       tokens[current_token].type == TRUE ||
                                       tokens[current_token].type == FALSE)) {
        parse_const();
        fprintf(output_file, ",\n"); // Add comma after parsing the constant
    }

    print_indent();
    fprintf(output_file, "%s", get_token_string(tokens[current_token].type));
    match(RIGHT_BRACKET);

    if (current_token < num_tokens && tokens[current_token].type == ASSIGN) {
        fprintf(output_file, ",\n");
        print_indent();
        fprintf(output_file, "%s,\n", get_token_string(tokens[current_token].type));
        match(ASSIGN);
        //fprintf(output_file, ",\n");

        print_indent();
        fprintf(output_file, "%s,\n", get_token_string(tokens[current_token].type));
        match(LEFT_BRACE);

        parse_argument_list();

        fprintf(output_file, ",\n");
        print_indent();
        fprintf(output_file, "%s", get_token_string(tokens[current_token].type));
        match(RIGHT_BRACE);
    }
    fprintf(output_file, ",\n");
    print_indent();
    fprintf(output_file, "%s\n", get_token_string(tokens[current_token].type));
    match(SEMICOLON);
    indent_level--;
    print_indent();
    fprintf(output_file, ")\n");
}

// <function_declaration> ::= <data_type> <identifier> "(" <parameter_list> ")" ( <block> | “;” )
void parse_function_declaration() {
    print_indent();
    fprintf(output_file, "Function_Declaration(\n");
    indent_level++;

    parse_data_type();
    fprintf(output_file, ",\n");

    parse_identifier();
    fprintf(output_file, ",\n");

    print_indent();
    fprintf(output_file, "%s,\n", get_token_string(tokens[current_token].type));
    match(LEFT_PARENTHESIS);
    
    parse_parameter_list();

    if (current_token < num_tokens && tokens[current_token].type == RIGHT_PARENTHESIS){
        fprintf(output_file, ",\n");
        print_indent();
        fprintf(output_file, "%s,\n", get_token_string(tokens[current_token].type));

        match(RIGHT_PARENTHESIS);
    }

    if (current_token < num_tokens && tokens[current_token].type == LEFT_BRACE) {
        parse_block();
        fprintf(output_file, "\n");
    } else {
        //fprintf(output_file, ",\n");
        print_indent();
        fprintf(output_file, "%s\n", get_token_string(tokens[current_token].type));

        match(SEMICOLON);
    }

    indent_level--;
    print_indent();
    fprintf(output_file, ")\n");
}

// <parameter_list> ::= [“void”] 
//   | <data_type> <identifier> {"," <data_type> <identifier>}
void parse_parameter_list() {
    print_indent();
    fprintf(output_file, "Parameter_List(\n");
    indent_level++;

    // Check if the parameter list is empty
    if (current_token < num_tokens && tokens[current_token].type == RIGHT_PARENTHESIS) {
        // Empty parameter list, do nothing
    } else {
        // Non-empty parameter list
        // Parse the first parameter
        if (current_token < num_tokens && (tokens[current_token].type == INT || tokens[current_token].type == FLOAT ||
                                           tokens[current_token].type == CHAR || tokens[current_token].type == BOOL)) {
            parse_data_type();
            fprintf(output_file, ",\n");
            parse_identifier();

            // Parse subsequent parameters
            while (current_token < num_tokens && tokens[current_token].type == COMMA) {
                match(COMMA);
                fprintf(output_file, ",\n");

                // Expect a data type for each subsequent parameter
                if (current_token < num_tokens && (tokens[current_token].type == INT || tokens[current_token].type == FLOAT ||
                                                   tokens[current_token].type == CHAR || tokens[current_token].type == BOOL)) {
                    parse_data_type();
                    fprintf(output_file, ",\n");
                    parse_identifier();
                } else {
                    // Error: Expected data type after comma in parameter list
                    fprintf(stderr, "Error: Expected data type after comma in parameter list at line %d\n", tokens[current_token].line_number);
                    exit(1);
                }
            }
        } else {
            // Error: Expected data type or ')' at the start of parameter list
            fprintf(stderr, "Error: Expected data type or ')' at the start of parameter list at line %d\n", tokens[current_token].line_number);
            exit(1);
        }
    }

    fprintf(output_file, "\n");
    indent_level--;
    print_indent();
    fprintf(output_file, ")");
}

// <data_type> ::= “int” | “float” | “char” | “bool”
void parse_data_type() {
    
    print_indent();
    fprintf(output_file, "Data_Type(%s)", get_token_string(tokens[current_token].type));
    if (!(current_token < num_tokens && (tokens[current_token].type == INT || tokens[current_token].type == FLOAT ||
                                       tokens[current_token].type == CHAR || tokens[current_token].type == BOOL))) {
        fprintf(output_file, "Error: Expected data type at line %d\n", tokens[current_token].line_number);
        exit(1);
    }
    match(tokens[current_token].type);
}

char* get_data_type_string(TokenType type) {
    switch (type) {
        case INT:
            return strdup("INT");
        case FLOAT:
            return strdup("FLOAT");
        case CHAR:
            return strdup("CHAR");
        case BOOL:
            return strdup("BOOL");
        default:
            return strdup("");
    }
}

char* get_token_string(TokenType type) {
    switch (type) {
        case LEFT_PARENTHESIS:
            return strdup("LEFT_PARENTHESIS");
        case RIGHT_PARENTHESIS:
            return strdup("RIGHT_PARENTHESIS");
        case LEFT_BRACKET:
            return strdup("LEFT_BRACKET");
        case RIGHT_BRACKET:
            return strdup("RIGHT_BRACKET");
        case LEFT_BRACE:
            return strdup("LEFT_BRACE");
        case RIGHT_BRACE:
            return strdup("RIGHT_BRACE");
        case COMMA:
            return strdup("COMMA");
        case SEMICOLON:
            return strdup("SEMICOLON");
        case MULTIPLY:
            return strdup("MULTIPLY");
        case EXPONENT:
            return strdup("EXPONENT");
        case PLUS:
            return strdup("PLUS");
        case MINUS:
            return strdup("MINUS");
        case DIVIDE:
            return strdup("DIVIDE");
        case INCREMENT:
            return strdup("INCREMENT");
        case DECREMENT:
            return strdup("DECREMENT");
        case EQUAL:
            return strdup("EQUAL");
        case NOT_EQUAL:
            return strdup("NOT_EQUAL");
        case ASSIGN:
            return strdup("ASSIGN");
        case LESS:
            return strdup("LESS");
        case LESS_EQUAL:
            return strdup("LESS_EQUAL");
        case GREATER:
            return strdup("GREATER");
        case GREATER_EQUAL:
            return strdup("GREATER_EQUAL");
        case NOT:
            return strdup("NOT");
        case OR:
            return strdup("OR");
        case AND:
            return strdup("AND");
        case COMMENT:
            return strdup("COMMENT");
        case MODULO:
            return strdup("MODULO");
        case FORMAT:
            return strdup("FORMAT");
        case IDENTIFIER:
            return (char *)tokens[current_token].lexeme;
        case STRING:
            return (char *)tokens[current_token].lexeme;
        case NUMBER:
            return (char *)tokens[current_token].lexeme;
        case CHARACTER_LITERAL:
            return (char *)tokens[current_token].lexeme;
        case CHAR:
            return strdup("CHAR");
        case INT:
            return strdup("INT");
        case FLOAT:
            return strdup("FLOAT");
        case BOOL:
            return strdup("BOOL");
        case IF:
            return strdup("IF");
        case ELSE:
            return strdup("ELSE");
        case FOR:
            return strdup("FOR");
        case WHILE:
            return strdup("WHILE");
        case RETURN:
            return strdup("RETURN");
        case PRINTF:
            return strdup("PRINTF");
        case SCANF:
            return strdup("SCANF");
        case TRUE:
            return strdup("TRUE");
        case FALSE:
            return strdup("FALSE");
        case ERROR_INVALID_CHARACTER:
            return strdup("ERROR_INVALID_CHARACTER");
        case ERROR_INVALID_IDENTIFIER:
            return strdup("ERROR_INVALID_IDENTIFIER");
        case TOKEN_EOF:
            return strdup("TOKEN_EOF");
        default:
            return strdup("");
    }
}

// <identifier> ::= identifier token
void parse_identifier() {
    print_indent();
    fprintf(output_file, "IDENTIFIER(\"%s\")", tokens[current_token].lexeme);
    match(IDENTIFIER);
}

// <block> ::= "{" <block-item-list>  "}"
void parse_block() {
    print_indent();
    fprintf(output_file, "Block(\n");
    indent_level++;
    print_indent();

    fprintf(output_file, "%s,\n", get_token_string(tokens[current_token].type));
    match(LEFT_BRACE);

    parse_block_item_list();
    fprintf(output_file, ",\n");

    print_indent();
    fprintf(output_file, "%s\n", get_token_string(tokens[current_token].type));
    match(RIGHT_BRACE);
    indent_level--;
    print_indent();
    fprintf(output_file, ")");
}

// <block_item_list> ::= (<block_item_list> <block_item>) | <block_item>
void parse_block_item_list() {
    print_indent();
    fprintf(output_file, "Block_Item_List(\n");
    indent_level++;
    while (current_token < num_tokens && tokens[current_token].type != RIGHT_BRACE) {
        parse_block_item();
        if (current_token < num_tokens && tokens[current_token].type == RIGHT_BRACE) {
            break;
        } else {
            fprintf(output_file, ",\n");
        }
    }
    fprintf(output_file, "\n");
    indent_level--;
    print_indent();
    fprintf(output_file, ")");
}

// <block_item> ::= <statement> | <variable_declaration> | <array_declaration>
void parse_block_item() {
    print_indent();
    fprintf(output_file, "Block_Item(\n");
    indent_level++;

    // Check if the current token could start a variable or array declaration
    if (current_token < num_tokens && (tokens[current_token].type == INT || tokens[current_token].type == FLOAT ||
                                       tokens[current_token].type == CHAR || tokens[current_token].type == BOOL)) {
        // Lookahead to check for identifier and then '[' or not
        if (current_token + 1 < num_tokens && tokens[current_token + 1].type == IDENTIFIER) {
            if (current_token + 2 < num_tokens && tokens[current_token + 2].type == LEFT_BRACKET) {
                parse_array_declaration();
            } else {
                parse_variable_declaration();
                fprintf(output_file, "\n");
            }
        } else {
            // Error: Expected identifier after data type in declaration
            fprintf(output_file, "Error: Expected identifier after data type at line %d\n", tokens[current_token].line_number);
            exit(1);
        }
    } else {
        // Otherwise, assume it's a statement
        parse_statement();
    }

    indent_level--;
    print_indent();
    fprintf(output_file, ")");
}

// <statement> ::= "return" <const> ;" | <const> ";" | ";" 
void parse_statement() {
    print_indent();
    fprintf(output_file, "Statement(\n");
    indent_level++;

    if (current_token < num_tokens && tokens[current_token].type == RETURN) {
        parse_return_statement();
    } else if (current_token < num_tokens && (tokens[current_token].type == NUMBER ||
                tokens[current_token].type == CHARACTER_LITERAL ||
                tokens[current_token].type == TRUE ||
                tokens[current_token].type == FALSE)) {
        parse_const_statement();
        
    } else if (current_token < num_tokens && tokens[current_token].type == WHILE) {
        parse_while_statement();
    } else if (current_token < num_tokens && tokens[current_token].type == FOR) {
        parse_for_statement();
    } else if (current_token < num_tokens && tokens[current_token].type == SEMICOLON) {
        print_indent();
        fprintf(output_file, "%s\n", get_token_string(tokens[current_token].type));
        match(SEMICOLON);
    } else if (current_token < num_tokens && tokens[current_token].type == LEFT_BRACE) {
        parse_block();
        fprintf(output_file, "\n");
    }
    else {
        fprintf(stderr, "Error: Invalid statement at line %d\n", tokens[current_token].line_number);
        fprintf(output_file, "Error: Invalid statement at line %d\n", tokens[current_token].line_number);
        exit(1);
    }

    indent_level--;
    print_indent();
    fprintf(output_file, ")\n");
}

// Implement last and only use <const> in place of <exp> for now
void parse_exp() {
    match(NUMBER);
}

Token *load_tokens(const char *filename, int *num_tokens) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error opening symbol table file: %s\n", filename);
        exit(1);
    }

    Token *token_list = (Token *)malloc(sizeof(Token) * MAX_TOKENS);
    if (token_list == NULL) {
        fprintf(stderr, "Error: Memory allocation failed!\n");
        exit(1);
    }

    *num_tokens = 0;

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        int token_code;
        char token_name[50];
        char lexeme_val[MAX_LEXEME_LENGTH];
        int line_num, col_num;

        if (sscanf(line, "%d | %s | %d | %d | %s", &token_code, token_name, &line_num, &col_num, lexeme_val) == 5) {
            token_list[*num_tokens].type = token_code;
            strcpy(token_list[*num_tokens].lexeme, lexeme_val);
            token_list[*num_tokens].line_number = line_num;
            token_list[*num_tokens].column_number = col_num;

            (*num_tokens)++;

            if (*num_tokens >= MAX_TOKENS) {
                fprintf(stderr, "Error: Maximum number of tokens exceeded!\n");
                exit(1);
            }
        }
    }

    fclose(fp);
    return token_list;
}

void parse_argument_list() {
    print_indent();
    fprintf(output_file, "Argument_List(\n");
    indent_level++;

    // Parse the first constant (updated to use parse_const)
    if (current_token < num_tokens && (tokens[current_token].type == NUMBER ||
                                       tokens[current_token].type == CHARACTER_LITERAL ||
                                       tokens[current_token].type == TRUE ||
                                       tokens[current_token].type == FALSE)) {
        parse_const();

        // Parse subsequent constants separated by commas (updated to use parse_const)
        while (current_token < num_tokens && tokens[current_token].type == COMMA) {
            fprintf(output_file, ",\n");
            match(COMMA);

            if (current_token < num_tokens && (tokens[current_token].type == NUMBER ||
                                               tokens[current_token].type == CHARACTER_LITERAL ||
                                               tokens[current_token].type == TRUE ||
                                               tokens[current_token].type == FALSE)) {
                parse_const();
            } else {
                // Error: Expected a constant after comma in argument list
                fprintf(stderr, "Error: Expected a constant after comma in argument list at line %d\n", tokens[current_token].line_number);
                fprintf(output_file, "Error: Expected a constant after comma in argument list\n");
                exit(1);
            }
        }
    }

    fprintf(output_file, "\n");
    indent_level--;
    print_indent();
    fprintf(output_file, ")");
}

// Function to parse a constant: <const> ::= <int> | <float> | <char> | <bool>
void parse_const() {
    print_indent();
    fprintf(output_file, "Constant(");

    if (current_token < num_tokens && tokens[current_token].type == NUMBER) {
        // Could be either int or float, based on our simplification
        fprintf(output_file, "%s", tokens[current_token].lexeme);
        match(NUMBER);
    } else if (current_token < num_tokens && tokens[current_token].type == CHARACTER_LITERAL) {
        fprintf(output_file, "%s", tokens[current_token].lexeme);
        match(CHARACTER_LITERAL);
    } else if (current_token < num_tokens && (tokens[current_token].type == TRUE || tokens[current_token].type == FALSE)) {
        fprintf(output_file, "%s", tokens[current_token].type == TRUE ? "true" : "false");
        match(tokens[current_token].type); // Match either TRUE or FALSE
    } else {
        fprintf(stderr, "Error: Expected a constant (int, float, char, or bool) at line %d\n", tokens[current_token].line_number);
        fprintf(output_file, "Error: Expected a constant (int, float, char, or bool))");
        exit(1);
    }

    fprintf(output_file, ")");
}

// Function to parse a return statement: "return" <const> ";"
void parse_return_statement() {
    print_indent();
    fprintf(output_file, "Return_Statement(\n");
    indent_level++;
    print_indent();
    fprintf(output_file, "%s,\n", get_token_string(tokens[current_token].type));
    match(RETURN);
    // Assuming <exp> is simplified to <const>
    parse_const();
    fprintf(output_file, ",\n");
    print_indent();
    fprintf(output_file, "%s\n", get_token_string(tokens[current_token].type));
    match(SEMICOLON);
    indent_level--;
    print_indent();
    fprintf(output_file, ")\n");
}

// Function to parse a constant statement: <const> ";"
void parse_const_statement() {
    print_indent();
    fprintf(output_file, "Const_Statement(\n");
    indent_level++;
    // Assuming <exp> is simplified to <const> which is a NUMBER
    parse_const();
    fprintf(output_file, ",\n");
    print_indent();
    fprintf(output_file, "%s\n", get_token_string(tokens[current_token].type));
    match(SEMICOLON);
    indent_level--;
    print_indent();
    fprintf(output_file, ")\n");
}

// Function to parse a while statement: "while" "(" <const> ")" <block>
void parse_while_statement() {
    print_indent();
    fprintf(output_file, "While_Statement(\n");
    indent_level++;

    print_indent();
    fprintf(output_file, "%s,\n", get_token_string(tokens[current_token].type));
    match(WHILE);

    print_indent();
    fprintf(output_file, "%s,\n", get_token_string(tokens[current_token].type));
    match(LEFT_PARENTHESIS);

    // Parse the condition (simplified to <const>)
    parse_const();
    fprintf(output_file, ",\n");

    print_indent();
    fprintf(output_file, "%s,\n", get_token_string(tokens[current_token].type));
    match(RIGHT_PARENTHESIS);

    // Parse the block
    parse_block();
    fprintf(output_file, "\n");

    indent_level--;
    print_indent();
    fprintf(output_file, ")\n");
}

// Function to parse a for loop statement
void parse_for_statement() {
    print_indent();
    fprintf(output_file, "For_Statement(\n");
    indent_level++;

    match(FOR);
    match(LEFT_PARENTHESIS);

    // Parse the initialization part
    if (current_token < num_tokens && (tokens[current_token].type == INT || tokens[current_token].type == FLOAT ||
                                       tokens[current_token].type == CHAR || tokens[current_token].type == BOOL)) {
        // Check if it's a variable or array declaration
        if (current_token + 1 < num_tokens && tokens[current_token + 1].type == IDENTIFIER) {
            if (current_token + 2 < num_tokens && tokens[current_token + 2].type == LEFT_BRACKET) {
                parse_array_declaration();
            } else {
                parse_variable_declaration();
                fprintf(output_file, ",\n");
            }
        } else {
            // Error: Expected identifier after data type
            fprintf(stderr, "Error: Expected identifier after data type in for loop initialization at line %d\n", tokens[current_token].line_number);
            exit(1);
        }
    } else {
        // It can also be a <const>
        parse_const_statement();
        fprintf(output_file, ",\n");
        match(SEMICOLON);
    }

    // Parse the condition part
    parse_const();
    fprintf(output_file, ",\n");

    match(SEMICOLON);

    // Parse the increment/decrement part
    parse_const();
    fprintf(output_file, ",\n");
    
    match(RIGHT_PARENTHESIS);

    // Parse the block
    parse_block();
    fprintf(output_file, "\n");

    indent_level--;
    print_indent();
    fprintf(output_file, ")\n");
}