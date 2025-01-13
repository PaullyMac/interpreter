#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "token.h"

// Data structure for the parse tree
typedef struct ParseTreeNode {
    char *name;
    Token *token;
    struct ParseTreeNode **children;
    int num_children;
} ParseTreeNode;

// Function prototypes for creating parse tree nodes
ParseTreeNode *create_program_node();
ParseTreeNode *create_declaration_node();
ParseTreeNode *create_function_declaration_node();
ParseTreeNode *create_variable_declaration_node();
ParseTreeNode *create_array_declaration_node();
ParseTreeNode *create_data_type_node();
ParseTreeNode *create_identifier_node();
ParseTreeNode *create_parameter_list_node();
ParseTreeNode *create_argument_list_node();
ParseTreeNode *create_block_node();
ParseTreeNode *create_block_item_list_node();
ParseTreeNode *create_block_item_node();
ParseTreeNode *create_statement_node();
ParseTreeNode *create_return_statement_node();
ParseTreeNode *create_const_statement_node();
ParseTreeNode *create_while_statement_node();
ParseTreeNode *create_for_statement_node();
ParseTreeNode *create_exp_node();
ParseTreeNode *create_const_node();

// Function prototypes for printing the parse tree
void print_parse_tree(ParseTreeNode *node, int indent_level);
void print_indent(int indent_level);

// Global variables to store the token list and the current token index
Token *tokens;
int current_token = 0;
int num_tokens;

// Global file pointer for the output file
FILE *output_file;

// Function prototypes
Token *load_tokens(const char *filename, int *num_tokens);
ParseTreeNode *parse_program();
ParseTreeNode *parse_declaration();
ParseTreeNode *parse_function_declaration();
ParseTreeNode *parse_variable_declaration();
ParseTreeNode *parse_array_declaration();
ParseTreeNode *parse_data_type();
Token parse_identifier();
ParseTreeNode *parse_parameter_list();
ParseTreeNode *parse_argument_list();
ParseTreeNode *parse_block();
ParseTreeNode *parse_block_item_list();
ParseTreeNode *parse_block_item();
ParseTreeNode *parse_statement();
ParseTreeNode *parse_return_statement();
ParseTreeNode *parse_const_statement();
ParseTreeNode *parse_while_statement();
ParseTreeNode *parse_for_statement();
ParseTreeNode *parse_exp();
ParseTreeNode *parse_const();
void match(TokenType type);

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

    ParseTreeNode *root = parse_program();
    printf("Parsing successful!\n");

    // Print the parse tree
    print_parse_tree(root, 0);
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

// <program> ::= { <declaration> }
ParseTreeNode *parse_program() {
    ParseTreeNode *node = create_program_node();
    while (current_token < num_tokens && tokens[current_token].type != TOKEN_EOF) {
        ParseTreeNode *declaration = parse_declaration();
        node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
        node->children[node->num_children++] = declaration;
    }
    return node;
}

// <declaration> ::= <variable_declaration> | <array_declaration> | <function_declaration>
ParseTreeNode *parse_declaration() {
    ParseTreeNode *node = create_declaration_node();
    if (current_token < num_tokens && (tokens[current_token].type == INT || tokens[current_token].type == FLOAT ||
                                       tokens[current_token].type == CHAR || tokens[current_token].type == BOOL)) {
        if (current_token + 1 < num_tokens && tokens[current_token+1].type == IDENTIFIER && current_token + 2 < num_tokens && tokens[current_token+2].type == LEFT_PARENTHESIS) {
            ParseTreeNode *function_declaration = parse_function_declaration();
            node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
            node->children[node->num_children++] = function_declaration;
        } else if(current_token + 1 < num_tokens && tokens[current_token+1].type == IDENTIFIER && current_token + 2 < num_tokens && tokens[current_token+2].type == LEFT_BRACKET) {
            ParseTreeNode *array_declaration = parse_array_declaration();
            node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
            node->children[node->num_children++] = array_declaration;
        } else {
            ParseTreeNode *variable_declaration = parse_variable_declaration();
            node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
            node->children[node->num_children++] = variable_declaration;
        }
    } else {
        fprintf(stderr, "Error: Invalid declaration at line %d\n", tokens[current_token].line_number);
        exit(1);
    }
    return node;
}

// <variable_declaration> ::= <data_type> <identifier> [ “=” <exp> ] “;”
//                         | <data_type> <identifier> { “,” <identifier> } “;”
ParseTreeNode *parse_variable_declaration() {
    ParseTreeNode *node = create_variable_declaration_node();
    ParseTreeNode *data_type = parse_data_type();
    node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
    node->children[node->num_children++] = data_type;

    Token identifier = parse_identifier();
    ParseTreeNode *identifier_node = create_identifier_node();
    identifier_node->token = malloc(sizeof(Token));
    *identifier_node->token = identifier;
    node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
    node->children[node->num_children++] = identifier_node;

    if (current_token < num_tokens && tokens[current_token].type == SEMICOLON) {
        match(SEMICOLON);
    } else if (current_token < num_tokens && tokens[current_token].type == ASSIGN) {
        match(ASSIGN);
        ParseTreeNode *const_node = parse_const();
        node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
        node->children[node->num_children++] = const_node;
        match(SEMICOLON);
    } else if (current_token < num_tokens && tokens[current_token].type == COMMA) {
        while (current_token < num_tokens && tokens[current_token].type == COMMA) {
            match(COMMA);
            Token identifier = parse_identifier();
            ParseTreeNode *identifier_node = create_identifier_node();
            identifier_node->token = malloc(sizeof(Token));
            *identifier_node->token = identifier;
            node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
            node->children[node->num_children++] = identifier_node;
        }
        match(SEMICOLON);
    } else {
        fprintf(stderr, "Error: Invalid variable declaration at line %d\n", tokens[current_token].line_number);
        exit(1);
    }
    return node;
}

// <array_declaration> ::= <data_type> <identifier> “[“ [<const>]  “]”  [ “=” “{“ <argument_list>“}” ] “;”
ParseTreeNode *parse_array_declaration() {
    ParseTreeNode *node = create_array_declaration_node();
    ParseTreeNode *data_type = parse_data_type();
    node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
    node->children[node->num_children++] = data_type;

    Token identifier = parse_identifier();
    ParseTreeNode *identifier_node = create_identifier_node();
    identifier_node->token = malloc(sizeof(Token));
    *identifier_node->token = identifier;
    node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
    node->children[node->num_children++] = identifier_node;

    match(LEFT_BRACKET);

    if (current_token < num_tokens && (tokens[current_token].type == INTEGER_LITERAL ||
                                       tokens[current_token].type == FLOAT_LITERAL ||
                                       tokens[current_token].type == CHARACTER_LITERAL ||
                                       tokens[current_token].type == TRUE ||
                                       tokens[current_token].type == FALSE)) {
        ParseTreeNode *const_node = parse_const();
        node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
        node->children[node->num_children++] = const_node;
    }

    match(RIGHT_BRACKET);

    if (current_token < num_tokens && tokens[current_token].type == ASSIGN) {
        match(ASSIGN);
        match(LEFT_BRACE);
        ParseTreeNode *argument_list = parse_argument_list();
        node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
        node->children[node->num_children++] = argument_list;
        match(RIGHT_BRACE);
    }

    match(SEMICOLON);
    return node;
}

// <function_declaration> ::= <data_type> <identifier> "(" <parameter_list> ")" ( <block> | “;” )
ParseTreeNode *parse_function_declaration() {
    ParseTreeNode *node = create_function_declaration_node();
    ParseTreeNode *data_type = parse_data_type();
    node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
    node->children[node->num_children++] = data_type;

    Token identifier = parse_identifier();
    ParseTreeNode *identifier_node = create_identifier_node();
    identifier_node->token = malloc(sizeof(Token));
    *identifier_node->token = identifier;
    node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
    node->children[node->num_children++] = identifier_node;

    match(LEFT_PARENTHESIS);
    ParseTreeNode *parameter_list = parse_parameter_list();
    node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
    node->children[node->num_children++] = parameter_list;
    match(RIGHT_PARENTHESIS);

    if (current_token < num_tokens && tokens[current_token].type == LEFT_BRACE) {
        ParseTreeNode *block = parse_block();
        node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
        node->children[node->num_children++] = block;
    } else {
        match(SEMICOLON);
    }
    return node;
}

// <parameter_list> ::= [“void”]
//   | <data_type> <identifier> {"," <data_type> <identifier>}
ParseTreeNode *parse_parameter_list() {
    ParseTreeNode *node = create_parameter_list_node();
    if (current_token < num_tokens && tokens[current_token].type == RIGHT_PARENTHESIS) {
        // Empty parameter list
    } else {
        if (current_token < num_tokens && (tokens[current_token].type == INT || tokens[current_token].type == FLOAT ||
                                           tokens[current_token].type == CHAR || tokens[current_token].type == BOOL)) {
            ParseTreeNode *data_type = parse_data_type();
            node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
            node->children[node->num_children++] = data_type;

            Token identifier = parse_identifier();
            ParseTreeNode *identifier_node = create_identifier_node();
            identifier_node->token = malloc(sizeof(Token));
            *identifier_node->token = identifier;
            node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
            node->children[node->num_children++] = identifier_node;

            while (current_token < num_tokens && tokens[current_token].type == COMMA) {
                match(COMMA);
                if (current_token < num_tokens && (tokens[current_token].type == INT || tokens[current_token].type == FLOAT ||
                                                   tokens[current_token].type == CHAR || tokens[current_token].type == BOOL)) {
                    ParseTreeNode *data_type = parse_data_type();
                    node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
                    node->children[node->num_children++] = data_type;

                    Token identifier = parse_identifier();
                    ParseTreeNode *identifier_node = create_identifier_node();
                    identifier_node->token = malloc(sizeof(Token));
                    *identifier_node->token = identifier;
                    node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
                    node->children[node->num_children++] = identifier_node;
                } else {
                    fprintf(stderr, "Error: Expected data type after comma in parameter list at line %d\n", tokens[current_token].line_number);
                    exit(1);
                }
            }
        } else {
            fprintf(stderr, "Error: Expected data type or ')' at the start of parameter list at line %d\n", tokens[current_token].line_number);
            exit(1);
        }
    }
    return node;
}

// <data_type> ::= “int” | “float” | “char” | “bool”
ParseTreeNode *parse_data_type() {
    ParseTreeNode *node = create_data_type_node();
    node->token = malloc(sizeof(Token));
    *node->token = tokens[current_token];

    if (!(current_token < num_tokens && (tokens[current_token].type == INT || tokens[current_token].type == FLOAT ||
                                       tokens[current_token].type == CHAR || tokens[current_token].type == BOOL))) {
        fprintf(stderr, "Error: Expected data type at line %d\n", tokens[current_token].line_number);
        exit(1);
    }
    match(tokens[current_token].type);
    return node;
}

// <identifier> ::= identifier token
Token parse_identifier() {
    Token identifier = tokens[current_token];
    match(IDENTIFIER);
    return identifier;
}

// <block> ::= "{" <block-item-list>  "}"
ParseTreeNode *parse_block() {
    ParseTreeNode *node = create_block_node();
    match(LEFT_BRACE);
    ParseTreeNode *block_item_list = parse_block_item_list();
    node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
    node->children[node->num_children++] = block_item_list;
    match(RIGHT_BRACE);
    return node;
}

// <block_item_list> ::= (<block_item_list> <block_item>) | <block_item>
ParseTreeNode *parse_block_item_list() {
    ParseTreeNode *node = create_block_item_list_node();
    while (current_token < num_tokens && tokens[current_token].type != RIGHT_BRACE) {
        ParseTreeNode *block_item = parse_block_item();
        node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
        node->children[node->num_children++] = block_item;
        if (current_token < num_tokens && tokens[current_token].type == RIGHT_BRACE) {
            break;
        }
    }
    return node;
}

// <block_item> ::= <statement> | <variable_declaration> | <array_declaration>
ParseTreeNode *parse_block_item() {
    ParseTreeNode *node = create_block_item_node();
    if (current_token < num_tokens && (tokens[current_token].type == INT || tokens[current_token].type == FLOAT ||
                                       tokens[current_token].type == CHAR || tokens[current_token].type == BOOL)) {
        if (current_token + 1 < num_tokens && tokens[current_token + 1].type == IDENTIFIER) {
            if (current_token + 2 < num_tokens && tokens[current_token + 2].type == LEFT_BRACKET) {
                ParseTreeNode *array_declaration = parse_array_declaration();
                node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
                node->children[node->num_children++] = array_declaration;
            } else {
                ParseTreeNode *variable_declaration = parse_variable_declaration();
                node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
                node->children[node->num_children++] = variable_declaration;
            }
        } else {
            fprintf(stderr, "Error: Expected identifier after data type in declaration at line %d\n", tokens[current_token].line_number);
            exit(1);
        }
    } else {
        ParseTreeNode *statement = parse_statement();
        node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
        node->children[node->num_children++] = statement;
    }
    return node;
}

// <statement> ::= "return" <const> ;" | <const> ";" | ";"
ParseTreeNode *parse_statement() {
    ParseTreeNode *node = create_statement_node();
    if (current_token < num_tokens && tokens[current_token].type == RETURN) {
        ParseTreeNode *return_statement = parse_return_statement();
        node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
        node->children[node->num_children++] = return_statement;
    } else if (current_token < num_tokens && (tokens[current_token].type == INTEGER_LITERAL ||
                tokens[current_token].type == FLOAT_LITERAL ||
                tokens[current_token].type == CHARACTER_LITERAL ||
                tokens[current_token].type == TRUE ||
                tokens[current_token].type == FALSE)) {
        ParseTreeNode *const_statement = parse_const_statement();
        node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
        node->children[node->num_children++] = const_statement;
    } else if (current_token < num_tokens && tokens[current_token].type == WHILE) {
        ParseTreeNode *while_statement = parse_while_statement();
        node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
        node->children[node->num_children++] = while_statement;
    } else if (current_token < num_tokens && tokens[current_token].type == FOR) {
        ParseTreeNode *for_statement = parse_for_statement();
        node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
        node->children[node->num_children++] = for_statement;
    } else if (current_token < num_tokens && tokens[current_token].type == SEMICOLON) {
        match(SEMICOLON);
    } else if (current_token < num_tokens && tokens[current_token].type == LEFT_BRACE) {
        ParseTreeNode *block = parse_block();
        node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
        node->children[node->num_children++] = block;
    } else {
        fprintf(stderr, "Error: Invalid statement at line %d\n", tokens[current_token].line_number);
        exit(1);
    }
    return node;
}

// Implement last and only use <const> in place of <exp> for now
ParseTreeNode *parse_exp() {
    return parse_const();
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

ParseTreeNode *parse_argument_list() {
    ParseTreeNode *node = create_argument_list_node();
    if (current_token < num_tokens && (tokens[current_token].type == INTEGER_LITERAL ||
                                       tokens[current_token].type == FLOAT_LITERAL ||
                                       tokens[current_token].type == CHARACTER_LITERAL ||
                                       tokens[current_token].type == TRUE ||
                                       tokens[current_token].type == FALSE)) {
        ParseTreeNode *const_node = parse_const();
        node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
        node->children[node->num_children++] = const_node;

        while (current_token < num_tokens && tokens[current_token].type == COMMA) {
            match(COMMA);
            if (current_token < num_tokens && (tokens[current_token].type == INTEGER_LITERAL ||
                                               tokens[current_token].type == FLOAT_LITERAL ||
                                               tokens[current_token].type == CHARACTER_LITERAL ||
                                               tokens[current_token].type == TRUE ||
                                               tokens[current_token].type == FALSE)) {
                ParseTreeNode *const_node = parse_const();
                node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
                node->children[node->num_children++] = const_node;
            } else {
                fprintf(stderr, "Error: Expected a constant after comma in argument list at line %d\n", tokens[current_token].line_number);
                exit(1);
            }
        }
    }
    return node;
}

// Function to parse a constant: <const> ::= <int> | <float> | <char> | <bool>
ParseTreeNode *parse_const() {
    ParseTreeNode *node = create_const_node();
    node->token = malloc(sizeof(Token));
    *node->token = tokens[current_token];

    if (current_token < num_tokens && tokens[current_token].type == INTEGER_LITERAL) {
        match(INTEGER_LITERAL);
    } else if (current_token < num_tokens && tokens[current_token].type == FLOAT_LITERAL) {
        match(FLOAT_LITERAL);
    } else if (current_token < num_tokens && tokens[current_token].type == CHARACTER_LITERAL) {
        match(CHARACTER_LITERAL);
    } else if (current_token < num_tokens && (tokens[current_token].type == TRUE || tokens[current_token].type == FALSE)) {
        match(tokens[current_token].type);
    } else {
        fprintf(stderr, "Error: Expected a constant (int, float, char, or bool) at line %d\n", tokens[current_token].line_number);
        exit(1);
    }
    return node;
}

// Function to parse a return statement: "return" <const> ";"
ParseTreeNode *parse_return_statement() {
    ParseTreeNode *node = create_return_statement_node();
    match(RETURN);
    ParseTreeNode *const_node = parse_const();
    node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
    node->children[node->num_children++] = const_node;
    match(SEMICOLON);
    return node;
}

// Function to parse a constant statement: <const> ";"
ParseTreeNode *parse_const_statement() {
    ParseTreeNode *node = create_const_statement_node();
    ParseTreeNode *const_node = parse_const();
    node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
    node->children[node->num_children++] = const_node;
    match(SEMICOLON);
    return node;
}

// Function to parse a while statement: "while" "(" <const> ")" <block>
ParseTreeNode *parse_while_statement() {
    ParseTreeNode *node = create_while_statement_node();
    match(WHILE);
    match(LEFT_PARENTHESIS);
    ParseTreeNode *const_node = parse_const();
    node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
    node->children[node->num_children++] = const_node;
    match(RIGHT_PARENTHESIS);
    ParseTreeNode *block = parse_block();
    node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
    node->children[node->num_children++] = block;
    return node;
}

// Function to parse a for loop statement
ParseTreeNode *parse_for_statement() {
    ParseTreeNode *node = create_for_statement_node();
    match(FOR);
    match(LEFT_PARENTHESIS);

    if (current_token < num_tokens && (tokens[current_token].type == INT || tokens[current_token].type == FLOAT ||
                                       tokens[current_token].type == CHAR || tokens[current_token].type == BOOL)) {
        if (current_token + 1 < num_tokens && tokens[current_token + 1].type == IDENTIFIER) {
            if (current_token + 2 < num_tokens && tokens[current_token + 2].type == LEFT_BRACKET) {
                ParseTreeNode *array_declaration = parse_array_declaration();
                node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
                node->children[node->num_children++] = array_declaration;
            } else {
                ParseTreeNode *variable_declaration = parse_variable_declaration();
                node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
                node->children[node->num_children++] = variable_declaration;
            }
        } else {
            fprintf(stderr, "Error: Expected identifier after data type in for loop initialization at line %d\n", tokens[current_token].line_number);
            exit(1);
        }
    } else {
        ParseTreeNode *const_statement = parse_const_statement();
        node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
        node->children[node->num_children++] = const_statement;
    }

    ParseTreeNode *condition = parse_const();
    node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
    node->children[node->num_children++] = condition;
    match(SEMICOLON);

    ParseTreeNode *increment = parse_const();
    node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
    node->children[node->num_children++] = increment;
    match(RIGHT_PARENTHESIS);

    ParseTreeNode *block = parse_block();
    node->children = realloc(node->children, sizeof(ParseTreeNode *) * (node->num_children + 1));
    node->children[node->num_children++] = block;

    return node;
}

// Function to allocate and initialize a new ParseTreeNode
ParseTreeNode *create_node(const char *name) {
    ParseTreeNode *node = malloc(sizeof(ParseTreeNode));
    node->name = strdup(name);
    node->token = NULL;
    node->children = NULL;
    node->num_children = 0;
    return node;
}

// Implement create functions for each non-terminal
ParseTreeNode *create_program_node() {
    return create_node("Program");
}

ParseTreeNode *create_declaration_node() {
    return create_node("Declaration");
}

ParseTreeNode *create_function_declaration_node() {
    return create_node("Function_Declaration");
}

ParseTreeNode *create_variable_declaration_node() {
    return create_node("Variable_Declaration");
}

ParseTreeNode *create_array_declaration_node() {
    return create_node("Array_Declaration");
}

ParseTreeNode *create_data_type_node() {
    return create_node("Data_Type");
}

ParseTreeNode *create_identifier_node() {
    return create_node("Identifier");
}

ParseTreeNode *create_parameter_list_node() {
    return create_node("Parameter_List");
}

ParseTreeNode *create_argument_list_node() {
    return create_node("Argument_List");
}

ParseTreeNode *create_block_node() {
    return create_node("Block");
}

ParseTreeNode *create_block_item_list_node() {
    return create_node("Block_Item_List");
}

ParseTreeNode *create_block_item_node() {
    return create_node("Block_Item");
}

ParseTreeNode *create_statement_node() {
    return create_node("Statement");
}

ParseTreeNode *create_return_statement_node() {
    return create_node("Return_Statement");
}

ParseTreeNode *create_const_statement_node() {
    return create_node("Const_Statement");
}

ParseTreeNode *create_while_statement_node() {
    return create_node("While_Statement");
}

ParseTreeNode *create_for_statement_node() {
    return create_node("For_Statement");
}

ParseTreeNode *create_exp_node() {
    return create_node("Exp");
}

ParseTreeNode *create_const_node() {
    return create_node("Constant");
}

// Function to print the parse tree with proper indentation to a file
void print_parse_tree(ParseTreeNode *node, int indent_level) {
    if (node == NULL) {
        return;
    }

    print_indent(indent_level);
    fprintf(output_file, "%s", node->name);

    if (node->token != NULL) {
        fprintf(output_file, "(%s", token_names[node->token->type]);
        if (node->token->type == IDENTIFIER || node->token->type == INTEGER_LITERAL ||
            node->token->type == FLOAT_LITERAL || node->token->type == CHARACTER_LITERAL ||
            node->token->type == STRING) {
            fprintf(output_file, ": \"%s\"", node->token->lexeme);
        }
        fprintf(output_file, ")");
    }

    if (node->num_children > 0) {
        fprintf(output_file, "(\n");
        for (int i = 0; i < node->num_children; i++) {
            print_parse_tree(node->children[i], indent_level + 1);
            if (i < node->num_children - 1) {
                fprintf(output_file, ",\n");
            } else {
                fprintf(output_file, "\n");
            }
        }
        print_indent(indent_level);
        fprintf(output_file, ")");
    }
}

// Helper function to print indentation to a file
void print_indent(int indent_level) {
    for (int i = 0; i < indent_level; i++) {
        fprintf(output_file, "  ");
    }
}