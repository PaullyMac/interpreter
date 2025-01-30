# Interpreter

Scanner, Parser, Compiler for a subset of the C language written in C

**Running the scanner and parser together**

For now just call `scanner.c` on the `test_main_program.core` files and then call `parser.c`.

```
.\scanner {filename}.core; .\parser;
```

The scanner writes the tokens into the `symbol_table.txt`, then the parser loads the tokens from it and writes the parse tree to `parse_tree_output.ebnf`.

## Progress tracker of parser

**Grammar rule the parser.c can parse**

Will serve to show our progress on parser.c, and will eventually match our grammar rule. This shows the current grammar rule that the parser can read and output a working parse tree.

```ebnf
<program> ::= { <declaration> }
<declaration> ::= <variable_declaration> 
                | <array_declaration> 
                | <function_declaration> 

<variable_declaration> ::= <data_type> <identifier> "=" <factor>  { "," <identifier> "=" <factor> } ";" 
                        | <data_type> <identifier> { "," <identifier> } ";" 
<array_declaration> ::= <data_type> <identifier> "[" [ <const> ] "]" [ "=" "{" [ <argument_list> ] "}" ] ";"
<function_declaration> ::= <data_type> <identifier> "(" <parameter_list> ")" ( <block> | ";" )

<parameter_list> ::= ["void"] |  <data_type> <identifier> { "," <data_type> <identifier> }
<argument_list> ::= <factor> { "," <factor> }

<data_type> ::= "int" | "float" | "char" | "bool"

<block> ::= "{" <block_item_list> "}"
<block_item_list> ::= { <block_item> }
<block_item> ::= <statement> | <variable_declaration> | <array_declaration>

<statement> ::= "return" <factor> ";"
              | <factor> ";"
              | ";"
              | <block>
              | <for_statement>
              | "while" "(" <factor> ")" <block>
              | "for" "(" (<variable_declaration> | <array_declaration> | <factor> ";") <factor> ";" <factor> ")" <block>
              | <if_statement>
              | <input_statement>
              | <outout_statement>

<input_statement> ::= "scanf" "(" <string> { "," "&" <identifier> } ")" ";"
<output_statement> ::= "printf" "(" <string> ")" ";"
                                    | "printf" "(" <string> "," <factor> ")" ";"
                                    | "printf" "(" <string> { "," <factor> } ")" ";"
                                    | "printf" "(" <identifier> ")" ";"
<if_statement> ::= "if" "(" <factor> ")" <block> [<else-clause>]
<else-clause> ::= "else" <block>
                | "else" <if_statement>

<factor> ::= <const>
            | <identifier>
            | "(" <factor> ")"
            | <identifier> "(" [ <argument_list> ] ")"
            | <identifier> "[" <const> "]" 
<const> ::= <int> | <float> | <char> | <bool>
<string> ::= STRING
<identifier> ::= identifier
<int> ::= integer_literal
<float> ::= FLOAT_LITERAL
<char> ::= CHARACTER_LITERAL
<bool> ::= "true" | "false"
```

**Test `test_main_program.core`**

The following test code

```c
bool array[5] = {1, 2};
int array[10];

// Test variables
int a, b, c;
int d = 5;
int c = bar(1000, 2, 3);
int b = a; 
int arr[3] = {(foo(5)), a[20], multiply(a)};

// Function prototypes
int isValid(bool x, int y, char z);
int empty(void);

int main(void) {
    // Test input statements
    scanf("%d");
    scanf("dog");
    scanf("%f", &value);
    scanf("%f %f", &value1, &value2);
    scanf(" ");

    // Test output statements
    printf("Hello, Universe!"); 
    printf(name)
    printf("Hello, your grade is %d %d", grade);
    printf("Hello, your grade is %d %d", grade, year);

    // Test if statements
    if (true) {
        return 0;
    }

    if (false) {
        bool a;
    } else {
        bool b;
    }
...
```

produces the following parse tree using my gawa-gawang notation ala *Writing a C Compiler*. The output of the parser is also stored at `parse_tree_output.ebnf`

```ebnf
Program(
  Declaration(
    Array_Declaration(
      Data_Type(
        bool
      ),
      Identifier(
        identifier: "array"
      ),
      l,
      Const(
        Int(
          integer_literal: "5"
        )
      ),
      RIGHT_BRACKET,
      ASSIGN,
      LEFT_BRACE,
      Argument_List(
        Factor(
          Const(
            Int(
              integer_literal: "1"
            )
          )
        ),
        COMMA,
        Factor(
          Const(
            Int(
              integer_literal: "2"
            )
          )
        )
      ),
      RIGHT_BRACE,
      SEMICOLON
    )
  ),
  Declaration(
    Array_Declaration(
      Data_Type(
        INT
      ),
      Identifier(
        identifier: "array"
      ),
      LEFT_BRACKET,
      Const(
        Int(
          integer_literal: "10"
        )
      ),
      RIGHT_BRACKET,
      SEMICOLON
    )
  ),
  Declaration(
    Variable_Declaration(
      Data_Type(
        INT
      ),
      Identifier(
        identifier: "a"
      ),
      COMMA,
      Identifier(
        identifier: "b"
      ),
      COMMA,
      Identifier(
        identifier: "c"
      ),
      SEMICOLON
    )
  ),
  Declaration(
    Variable_Declaration(
      Data_Type(
        INT
      ),
      Identifier(
        identifier: "d"
      ),
      ASSIGN,
      Factor(
        Const(
          Int(
            integer_literal: "5"
          )
        )
      ),
      SEMICOLON
    )
  ),
  Declaration(
    Variable_Declaration(
      Data_Type(
        INT
      ),
      Identifier(
        identifier: "c"
      ),
      ASSIGN,
      Factor(
        Identifier(
          identifier: "bar"
        ),
        LEFT_PARENTHESIS,
        Argument_List(
          Factor(
            Const(
              Int(
                integer_literal: "1000"
              )
            )
          ),
          COMMA,
          Factor(
            Const(
              Int(
                integer_literal: "2"
              )
            )
          ),
          COMMA,
          Factor(
            Const(
              Int(
                integer_literal: "3"
              )
            )
          )
        ),
        RIGHT_PARENTHESIS
      ),
      SEMICOLON
    )
  ),
  Declaration(
    Variable_Declaration(
      Data_Type(
        INT
      ),
      Identifier(
        identifier: "b"
      ),
      ASSIGN,
      Factor(
        Identifier(
          identifier: "a"
        )
      ),
      SEMICOLON
    )
  ),
  Declaration(
    Array_Declaration(
      Data_Type(
        INT
      ),
      Identifier(
        identifier: "arr"
      ),
      LEFT_BRACKET,
      Const(
        Int(
          integer_literal: "3"
        )
      ),
      RIGHT_BRACKET,
      ASSIGN,
      LEFT_BRACE,
      Argument_List(
        Factor(
          LEFT_PARENTHESIS,
          Factor(
            Identifier(
              identifier: "foo"
            ),
            LEFT_PARENTHESIS,
            Argument_List(
              Factor(
                Const(
                  Int(
                    integer_literal: "5"
                  )
                )
              )
            ),
            RIGHT_PARENTHESIS
          ),
          RIGHT_PARENTHESIS
        ),
        COMMA,
        Factor(
          Identifier(
            identifier: "a"
          ),
          LEFT_BRACKET,
          Const(
            Int(
              integer_literal: "20"
            )
          ),
          RIGHT_BRACKET
        ),
        COMMA,
        Factor(
          Identifier(
            identifier: "multiply"
          ),
...
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
      identifier("main"),
      LEFT_PARENTHESIS,
      Parameter_List(),
      Block(
        LEFT_BRACE,
        Block_Item_List(
          Block_Item(
            Statement(
              semicolon
            )
          )
        )
        right_brace
      )
      right_parenthesis,
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
## Future Work and Development Roadmap

The next stages of this project involve the following development milestones:

- **Full Grammar Implementation:** Complete implementation of all grammar rules according to our desired C subset.
- **Improved Error Handling:** Enhance the parser's error detection and recovery capabilities.
- **Semantic Analysis:** Implement a semantic analyzer to check for type compatibility, scope issues, and other semantic errors.
- **Intermediate Representation (IR):** Generate an intermediate code representation, such as three-address code, as a step before code generation.
- **Code Generation:** Develop a code generator to translate the intermediate representation into machine code or assembly.
- **Optimization:** Explore opportunities for optimization, such as constant folding or register allocation.

## References

- [Crafting Interpreters](https://craftinginterpreters.com/) creates two implementations of their language `lox`:
a Java interpreter `jlox` and a C interpreter/compiler to bytecode `clox`
- [Sebesta - Concepts of Programming Languages](https://www.pearson.com/en-us/subject-catalog/p/concepts-of-programming-languages/P200000003361) assigned book
