lispc
=====

A simple Lisp interpreter written in C. It implements the most basic Lisp special forms (called operators here) as well as a small number of primitive procedures (arithmetic, cons/car/cdr and so on). It evaluates expressions according to the environment model described in SICP, so closures are supported. Tail recursion is however not optimized.

## Building

lispc should build out of the box on any system. Using gcc, simply do

> `gcc *.c -o lisp -std=c99`

Running the resulting executable will start the lispc interpreter. It does not parse files at the moment, and the REPL is very simple.

## Language

lispc is a minimalistic Lisp language. Expressions in lispc are either symbols, numbers or lists. Symbols are used as names in variable bindings. A symbol evaluates to the value it is bound to. Numbers evaluate to themselves. List evaluation follows two basic rules: If the first element is a symbol and the symbol denotes one of the *operators*, evaluation rules for that operator is followed. Otherwise, every element in the list is evaluated, and the first element -- which has to be a function -- is applied to the rest of the list. 

### Operators

#### `define` 

`(define a b)` introduces a new *variable binding* where the symbol `a` is bound to the value `b`. 

#### `\` or `lambda`

`(\ (a1 a2 ...) exp)` constructs a function whose formal parameters are `a1`, `a2` and so on, and whose expression is `exp`. 

#### `if` 

`(if test true-exp false-exp)` evaluates `test`. If the result is non-`NIL`, `true-exp` is evaluated. Otherwise, `false-exp` is evaluated. 

#### `quote`

`(quote exp)` simply evaluates to `exp`.

#### `set`

`(set a b)` changes the value of the variable `a` refers to, to `b`.

#### `let`

`(let ((a1 b1) (a2 b2) ...) exp)` introduces *local bindings* of the symbol `a1` to the symbol `b1`, the symbol `a2` to the symbol `b2`, and so on. With these bindings, the `exp` expression is evaluated. The bindings cease to exist outside of the `let` form.

#### `do`

`(do exp1 exp2 ...)` evaluates `exp1`, `exp2` and so on, and uses the last evaluated value as its value. 

### Primitives

