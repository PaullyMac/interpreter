// Parse one token at a time from the symbol_table.txt
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>

#include "token.h"

Token *load_tokens(const char *filename, int *num_tokens);
void print_tokens(Token *token_list, int num_tokens);

int main(int argc, char *argv[]) {
    int num_tokens;
    Token *token_list = load_tokens("symbol_table.txt", &num_tokens);

    print_tokens(token_list, num_tokens);

    return 0;
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

void print_tokens(Token *token_list, int num_tokens) {
    printf("Tokens:\n");
    for (int i = 0; i < num_tokens; i++) {
        printf("%-10d %-30s %-30s %-2d %-2d\n",
               token_list[i].type,
               token_names[token_list[i].type],
               token_list[i].lexeme,
               token_list[i].line_number,
               token_list[i].column_number);
    }
    printf("\n");
}