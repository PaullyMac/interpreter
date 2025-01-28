#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "token.h"
#include "parser.h"

// Global variables to store the token list and the current token index
Token *tokens;
int current_token = 0;
int num_tokens;
bool panic_mode = false;

// Global file pointer for the output file
FILE *output_file;


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

    printf("\nPARSING!\n\n");
    ParseTreeNode *root = parse_program();

    if (panic_mode) {
        printf("Parsing failed!\n");
        fclose(output_file);
        remove("parse_tree_output.ebnf");
    } else {
        printf("Parsing successful!\n");
        print_parse_tree(root, 0);
        fclose(output_file);
    }
    free_parse_tree(root);
    free(tokens);
    return 0;
}


Token *load_tokens(const char *filename, int *num_tokens) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Error opening symbol table file: %s\n", filename);
        synchronize();
    }

    Token *token_list = NULL;
    size_t capacity = 16;
    token_list = malloc(sizeof(Token) * capacity);

    if (!token_list) {
        fprintf(stderr, "Error: Memory allocation failed in load_tokens!\n");
        fclose(fp);
        synchronize();
    }

    *num_tokens = 0;

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (*num_tokens >= capacity) {
            capacity *= 2;
            Token *new_list = realloc(token_list, sizeof(Token) * capacity);
            if (!new_list) {
                fprintf(stderr, "Error: Memory reallocation failed in load_tokens!\n");
                free(token_list);
                fclose(fp);
                synchronize();
            }
            token_list = new_list;
        }

        int token_code;
        char token_name[50];
        char lexeme_val[MAX_LEXEME_LENGTH];
        int line_num, col_num;

        if (sscanf(line, "%d | %[^|] | %d | %d | %[^\n]", &token_code, token_name, &line_num, &col_num, lexeme_val) == 5) {
                    token_list[*num_tokens].type = token_code;

                    // Trim trailing spaces from token_name
                    int len = strlen(token_name);
                    while (len > 0 && isspace(token_name[len - 1])) {
                        token_name[len - 1] = '\0';
                        len--;
                    }

                    // Trim trailing spaces from lexeme_val
                    len = strlen(lexeme_val);
                    while (len > 0 && isspace(lexeme_val[len - 1])) {
                        lexeme_val[len - 1] = '\0';
                        len--;
                    }
                    
                    strncpy(token_list[*num_tokens].lexeme, lexeme_val, MAX_LEXEME_LENGTH - 1);
                    token_list[*num_tokens].lexeme[MAX_LEXEME_LENGTH - 1] = '\0';
                    
                    token_list[*num_tokens].line_number = line_num;
                    token_list[*num_tokens].column_number = col_num;

                    (*num_tokens)++;
        }
    }
    fclose(fp);
    return token_list;
}

// Helper function to add a child to a parse tree node
void add_child(ParseTreeNode *parent, ParseTreeNode *child) {
    parent->num_children++;
    ParseTreeNode **new_children = realloc(parent->children, sizeof(ParseTreeNode *) * parent->num_children);
    if (!new_children) {
        fprintf(stderr, "Error: Memory allocation failed in add_child\n");
        synchronize(); 
    }
    parent->children = new_children;
    parent->children[parent->num_children - 1] = child;
}

// Helper function to match the current token with the expected type and create a node for it
ParseTreeNode *match_and_create_node(TokenType type, const char* node_name) {
    printf("Parsing token: %-20s %-20s Line: %d, Column: %d\n", token_names[tokens[current_token].type], tokens[current_token].lexeme, tokens[current_token].line_number, tokens[current_token].column_number);
    ParseTreeNode *node = create_node(node_name);
    node->token = malloc(sizeof(Token));
    if (!node->token) {
        fprintf(stderr, "Error: Memory allocation failed in match_and_create_node\n");
        synchronize();
    }
    *node->token = tokens[current_token];

    if (current_token < num_tokens && tokens[current_token].type == type) {
        current_token++;
    } else {
        report_error("Unexpected token", type);
        synchronize();
    }
    return node;
}

void match(TokenType type) {
    if (current_token < num_tokens && tokens[current_token].type == type) {
        current_token++;
    } else {
        fprintf(stderr, "Error: Expected token type %s but found %s at line %d\n",
            token_names[type], token_names[tokens[current_token].type],
            tokens[current_token].line_number);
        synchronize();
    }
}

// Function to print the parse tree with proper indentation to a file
void print_parse_tree(ParseTreeNode *node, int indent_level) {
    if (node == NULL) {
        return;
    }

    print_indent(indent_level);

    // Check if the node is a terminal node (has a token)
    if (node->token != NULL) {
        // If it's a literal, print the token type and lexeme
        if (node->token->type == integer_literal ||
            node->token->type == float_literal ||
            node->token->type == character_literal ||
            node->token->type == identifier ||
            node->token->type == string) {
                // Only have a single set of quotations for String literals
                if (node->token->type == string) {
                    fprintf(output_file, "%s: %s", token_names[node->token->type], node->token->lexeme);
                } else {
                    fprintf(output_file, "%s: \"%s\"", token_names[node->token->type], node->token->lexeme);
                }
        }
        // Otherwise, just print the token type
        else {
            fprintf(output_file, "%s", token_names[node->token->type]);
        }
    }
    // If it's not a terminal node, print the node name and recurse
    else {
        fprintf(output_file, "%s(", node->name);

        // Recursively print the children
        if (node->num_children > 0) {
            fprintf(output_file, "\n");
            for (int i = 0; i < node->num_children; i++) {
                print_parse_tree(node->children[i], indent_level + 1);
                if (i < node->num_children - 1) {
                    fprintf(output_file, ",\n");
                }
            }
            fprintf(output_file, "\n");
            print_indent(indent_level);
        }
        fprintf(output_file, ")");
    }
}


// Helper function to print indentation to a file
void print_indent(int indent_level) {
    for (int i = 0; i < indent_level; i++) {
        fprintf(output_file, "  ");
    }
}

void free_parse_tree(ParseTreeNode *node) {
    if (!node) return;
    
    for (int i = 0; i < node->num_children; i++) {
        free_parse_tree(node->children[i]);
    }
    
    free(node->token);
    free(node->name);
    free(node->children);
    free(node);
}

void report_error(const char *message, TokenType expected) {
    fprintf(stderr, "Error: %s, Expected: %s, Line: %d, Column: %d\n",
            message,
            token_names[expected],
            tokens[current_token].line_number,
            tokens[current_token].column_number);
    panic_mode = true;
}

void synchronize() {
    panic_mode = true;
    while (current_token < num_tokens) {
        // Synchronize on statement/declaration boundaries
        if (tokens[current_token].type == semicolon ||
            tokens[current_token].type == right_brace ||
            tokens[current_token].type == int_kw ||
            tokens[current_token].type == float_kw ||
            tokens[current_token].type == char_kw ||
            tokens[current_token].type == bool_kw ||
            tokens[current_token].type == for_kw ||
            tokens[current_token].type == while_kw ||
            tokens[current_token].type == if_kw) {
            return;
        }
        current_token++;
    }
}

// <program> ::= { <declaration> }
ParseTreeNode *parse_program() {
    ParseTreeNode *node = create_program_node();
    
    while (current_token < num_tokens && tokens[current_token].type != token_eof) {
        ParseTreeNode *declaration = parse_declaration();
        if (declaration == NULL) {
            // If not a valid declaration, synchronize and continue
            fprintf(stderr, "Error: Invalid declaration at Line: %d\n", 
                    tokens[current_token].line_number);
            synchronize();
            continue;
        }
        add_child(node, declaration);
    }
    return node;
}

// <declaration> ::= <variable_declaration> | <array_declaration> | <function_declaration>
ParseTreeNode *parse_declaration() {
    ParseTreeNode *node = create_declaration_node();

    // Return NULL if not a valid declaration start
    if (current_token >= num_tokens || 
        (tokens[current_token].type != int_kw && 
        tokens[current_token].type != float_kw &&
        tokens[current_token].type != char_kw && 
        tokens[current_token].type != bool_kw)) {
        return NULL;
    }

    // Find valid category of declaration
    if (current_token + 1 < num_tokens && 
        tokens[current_token+1].type == identifier) {
        
        if (current_token + 2 < num_tokens && 
            tokens[current_token+2].type == left_parenthesis) {
            ParseTreeNode *function_declaration = parse_function_declaration();
            add_child(node, function_declaration);
        } 
        else if(current_token + 2 < num_tokens && 
                tokens[current_token+2].type == left_bracket) {
            ParseTreeNode *array_declaration = parse_array_declaration();
            add_child(node, array_declaration);
        } 
        else {
            ParseTreeNode *variable_declaration = parse_variable_declaration();
            add_child(node, variable_declaration);
        }
        return node;
    }

    return NULL; // Not a valid declaration
}

// <variable_declaration> ::= <data_type> <identifier> [ “=” <exp> ] “;”
//                         | <data_type> <identifier> { “,” <identifier> } “;”
ParseTreeNode *parse_variable_declaration() {
    ParseTreeNode *node = create_variable_declaration_node();
    ParseTreeNode *data_type = parse_data_type();
    add_child(node, data_type);

    // Parse first identifier
    ParseTreeNode *identifier_node = parse_identifier();
    add_child(node, identifier_node);

    // Handle assignment if present
    if (current_token < num_tokens && tokens[current_token].type == assign) {
        add_child(node, match_and_create_node(assign, "Assign"));
        
        // Parse the assignment expression
        ParseTreeNode *exp_node = parse_exp();
        add_child(node, exp_node);
    }
    
    // Handle multiple declarations
    while (current_token < num_tokens && tokens[current_token].type == comma) {
        add_child(node, match_and_create_node(comma, "Comma"));
        
        // Parse next identifier
        identifier_node = parse_identifier();
        add_child(node, identifier_node);
        
        // Handle assignment for this identifier if present
        if (current_token < num_tokens && tokens[current_token].type == assign) {
            add_child(node, match_and_create_node(assign, "Assign"));
            
            // Parse the assignment expression
            ParseTreeNode *exp_node = parse_exp();
            add_child(node, exp_node);
        }
    }

    // Expect semicolon at end
    if (current_token < num_tokens && tokens[current_token].type == semicolon) {
        add_child(node, match_and_create_node(semicolon, "Semicolon"));
    } else {
        fprintf(stderr, "Error: Expected semicolon at end of variable declaration at line %d\n", 
                tokens[current_token].line_number);
        synchronize();
    }

    return node;
}

// <array_declaration> ::= <data_type> <identifier> “[“ [<const>]  “]”  [ “=” “{“ <argument_list>“}” ] “;”
ParseTreeNode *parse_array_declaration() {
    ParseTreeNode *node = create_array_declaration_node();
    ParseTreeNode *data_type = parse_data_type();
    add_child(node, data_type);

    ParseTreeNode *identifier_node = parse_identifier();
    add_child(node, identifier_node);

    add_child(node, match_and_create_node(left_bracket, "Left_Bracket"));

    if (current_token < num_tokens && (tokens[current_token].type == integer_literal ||
                                    tokens[current_token].type == float_literal ||
                                    tokens[current_token].type == character_literal ||
                                    tokens[current_token].type == true_kw ||
                                    tokens[current_token].type == false_kw)) {
        ParseTreeNode *const_node = parse_const();
        add_child(node, const_node);
    }

    add_child(node, match_and_create_node(right_bracket, "Right_Bracket"));

    if (current_token < num_tokens && tokens[current_token].type == assign) {
        add_child(node, match_and_create_node(assign, "Assign"));
        add_child(node, match_and_create_node(left_brace, "Left_Brace"));

        // Parse argument list
        if (current_token < num_tokens && tokens[current_token].type != right_brace) {
            ParseTreeNode *argument_list = parse_argument_list();
            add_child(node, argument_list);
        }

        add_child(node, match_and_create_node(right_brace, "Right_Brace"));
    }

    add_child(node, match_and_create_node(semicolon, "Semicolon"));
    return node;
}

// <function_declaration> ::= <data_type> <identifier> "(" <parameter_list> ")" ( <block> | “;” )
ParseTreeNode *parse_function_declaration() {
    ParseTreeNode *node = create_function_declaration_node();
    ParseTreeNode *data_type = parse_data_type();
    add_child(node, data_type);

    ParseTreeNode *identifier_node = parse_identifier();
    add_child(node, identifier_node);

    add_child(node, match_and_create_node(left_parenthesis, "Left_Parenthesis"));

    ParseTreeNode *parameter_list = parse_parameter_list();
    add_child(node, parameter_list);

    add_child(node, match_and_create_node(right_parenthesis, "Right_Parenthesis"));

    if (current_token < num_tokens && tokens[current_token].type == left_brace) {
        ParseTreeNode *block = parse_block();
        add_child(node, block);
    } else if (current_token < num_tokens && tokens[current_token].type == semicolon) {
        add_child(node, match_and_create_node(semicolon, "Semicolon"));
    } else {
        report_error("Invalid function declaration, Expected: \"{\" or \";\", Current: %s", tokens[current_token].type);
    }
    return node;
}

// <parameter_list> ::= [“void”]
//   | <data_type> <identifier> {"," <data_type> <identifier>}
ParseTreeNode *parse_parameter_list() {
    ParseTreeNode *node = create_parameter_list_node();
    if (current_token < num_tokens && tokens[current_token].type == right_parenthesis) {
        // Empty parameter list
    } else if (current_token < num_tokens && tokens[current_token].type == void_kw) {
            add_child(node, match_and_create_node(void_kw, "VOID"));
    } else {
        if (current_token < num_tokens && (tokens[current_token].type == int_kw || tokens[current_token].type == float_kw ||
                                        tokens[current_token].type == char_kw || tokens[current_token].type == bool_kw)) {
            ParseTreeNode *data_type = parse_data_type();
            add_child(node, data_type);

            ParseTreeNode *identifier_node = parse_identifier();
            add_child(node, identifier_node);

            while (current_token < num_tokens && tokens[current_token].type == comma) {
                add_child(node, match_and_create_node(comma, "Comma"));

                if (current_token < num_tokens && (tokens[current_token].type == int_kw || tokens[current_token].type == float_kw ||
                                                tokens[current_token].type == char_kw || tokens[current_token].type == bool_kw)) {
                    ParseTreeNode *data_type = parse_data_type();
                    add_child(node, data_type);

                    ParseTreeNode *identifier_node = parse_identifier();
                    add_child(node, identifier_node);

                } else {
                    fprintf(stderr, "Error: Expected data type after comma in parameter list at line %d\n", tokens[current_token].line_number);
                    synchronize();
                }
            }
        } else {
            fprintf(stderr, "Error: Expected data type or ')' at the start of parameter list at line %d\n", tokens[current_token].line_number);
            synchronize();
        }
    }
    return node;
}

// <data_type> ::= “int” | “float” | “char” | “bool”
ParseTreeNode *parse_data_type() {
    ParseTreeNode *node = create_data_type_node();
    if (current_token < num_tokens) {
        if (tokens[current_token].type == int_kw) {
            add_child(node, match_and_create_node(int_kw, "INTT"));
        } else if (tokens[current_token].type == float_kw) {
            add_child(node, match_and_create_node(float_kw, "FLOATT"));
        } else if (tokens[current_token].type == char_kw) {
            add_child(node, match_and_create_node(char_kw, "CHARR"));
        } else if (tokens[current_token].type == bool_kw) {
            add_child(node, match_and_create_node(bool_kw, "BOOL"));
        } else {
            fprintf(stderr, "Error: Expected data type at line %d\n", tokens[current_token].line_number);
            synchronize();
        }
    }
    return node;
}

// <identifier> ::= identifier token
ParseTreeNode *parse_identifier() {
    ParseTreeNode *node = create_identifier_node();
    if (tokens[current_token].type == identifier) {
        add_child(node, match_and_create_node(identifier, "IDENTIFIERR"));
    } else {
        fprintf(stderr, "Error: Expected data type at line %d\n", tokens[current_token].line_number);
        synchronize();
    }

    return node;
}

// <block> ::= "{" <block-item-list>  "}"
ParseTreeNode *parse_block() {
    ParseTreeNode *node = create_block_node();
    add_child(node, match_and_create_node(left_brace, "Left_Brace"));

    while (current_token < num_tokens && tokens[current_token].type != right_brace) {
        ParseTreeNode *block_item = parse_block_item();
        if (block_item != NULL) {
            add_child(node, block_item);
        } else {
            synchronize();
            if (current_token >= num_tokens || 
                tokens[current_token].type == right_brace) {
                break;
            }
        }
    }

    if (current_token < num_tokens && tokens[current_token].type == right_brace) {
        add_child(node, match_and_create_node(right_brace, "Right_Brace"));
    } else {
        fprintf(stderr, "Error: Missing closing brace at line %d\n", 
                tokens[current_token-1].line_number);
        synchronize();
    }

    return node;
}

// <block_item_list> ::= (<block_item_list> <block_item>) | <block_item>
ParseTreeNode *parse_block_item_list() {
    ParseTreeNode *node = create_block_item_list_node();
    while (current_token < num_tokens) {
        if (tokens[current_token].type == right_brace) {
            break; // Exit the loop if we encounter a RIGHT_BRACE
        }
        ParseTreeNode *block_item = parse_block_item();
        if (block_item != NULL) {
            add_child(node, block_item);
        } else {
            return node;
        }
    }
    return node;
}

// <block_item> ::= <statement> | <variable_declaration> | <array_declaration>
ParseTreeNode *parse_block_item() {
    ParseTreeNode *node = create_block_item_node();
    
    // Check for variable/array declarations first
    if (current_token < num_tokens && 
        (tokens[current_token].type == int_kw || 
        tokens[current_token].type == float_kw ||
        tokens[current_token].type == char_kw || 
        tokens[current_token].type == bool_kw)) {
        
        // Look ahead to distinguish between array and variable declaration
        if (current_token + 2 < num_tokens && 
            tokens[current_token + 2].type == left_bracket) {
            add_child(node, parse_array_declaration());
        } else {
            add_child(node, parse_variable_declaration());
        }
    } else {
        // If not a declaration, must be a statement
        add_child(node, parse_statement());
    }
    
    return node;
}

// <statement> ::= "return" <const> ;" | <const> ";" | ";" 
ParseTreeNode *parse_statement() {
    ParseTreeNode *node = create_statement_node();
    
    switch (tokens[current_token].type) {
        case return_kw:
            add_child(node, parse_return_statement());
            break;
        case identifier:
            // Handle function calls and assignments
            ParseTreeNode *exp = parse_exp();
            add_child(node, exp);
            add_child(node, match_and_create_node(semicolon, "Semicolon")); 
            break;
        case if_kw:
            add_child(node, parse_if_statement());
            break;
        case while_kw:
            add_child(node, parse_while_statement());
            break;
        case for_kw:
            add_child(node, parse_for_statement());
            break;
        case scanf_kw:
            add_child(node, parse_input_statement());
            break;
        case printf_kw:
            add_child(node, parse_output_statement());
            break;
        case semicolon:
            add_child(node, match_and_create_node(semicolon, "Semicolon"));
            break;
        case left_brace:
            add_child(node, parse_block());
            break;
        default:
            add_child(node, parse_expression_statement());
            break;
    }

    return node;
}

ParseTreeNode *parse_argument_list() {
    ParseTreeNode *node = create_argument_list_node();
    // The first argument should be parsed as a full expression
    ParseTreeNode *exp = create_exp_node(); // Create an Exp node
    add_child(exp, parse_exp());
    add_child(node, exp); // Add the Exp node to the argument list

    while (current_token < num_tokens && tokens[current_token].type == comma) {
        add_child(node, match_and_create_node(comma, "Comma"));
        // Subsequent arguments are also full expressions
        ParseTreeNode *exp = create_exp_node(); // Create an Exp node
        add_child(exp, parse_exp());
        add_child(node, exp); // Add the Exp node to the argument list
    }
    return node;
}

// <exp> ::= <logical_or_exp> | (<identifier> | <identifier> "[" <const> "]") "=" <exp>
ParseTreeNode *parse_exp() {
    if (current_token < num_tokens && tokens[current_token].type == identifier) {
        int lookahead = current_token + 1;

        // Check if array lvalue
        if (current_token + 1 < num_tokens && tokens[current_token + 1].type == left_bracket) {
            lookahead = current_token + 2;

            while (lookahead < num_tokens && tokens[lookahead].type != right_bracket) {
                lookahead++;
            }
            
            if (lookahead < num_tokens && tokens[lookahead].type == right_bracket) {
                lookahead++;
            }
            else {
                fprintf(stderr, "Error: Missing closing bracket in array access at line %d\n", tokens[current_token].line_number);
                synchronize();
            }
        }

        if (lookahead < num_tokens && tokens[lookahead].type == assign) {
            ParseTreeNode *node = create_exp_node();

            add_child(node, parse_identifier());

            // Handle array access if present
            if (current_token < num_tokens && tokens[current_token].type == left_bracket) {
                add_child(node, match_and_create_node(left_bracket, "Left_Bracket"));
                add_child(node, parse_const());
                add_child(node, match_and_create_node(right_bracket, "Right_Bracket"));
            }

            add_child(node, match_and_create_node(assign, "Assign"));

            // Recursively build sequence of assignment operators
            add_child(node, parse_exp());

            return node;
        }
    }

    return parse_logical_or_exp();
}

// <logical_or_exp> ::= <logical_and_exp> | <logical_or_exp> "||" <logical_and_exp>
ParseTreeNode *parse_logical_or_exp() {
    ParseTreeNode *left = parse_logical_and_exp();

    // Only go through the parsing of OR's if it's not a lone factor 
    if (current_token < num_tokens && tokens[current_token].type == or) {
        ParseTreeNode *node = create_logical_or_exp_node();

        // Add the factor (left operand) to the logical or node
        add_child(node, left);

        // Build the logical OR nodes bottom-up
        while (current_token < num_tokens && tokens[current_token].type == or) {
            add_child(node, match_and_create_node(or, "Operator"));

            // Add the factor (right operand) to the logical or node
            ParseTreeNode *right = parse_logical_and_exp();
            add_child(node, right);

            // Assure left-associativity, create a new node above the previous if there are more ORs
            if (current_token < num_tokens && tokens[current_token].type == or) {
              ParseTreeNode *new_node = create_logical_or_exp_node();

              // The previous OR node we built is added as a child to a new OR node
            add_child(new_node, node);
            node = new_node;
            }
        }
        return node;
    }

    // Return the factor directly if there's no OR
    return left; 
}

// <logical_and_exp> ::= <power_exp> | <logical_and_exp> "&&" <factor>
ParseTreeNode *parse_logical_and_exp() {
    ParseTreeNode *left = parse_power_exp();

    if (current_token < num_tokens && tokens[current_token].type == and) {
        ParseTreeNode *node = create_logical_and_exp_node();
        add_child(node, left);

        while (current_token < num_tokens && tokens[current_token].type == and) {
            add_child(node, match_and_create_node(and, "Operator"));
            ParseTreeNode *right = parse_power_exp();
            add_child(node, right);

            if (current_token < num_tokens && tokens[current_token].type == and) {
              ParseTreeNode *new_node = create_logical_and_exp_node();
            add_child(new_node, node);
            node = new_node;
            }
        }
        return node;
    }

    return left; 
}

// TODO
ParseTreeNode *parse_equality_exp() {
    ParseTreeNode *node = parse_relational_exp();

    while (current_token < num_tokens &&
        (tokens[current_token].type == equal || tokens[current_token].type == not_equal)) {
        TokenType op_type = tokens[current_token].type;
        match(op_type);
        ParseTreeNode *new_node = create_node("Equality");
        add_child(new_node, node);
        add_child(new_node, match_and_create_node(op_type, "Operator"));
        add_child(new_node, parse_relational_exp());
        node = new_node;
    }
    return node;
}

// TODO
ParseTreeNode *parse_relational_exp() {
    ParseTreeNode *node = parse_additive_exp();

    while (current_token < num_tokens &&
        (tokens[current_token].type == less || tokens[current_token].type == greater ||
        tokens[current_token].type == less_equal || tokens[current_token].type == greater_equal)) {
        TokenType op_type = tokens[current_token].type;
        match(op_type);
        ParseTreeNode *new_node = create_node("Relational");
        add_child(new_node, node);
        add_child(new_node, match_and_create_node(op_type, "Operator"));
        add_child(new_node, parse_additive_exp());
        node = new_node;
    }
    return node;
}

// TODO
ParseTreeNode *parse_additive_exp() {
    ParseTreeNode *node = parse_multiplicative_exp();
    while (current_token < num_tokens &&
        (tokens[current_token].type == plus || tokens[current_token].type == minus)) {
        TokenType op_type = tokens[current_token].type;
        match(op_type);
        ParseTreeNode *new_node = create_node("AddSub");
        add_child(new_node, node);
        add_child(new_node, match_and_create_node(op_type, "Operator"));
        add_child(new_node, parse_multiplicative_exp());
        node = new_node;
    }
    return node;
}

// TODO
ParseTreeNode *parse_multiplicative_exp() {
    ParseTreeNode *node = parse_power_exp();

    while (current_token < num_tokens &&
        (tokens[current_token].type == multiply || tokens[current_token].type == divide || tokens[current_token].type == modulo)) {
        TokenType op_type = tokens[current_token].type;
        match(op_type);
        ParseTreeNode *new_node = create_node("MulDivMod");
        add_child(new_node, node);
        add_child(new_node, match_and_create_node(op_type, "Operator"));
        add_child(new_node, parse_power_exp());
        node = new_node;
    }
    return node;
}

// <power_exp> ::= <factor> | <factor> ”^” <power_exp>
ParseTreeNode *parse_power_exp() {
    ParseTreeNode *node = create_power_exp_node();

    // Start with the left operand (<unary_exp>)
    ParseTreeNode *left = parse_factor(); 

    // Check if there's a "^" operator.
    if (current_token < num_tokens && tokens[current_token].type == exponent) {
        add_child(node, left);
        add_child(node, match_and_create_node(exponent, "Exponent_Op"));
        ParseTreeNode *right = parse_power_exp(); // Recurse on right side to get the expression
        add_child(node, right);

        return node;
    }

    // Pass to the next exp if power_exp unused
    return left;
}

//  TODO
ParseTreeNode *parse_unary_exp() {
    // <unary_exp> ::= <factor> | <unop> <unary_exp>
    if (tokens[current_token].type == plus ||
        tokens[current_token].type == minus ||
        tokens[current_token].type == not_equal) {
        ParseTreeNode *node = create_node("UnaryOp");
        TokenType op = tokens[current_token].type;
        match(op);
        add_child(node, match_and_create_node(op, "Unary_Operator"));
        add_child(node, parse_unary_exp());
        return node;
    }
    return parse_factor();
}

ParseTreeNode *parse_factor() {
    ParseTreeNode *node = create_node("Factor");

    if (current_token < num_tokens) {
        switch (tokens[current_token].type) {
            case integer_literal:
            case float_literal:
            case character_literal:
            case true_kw:
            case false_kw:
                add_child(node, parse_const());
                break;
            case identifier:
                add_child(node, parse_identifier());
                if (current_token < num_tokens && tokens[current_token].type == left_parenthesis) {
                    add_child(node, match_and_create_node(left_parenthesis, "Left_Parenthesis"));
                    if (current_token < num_tokens && tokens[current_token].type != right_parenthesis) {
                        add_child(node, parse_argument_list());
                    }
                    add_child(node, match_and_create_node(right_parenthesis, "Right_Parenthesis"));
                } else if (current_token < num_tokens && tokens[current_token].type == left_bracket) {
                    add_child(node, match_and_create_node(left_bracket, "Left_Bracket"));
                    add_child(node, parse_const());
                    add_child(node, match_and_create_node(right_bracket, "Right_Bracket"));
                }
                break;
            case left_parenthesis:
                add_child(node, match_and_create_node(left_parenthesis, "Left_Parenthesis"));

                ParseTreeNode *exp_node = parse_exp();
                add_child(node, exp_node);
                add_child(node, match_and_create_node(right_parenthesis, "Right_Parenthesis"));
                break;
            default:
                fprintf(stderr, "Error: Unexpected token in factor at line %d\n", tokens[current_token].line_number);
                synchronize();
                break;
        }
    }

    return node;
}

// Function to parse a constant: <const> ::= <int> | <float> | <char> | <bool>
ParseTreeNode *parse_const() {
    ParseTreeNode *node = create_const_node();
    
    if (current_token < num_tokens) {
        switch (tokens[current_token].type)
        {
            case integer_literal:
                ParseTreeNode *int_node = parse_int_literal();
                add_child(node, int_node);
                break;
            case float_literal:
                ParseTreeNode *float_node = parse_float_literal();
                add_child(node, float_node);
                break;
            case character_literal:
                ParseTreeNode *char_node = parse_char_literal();
                add_child(node, char_node);
                break;
            case true_kw:
            case false_kw:
                ParseTreeNode *bool_node = parse_bool_literal();
                add_child(node, bool_node);
                break;
            
            default:
                fprintf(stderr, "Error: Expected a constant (int, float, char, or bool) at line %d\n", tokens[current_token].line_number);
                synchronize();
        }
    }

    return node; 
}


// Function to parse a return statement: "return" <const> ";"
ParseTreeNode *parse_return_statement() {
    ParseTreeNode *node = create_return_statement_node();
    add_child(node, match_and_create_node(return_kw, "RETURNN"));

    add_child(node, parse_factor());

    add_child(node, match_and_create_node(semicolon, "SEMICOLONN"));
    return node;
}

ParseTreeNode *parse_expression_statement() {
    ParseTreeNode *node = create_expression_statement_node();
    // Create an Exp node to wrap the expression
    ParseTreeNode *exp_node = create_exp_node();
    add_child(exp_node, parse_exp());
    add_child(node, exp_node);

    // Match semicolon at the end of the expression statement
    if (current_token < num_tokens && tokens[current_token].type == semicolon) {
        add_child(node, match_and_create_node(semicolon, "Semicolon"));
    } else {
        fprintf(stderr, "Error: Expected semicolon at end of expression statement at line %d\n", tokens[current_token].line_number);
        synchronize();
    }

    return node;
}

ParseTreeNode *parse_factor_statement()
{
    ParseTreeNode *node = create_factor_statement_node();
    ParseTreeNode *factor_node = parse_factor();
    add_child(node, factor_node);

    add_child(node, match_and_create_node(semicolon, "Semicolon"));
    return node;
}

// Function to parse a constant statement: <const> ";"
ParseTreeNode *parse_const_statement() {
    ParseTreeNode *node = create_const_statement_node();
    ParseTreeNode *const_node = parse_const();
    add_child(node, const_node);

    add_child(node, match_and_create_node(semicolon, "Semicolon"));
    return node;
}

// Function to parse a while statement: "while" "(" <const> ")" <block>
ParseTreeNode *parse_while_statement() {
    ParseTreeNode *node = create_while_statement_node();
    add_child(node, match_and_create_node(while_kw, "While"));
    add_child(node, match_and_create_node(left_parenthesis, "Left_Parenthesis"));

    ParseTreeNode *exp = parse_exp();
    add_child(node, exp);

    add_child(node, match_and_create_node(right_parenthesis, "Right_Parenthesis"));

    ParseTreeNode *block = parse_block();
    add_child(node, block);

    return node;
}

// Function to parse a for loop statement
ParseTreeNode *parse_for_statement() {
    ParseTreeNode *node = create_for_statement_node();
    
    // Match "for" and "("
    add_child(node, match_and_create_node(for_kw, "For"));
    add_child(node, match_and_create_node(left_parenthesis, "Left_Parenthesis"));

    // Parse initialization
    if (tokens[current_token].type == int_kw || 
        tokens[current_token].type == float_kw ||
        tokens[current_token].type == char_kw || 
        tokens[current_token].type == bool_kw) {
        
        // Handle declarations
        if (current_token + 2 < num_tokens && 
            tokens[current_token + 2].type == left_bracket) {
            add_child(node, parse_array_declaration());
        } else {
            add_child(node, parse_variable_declaration());
        }
    } else {
        // Handle expression case
        ParseTreeNode *init_exp = parse_exp();
        add_child(node, init_exp);
        add_child(node, match_and_create_node(semicolon, "Semicolon"));
    }

    // Parse condition
    ParseTreeNode *condition = parse_exp();
    add_child(node, condition);
    add_child(node, match_and_create_node(semicolon, "Semicolon"));

    // Parse increment
    ParseTreeNode *increment = parse_exp();
    add_child(node, increment);

    // Match closing ")" and parse block
    add_child(node, match_and_create_node(right_parenthesis, "Right_Parenthesis"));
    add_child(node, parse_block());

    return node;
}

ParseTreeNode *parse_input_statement() {
    ParseTreeNode *node = create_input_statement_node();
    add_child(node, match_and_create_node(scanf_kw, "Scanf"));
    add_child(node, match_and_create_node(left_parenthesis, "Left_Parenthesis"));

    add_child(node, match_and_create_node(string, "String"));

    while (current_token < num_tokens && tokens[current_token].type == comma) {
        add_child(node, match_and_create_node(comma, "Comma"));
        add_child(node, match_and_create_node(ampersand, "Ampersand"));
        ParseTreeNode *identifier = parse_identifier();
        add_child(node, identifier);
    }

    add_child(node, match_and_create_node(right_parenthesis, "Right_Parenthesis"));
    add_child(node, match_and_create_node(semicolon, "Semicolon"));
    return node;
}

ParseTreeNode *parse_output_statement() {
    ParseTreeNode *node = create_output_statement_node();
    
    // Match printf and left parenthesis
    add_child(node, match_and_create_node(printf_kw, "Printf"));
    add_child(node, match_and_create_node(left_parenthesis, "Left_Parenthesis"));

    // Handle printf arguments
    if (current_token < num_tokens) {
        if (tokens[current_token].type == string) {
            add_child(node, match_and_create_node(string, "String"));

            // Handle variable arguments after format string
            while (current_token < num_tokens && tokens[current_token].type == comma) {
                add_child(node, match_and_create_node(comma, "Comma"));
                ParseTreeNode *exp = parse_exp();
                add_child(node, exp);
            }
        } else if (tokens[current_token].type == identifier) {
            ParseTreeNode *identifier = parse_identifier();
            add_child(node, identifier);
        } else {
            fprintf(stderr, "Error: Expected string or identifier in printf at line %d\n", 
                    tokens[current_token].line_number);
            synchronize();
            return node;
        }
    }

    // Match closing parenthesis and semicolon
    add_child(node, match_and_create_node(right_parenthesis, "Right_Parenthesis")); 
    add_child(node, match_and_create_node(semicolon, "Semicolon"));

    return node;
}

ParseTreeNode *parse_if_statement() {
    ParseTreeNode *node = create_if_statement_node();
    add_child(node, match_and_create_node(if_kw, "If"));
    add_child(node, match_and_create_node(left_parenthesis, "Left_Parenthesis"));

    ParseTreeNode *condition = parse_exp();
    add_child(node, condition);

    add_child(node, match_and_create_node(right_parenthesis, "Right_Parenthesis"));

    ParseTreeNode *if_block = parse_block();
    add_child(node, if_block);

    while (current_token < num_tokens && tokens[current_token].type == else_kw) {
        ParseTreeNode *else_clause = parse_else_clause();
        add_child(node, else_clause);
    }
    return node;
}

// "else" <block> | "else" <if_statement>
ParseTreeNode *parse_else_clause() {
    ParseTreeNode *node = create_node("Else_Clause");
    add_child(node, match_and_create_node(else_kw, "Else"));

    if (current_token < num_tokens && tokens[current_token].type == if_kw) {
        ParseTreeNode *if_statement = parse_if_statement();
        add_child(node, if_statement);
    } else {
        ParseTreeNode *else_block = parse_block();
        add_child(node, else_block);
    }
    return node;
}

ParseTreeNode *parse_int_literal() {
    ParseTreeNode *node = create_int_literal_node();
    add_child(node, match_and_create_node(integer_literal, "INTEGER_LITERALL"));

    return node;
}
ParseTreeNode *parse_float_literal() {
    ParseTreeNode *node = create_float_literal_node();
    add_child(node, match_and_create_node(float_literal, "FLOAT_LITERALL"));
    
    return node;
}
ParseTreeNode *parse_char_literal() {
    ParseTreeNode *node = create_char_literal_node();
    add_child(node, match_and_create_node(character_literal, "CHARACTER_LITERALL"));
    
    return node;
}
ParseTreeNode *parse_bool_literal() {
    ParseTreeNode *node = create_bool_literal_node();

    if (tokens[current_token].type == true_kw) {
        add_child(node, match_and_create_node(true_kw, "TRUEE"));
    } else {
        add_child(node, match_and_create_node(false_kw, "FALSEE"));
    }
    
    return node;
}

// Function to allocate and initialize a new ParseTreeNode
ParseTreeNode *create_node(const char *name) {
    ParseTreeNode *node = malloc(sizeof(ParseTreeNode));
    if (!node) {
        fprintf(stderr, "Error: Memory allocation failed in create_node\n");
        synchronize();
    }
    node->name = strdup(name);
    if (!node->name) {
        fprintf(stderr, "Error: Memory allocation failed in create_node\n");
        synchronize();
    }
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

ParseTreeNode *create_expression_statement_node()
{
    return create_node("Expression_Statement");
}

ParseTreeNode *create_factor_statement_node()
{
    return create_node("Factor_Statement");
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

ParseTreeNode *create_if_statement_node()
{
    return create_node("If_Statement");
}

ParseTreeNode *create_input_statement_node()
{
    return create_node("Input_Statement");
}

ParseTreeNode *create_output_statement_node()
{
    return create_node("Output_Statement");
}

ParseTreeNode *create_exp_node() {
    return create_node("Exp");
}

ParseTreeNode *create_logical_or_exp_node() {
    return create_node("Logical_Or");
}

ParseTreeNode *create_logical_and_exp_node() {
    return create_node("Logical_And");
}

ParseTreeNode *create_power_exp_node(){
    return create_node("Exponent");
}

ParseTreeNode *create_unary_exp_node(){
    return create_node("Unary_Exp");
}

ParseTreeNode *create_factor_node()
{
    return create_node("Factor");
}

ParseTreeNode *create_const_node() {
    return create_node("Const");
}

ParseTreeNode *create_int_literal_node()
{
    return create_node("Int");
}
ParseTreeNode *create_float_literal_node()
{
    return create_node("Float");
}
ParseTreeNode *create_char_literal_node()
{
    return create_node("Char");
}
ParseTreeNode *create_bool_literal_node()
{
    return create_node("Bool");
}