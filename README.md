# Beelang

Or just Bee, is another programming language created to learn about interpreters. This is a functional, statically and inferred typed
programming language, the idea it's to incrementally add new features and make a fully-featured toy.

## Features

* Read scripts from stdin.
* Function and variable binding/definition.
* Common arithmetic and logic operations.
  - Add (`+`), sub (`-`), mul (`*`), div (`/`).
  - And (`&, &&`), or (`|, ||`), not (`!`), xor (`^`).
  - Equal (`==`), not equal (`!=`), less than (`<`, `<=`) and greater than (`>`, `>=`).
* If-else conditions.
* List collections (`[expr, expr, expr, ...]`).

## Work in progress

* Modules.
* Lists (with borrowing model).
* Dictionaries (also with borrowing model).
* Map-Filter-Reduce.
* Lazy iterators.
* Comments lol.
* Bultin functions.

## Desired features

* ADTs
* JIT
* FFI
* Concurrency?
* Some kind of "side effect" controller?

As you might think, this project is still on heavy development and it's far from being stable, there are leaks on the overall memory
model and there is no optimizations at all.

## Syntax and program structure

My main objective during the design was to build a simplified version of a more complex language, we can fit the entire syntax
in the following block:

```
def greet(who) = "hello " + who + "!"

def double(x) = let fact = 2 in (x * fact if x else 0)

def main() = double(2)
```

A summary of the feature set:
  * Only two literals are supported: `NUMBER` and `STRING`.
  * `def` defines a new function into the current scope
  * `let` forks the scope assigning new bindings (i.e. `a = 1, b = 2, c = 3`), then executes an expression within that scope.
  * `if-else` it's just like your normal if-else.
  * Variable and expression types are lazly interpreted.

## Memory model

I didn't want to build a garbage collector so I packed everything into "scopes", in the future I would like to add some kind of
borrow-checker to handle references, for now all values are copied when forking a scope and released when leaving the scope.

```
(here is the global scope)

def f(a) = let b in a + b
               ^ b is defined in let scope, released after let ends
      ^ a is defined in f scope, released after f ends
    ^ f is defined into global scope, released after the program ends
```

## Building the project

`bison` and `lex` are required to build the `bee` target, if you happen to be in a decent Unix box with the proper setup to
c development then just `make` it. Run programs with `./bin/bee < program.bee`

## Project structure

I really hate having a directory to each part of the program, this project it's fairy simple so its structure just serves my needs.

* ast.h/ast.c - Structure for the AST nodes, also some "make" to make my life easier on the YACC file.
* builtins.h/builtins.c - Here goes the wrappers for the native procedures.
* examples - Candies
* lexer.l/parser.y - BISON/LEX stuff, if you want understand totally the syntax, begin with this files.
- Makefile - Magic
* misc - More candies
* run.h/run.c - Interpreter stuff.

## Contribution

Fork and PR. Issues will be managed here.
