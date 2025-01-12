# Interpreter

Scanner, Parser, Compiler for a subset of the C language written in C

## Progress tracker of parser

**Grammar rule the parser.c can parse**

Will serve to show our progress on parser.c, and will eventually match our grammar rule. This shows the current grammar rule that the parser can read and output a working parse tree.

```ebnf
<program> ::= { <declaration> }
<declaration> ::= <variable_declaration> | <array_declaration> | <function_declaration>
<variable_declaration> ::= <data_type> <identifier> [ "=" <const> ] ";"
                        | <data_type> <identifier> { "," <identifier> } ";"
<array_declaration> ::= <data_type> <identifier> "[" [ <const> ] "]" [ "=" "{" [ <argument_list> ] "}" ] ";"
<function_declaration> ::= <data_type> <identifier> "(" <parameter_list> ")" ( <block> | ";" )
<parameter_list> ::= <data_type> <identifier> { "," <data_type> <identifier> }
<argument_list> ::= <const> { "," <const> }
<data_type> ::= "int" | "float" | "char" | "bool"
<block> ::= "{" <block_item_list> "}"
<block_item_list> ::= { <block_item> }
<block_item> ::= <statement> | <variable_declaration> | <array_declaration>
<statement> ::= return <const> ";"
            ::= <const> ";"
            ::= ";"
            ::= <block>
<const> ::= <int> | <float> | <char> | <bool>
<identifier> ::= IDENTIFIER
<int> ::= constant token
<float> ::= float token
<char> ::= char token
<bool> ::= bool token
```

**Test `test_main_program.core`**

The following test code

```c
bool array[5] = {1, 2};
int array[10];

int a, b, c;
int d;

int isValid(bool x, int y, char z);
int isNotValid();

int main() {
    bool array[5] = {};
    int array[1];
    ;
    int e;
    ;
    int f;
    int g;
}

bool isValid(bool x, int y, char z) {
    int a, b;
}
```

produces the following parse tree using my gawa-gawang notation ala *Writing a C Compiler*. The output of the parser is also stored at `parse_tree_output.ebnf`

```ebnf
Program(
  Declaration(
    Array_Declaration(
      Data_Type(BOOL),
      IDENTIFIER("array"),
      LEFT_BRACKET,
      Constant(5),
      RIGHT_BRACKET,
      ASSIGN,
      LEFT_BRACE,
      Argument_List(
        Constant(1),
        Constant(2)
      ),
      RIGHT_BRACE,
      SEMICOLON
    )
  ),
  Declaration(
    Array_Declaration(
      Data_Type(INT),
      IDENTIFIER("array"),
      LEFT_BRACKET,
      Constant(10),
      RIGHT_BRACKET,
      SEMICOLON
    )
  ),
  Declaration(
    Variable_Declaration(
      Data_Type(INT),
      IDENTIFIER("a"),
      COMMA,
      IDENTIFIER("b"),
      COMMA,
      IDENTIFIER("c"),
      SEMICOLON
    )
  ),
  Declaration(
    Variable_Declaration(
      Data_Type(INT),
      IDENTIFIER("d"),
      SEMICOLON
    )
  ),
  Declaration(
    Function_Declaration(
      Data_Type(INT),
      IDENTIFIER("isValid"),
      LEFT_PARENTHESIS,
      Parameter_List(
        Data_Type(BOOL),
        IDENTIFIER("x"),
        Data_Type(INT),
        IDENTIFIER("y"),
        Data_Type(CHAR),
        IDENTIFIER("z")
      ),
      RIGHT_PARENTHESIS,
      SEMICOLON
    )
  ),
  Declaration(
    Function_Declaration(
      Data_Type(INT),
      IDENTIFIER("isNotValid"),
      LEFT_PARENTHESIS,
      Parameter_List(

      ),
      RIGHT_PARENTHESIS,
      SEMICOLON
    )
  ),
  Declaration(
    Function_Declaration(
      Data_Type(INT),
      IDENTIFIER("main"),
      LEFT_PARENTHESIS,
      Parameter_List(

      ),
      RIGHT_PARENTHESIS,
      Block(
        LEFT_BRACE,
        Block_Item_List(
          Block_Item(
            Array_Declaration(
              Data_Type(BOOL),
              IDENTIFIER("array"),
              LEFT_BRACKET,
              Constant(5),
              RIGHT_BRACKET,
              ASSIGN,
              LEFT_BRACE,
              Argument_List(

              ),
              RIGHT_BRACE,
              SEMICOLON
            )
          ),
          Block_Item(
            Array_Declaration(
              Data_Type(INT),
              IDENTIFIER("array"),
              LEFT_BRACKET,
              Constant(1),
              RIGHT_BRACKET,
              SEMICOLON
            )
          ),
          Block_Item(
            Statement(
              SEMICOLON
            )
          ),
          Block_Item(
            Variable_Declaration(
              Data_Type(INT),
              IDENTIFIER("e"),
              SEMICOLON
            )
          ),
          Block_Item(
            Statement(
              SEMICOLON
            )
          ),
          Block_Item(
            Variable_Declaration(
              Data_Type(INT),
              IDENTIFIER("f"),
              SEMICOLON
            )
          ),
          Block_Item(
            Variable_Declaration(
              Data_Type(INT),
              IDENTIFIER("g"),
              SEMICOLON
            )
          )
        ),
        RIGHT_BRACE
      )
    )
  ),
  Declaration(
    Function_Declaration(
      Data_Type(BOOL),
      IDENTIFIER("isValid"),
      LEFT_PARENTHESIS,
      Parameter_List(
        Data_Type(BOOL),
        IDENTIFIER("x"),
        Data_Type(INT),
        IDENTIFIER("y"),
        Data_Type(CHAR),
        IDENTIFIER("z")
      ),
      RIGHT_PARENTHESIS,
      Block(
        LEFT_BRACE,
        Block_Item_List(
          Block_Item(
            Variable_Declaration(
              Data_Type(INT),
              IDENTIFIER("a"),
              COMMA,
              IDENTIFIER("b"),
              SEMICOLON
            )
          )
        ),
        RIGHT_BRACE
      )
    )
  )
)
```

## Scanner

The first two parts in the Scanner is the reading of the file, and the error handling. And only after is the reading of lexemes/tokens.

**Only write valid tokens into Symbol Table**

In parsing later on, we have to take a token one at a time, and it would be messy if we still have to clear
and check for error/invalid tokens and also comments. The symbol table now only contains real tokens for our language.

Our scanner will now only print error tokens and invalid characters at the console, and no longer write them
into the symbol table.

The same for comments. We'll also try not writing `COMMENT` tokens into the symbol table since it's of no use in parsing. But we still scan it and print into the console.

## Parser

The tokens were not loaded into memory in our lexical analysis, only stored in the symbol table. 
It looks like we have to read from the symbol table one token at a time (a stream of tokens) for the parsing.

But it feels icky to read one-at-a-time directly from the symbol table file. Will try to load it as a list/array of tokens for now for cleaner access, and then call the next token from it.

**Syntax Analyzer Rubric**

1. Input must be read one by one from the symbol table (one token at a time)
2. A parser algorithm was implemented
    *   Algorithm: Recursive Descent Parsing
3. Transition Diagram
4. Parsing table
5. Input Statement /1
6. Output Statement /3
7. Assignment Statement /3
8. Condition Statement /3
9. Iterative Statement /2
10. Declaration Statement
11. Error Recovery Method used:
12. Error Messages

**Testing the parser**

For now just call `scanner.c` on the `test_parse.core` files and then call `parser.c`.

```
.\scanner {filename}.core; .\parser;
```

**Recursive Descent parser and Pratt parsing**

We will use *recursive descent* in parsing the program: a subprogram to parse each non-terminal symbol in the formal grammar. 

Discussions and references seem to recommend utilizing Pratt parsing or Precedence climbing to parse expressions to tackle the difficulty of precedence in recrusive descent.

- *Writing a C Compiler*
- [Simple but powerful Pratt parsing](https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html)
- [What are the advantages of pratt parsing over recursive descent parsing?](https://www.reddit.com/r/ProgrammingLanguages/comments/zfnb1s/what_are_the_advantages_of_pratt_parsing_over/)
- [Pratt Parsers: Expression Parsing Made Easy](https://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/)

### Parse tree output

The parser must output a parse tree. For now our goal is a pretty-printer outputting the parse tree (not necessarilly a working data structure). I think this is done by each parse function for each grammar rule outputting the rule and its contents.

I am not sure what notation we will use for the parse tree, I don't think it really matters.

**Example code**

```
int main() {
    ;
}
```

**Example parse tree output** 

Made up the notation similar from *Writing a C Compiler*)

```ebnf
Program(
  Declaration(
    Function_Declaration(
      Data_type(INT),
      IDENTIFIER("main"),
      LEFT_PARENTHESIS,
      Parameter_List(),
      Block(
        LEFT_BRACE,
        Block_Item_List(
          Block_Item(
            Statement(
              SEMICOLON
            )
          )
        )
        RIGHT_BRACE
      )
      RIGHT_PARENTHESIS,
    )
  )
)
```

**Pretty-printing general rules**
It's a bit hard handling all the ways to close of non-terminals with a comma and newline. It seems like the formatting should be handled by the non-terminal function that called the next non-terminal instead of it being within the latter's code. 

For example, <declaration> does not have any sibling nodes when within <block-item>, while within <progam> it can happen. So I'll try formatting be a responsibility of the calling function.

### Notes on grammar errors while trying to implement the parser

**Can not do function declaration within functions**

We can have `<function_declaration>` within `<block>`, and since the body of a function is itself a block our grammar rules allows for this. Either specificy the <block> to only allow for <variable_declaration> and <array_declaration>.

**Array declaration without assignment, without array size**

```
int arr[]; // is this allowed?
```

## References

- [Crafting Interpreters](https://craftinginterpreters.com/) creates two implementations of their language `lox`:
a Java interpreter `jlox` and a C interpreter/compiler to bytecode `clox`
- [Sebesta - Concepts of Programming Languages](https://www.pearson.com/en-us/subject-catalog/p/concepts-of-programming-languages/P200000003361) assigned book