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
void parse_function();
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

// <program> ::= <function>
void parse_program() {
    parse_function();
}

// <function> ::= "int" <identifier> "(" "void" ")" "{" <statement> "}"
void parse_function() {
    match(INT);
    match(IDENTIFIER);
    match(LEFT_PARENTHESIS);
    match(RIGHT_PARENTHESIS);
    match(LEFT_BRACE);
    parse_statement();
    match(RIGHT_BRACE);
}

// <statement> ::= "return" <exp> ";"
void parse_statement() {
    match(RETURN);
    parse_exp();
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