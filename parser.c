#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"

// Global variables to store the token list and the current token index
Token *tokens;
int current_token = 0;
int num_tokens;

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
void parse_block();
void parse_block_item_list();
void parse_block_item();
void parse_statement();
void parse_exp();
void match(TokenType type);

int main(void) {
    tokens = load_tokens("symbol_table.txt", &num_tokens);
    if (tokens == NULL) {
        return 1;
    }

    parse_program();
    printf("Parsing successful!\n");

    free(tokens);
    return 0;
}

// Helper function to match the current token with the expected type
void match(TokenType type) {
    if (current_token < num_tokens && tokens[current_token].type == type) {
        current_token++;
    } else {
        printf("Error: Expected token type %s but found %s at line %d\n",
               token_names[type], token_names[tokens[current_token].type],
               tokens[current_token].line_number);
        exit(1);
    }
}

// <program> ::= { <declaration> }
void parse_program() {
    while (current_token < num_tokens && tokens[current_token].type != TOKEN_EOF) {
        parse_declaration();
    }
}

// <declaration> ::= <variable_declaration> | <array_declaration> | <function_declaration>
void parse_declaration() {
    if (current_token < num_tokens && (tokens[current_token].type == INT || tokens[current_token].type == FLOAT ||
                                       tokens[current_token].type == CHAR || tokens[current_token].type == BOOL)) {
        parse_data_type();
        parse_identifier();

        if (current_token < num_tokens && tokens[current_token].type == LEFT_PARENTHESIS)
        {
            parse_function_declaration();
        }
        
        else if(current_token < num_tokens && tokens[current_token].type == LEFT_BRACKET)
        {
            parse_array_declaration();
        }
        else{
            parse_variable_declaration();
        }
        
    } else {
        printf("Error: Invalid declaration at line %d\n", tokens[current_token].line_number);
        exit(1);
    }
}

// <variable_declaration> ::= <data_type> <identifier> [ “=” <exp> ] “;”
//                         | <data_type> <identifier> { “,” <identifier> } “;”
void parse_variable_declaration(){
    if (current_token < num_tokens && tokens[current_token].type == SEMICOLON) {
        match(SEMICOLON);
    } else if (current_token < num_tokens && tokens[current_token].type == ASSIGN) {
        match(ASSIGN);
        parse_exp();
        match(SEMICOLON);
    } else if (current_token < num_tokens && tokens[current_token].type == COMMA) {
        while (current_token < num_tokens && tokens[current_token].type == COMMA) {
            match(COMMA);
            parse_identifier();
        }
        match(SEMICOLON);
    } else {
        printf("Error: Invalid variable declaration at line %d\n", tokens[current_token].line_number);
        exit(1);
    }
}

// <array_declaration> ::= <data_type> <identifier> “[“ [<const>]  “]”  [ “=” “{“ <argument_list>“}” ] “;”
void parse_array_declaration(){
    match(LEFT_BRACKET);
    if (current_token < num_tokens && tokens[current_token].type == NUMBER) {
        match(NUMBER);
    }
    match(RIGHT_BRACKET);
    if (current_token < num_tokens && tokens[current_token].type == ASSIGN) {
        match(ASSIGN);
        match(LEFT_BRACE);
        //parse_argument_list(); // You haven't implemented parse_argument_list yet
        match(RIGHT_BRACE);
    }
    match(SEMICOLON);
}

// <function_declaration> ::= <data_type> <identifier> "(" <parameter_list> ")" ( <block> | “;” )
void parse_function_declaration() {
    match(LEFT_PARENTHESIS);
    parse_parameter_list();
    match(RIGHT_PARENTHESIS);
    if (current_token < num_tokens && tokens[current_token].type == LEFT_BRACE) {
        parse_block();
    } else {
        match(SEMICOLON);
    }
}

// <parameter_list> ::= [“void”]
//   | <data_type> <identifier> {"," <data_type> <identifier>}
void parse_parameter_list() {
    // For now, we'll only handle the [“void”] case
    // You can expand this later to handle the full grammar
    // if (current_token < num_tokens && tokens[current_token].type == VOID) {
    //     match(VOID);
    // }
}

// <data_type> ::= “int” | “float” | “char” | “bool”
void parse_data_type() {
    if (current_token < num_tokens && (tokens[current_token].type == INT || tokens[current_token].type == FLOAT ||
                                       tokens[current_token].type == CHAR || tokens[current_token].type == BOOL)) {
        match(tokens[current_token].type);
    } else {
        printf("Error: Expected data type at line %d\n", tokens[current_token].line_number);
        exit(1);
    }
}

// <identifier> ::= identifier token
void parse_identifier() {
    match(IDENTIFIER);
}

// <block> ::= "{" <block-item-list>  "}"
void parse_block() {
    match(LEFT_BRACE);
    parse_block_item_list();
    match(RIGHT_BRACE);
}

// <block_item_list> ::= (<block_item_list> <block_item>) | <block_item>
void parse_block_item_list() {
    while (current_token < num_tokens && tokens[current_token].type != RIGHT_BRACE) {
        parse_block_item();
    }
}

// <block_item> ::= <statement> | <declaration>
void parse_block_item() {
    // Simplified for now, assuming only statements
    parse_statement();
}

// <statement> ::= ";"
void parse_statement() {
    match(SEMICOLON);
}

// <exp> ::= <int>
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