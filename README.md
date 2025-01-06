# Interpreter

Scanner, Parser, Compiler for a subset of the C language written in C

- [ ] For the parser follow [Writing a C Compiler](https://nostarch.com/writing-c-compiler), it slowly builds each C feature by chapter and its parsing (together with the grammar, BNF, AST, etc.)
- [ ] Checkout the accompanying parser tests in [Test cases for Writing a C Compiler](https://github.com/nlsandler/writing-a-c-compiler-tests/tree/main)

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

**Recursive Descent parser and Pratt parsing**

We will use *recursive descent* in parsing the program: a subprogram to parse each non-terminal symbol in the formal grammar. 

Discussions and references seem to recommend utilizing Pratt parsing or Precedence climbing to parse expressions to tackle the difficulty of precedence in recrusive descent.

- *Writing a C Compiler*
- [Simple but powerful Pratt parsing](https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html)
- [What are the advantages of pratt parsing over recursive descent parsing?](https://www.reddit.com/r/ProgrammingLanguages/comments/zfnb1s/what_are_the_advantages_of_pratt_parsing_over/)
- [Pratt Parsers: Expression Parsing Made Easy](https://journal.stuffwithstuff.com/2011/03/19/pratt-parsers-expression-parsing-made-easy/)

## References

- [Crafting Interpreters](https://craftinginterpreters.com/) creates two implementations of their language `lox`:
a Java interpreter `jlox` and a C interpreter/compiler to bytecode `clox`
- [Sebesta - Concepts of Programming Languages](https://www.pearson.com/en-us/subject-catalog/p/concepts-of-programming-languages/P200000003361) assigned book