#ifndef SCANNER_H
#define SCANNER_H

#include "token.h"

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
TokenType keywords(char *lexeme);
int peek();
void unget_char(int ch);
void set_token_end_column();

#endif // SCANNER_H