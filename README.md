lispc
=====

A simple Lisp interpreter written in C. It implements the most basic Lisp special forms (called operators here) as well as a small number of primitive procedures. It is lexically scoped and supports closures. 

## Building

lispc should build out of the box on any system. Using gcc, simply do

> `gcc *.c -o lisp -std=c99`

Running the resulting executable will start the lispc interpreter. It does not parse files at the moment, and the REPL is very simple.

## Language

lispc is a minimalistic Lisp language. Expressions in lispc are either symbols, procedures, numbers or lists. Symbols are used as names in variable bindings, or can be used as identifiers for other purposes. A symbol evaluates to the value it is bound to. Numbers and procedures evaluate to themselves. List evaluation follows two basic rules: If the first element is a symbol and the symbol denotes one of the *operators*, evaluation rules for the particular operator is followed. If the first element does not denote one of the operators, the list expression is a function call. Every element in the list is evaluated, and the first element, if resulting in a procedure, is called with the rest of the elements as arguments.

Some examples:

Numbers evaluate to themselves:

    LISP> 1234
    => 1234

`def` is an operator, so the following list expression will be an operator call:

    LISP> (def k 2)
    => K

The symbol `k` will after that call refer to the value 2:

    LISP> k
    => 2

A list where the first element is not an operator is a function call:

    LISP> (+ 1 (* 1 2) 3)
    => 6

### Operators

#### `\` (lambda)

`(\ (a1 a2 ...) exp)` constructs a function whose formal parameters are `a1`, `a2` and so on, and whose expression is `exp`. 

Example:

    (\ (x y z) (+ x y z))           ; a function which takes three arguments and sums them
    ((\ (x y z) (+ x y z)) 1 2 3)   ; create the described procedure and call it with arguments 1, 2 and 3

#### `def` 

`(def a b)` introduces a new *variable binding* where the symbol `a` is bound tothe value `b`.

Example:

    (def a 5)                       ; define a to be 5
    (def square (\ (x) (* x x)))    ; set square to be the function which squares its argument

#### `if` 

`(if test true-exp false-exp)` evaluates `test`. If the result is non-`NIL`, `true-exp` is evaluated. Otherwise, `false-exp` is evaluated. 

Example:

    (if (= x 2) (+ x 1) 2)          ; evaluates (+ x 1) if x is equal to 2, or evaluates 2 if not

#### `'` (quote)

`(' exp)` simply evaluates to `exp`. The parser will convert expressions of the form `'x` to `(' x)`.

Example:

    LISP> (' k)
    => K
    LISP> 'k
    => K

#### `set!`

`(set! a b)` changes the value what `a` refers to, to the value `b`. 

#### `let`

`(let ((a1 b1) (a2 b2) ...) exp)` introduces *local bindings* of the symbol `a1` to the value `b1`, the symbol `a2` to the value `b2`, and so on. With these bindings, the `exp` expression is evaluated. The bindings only exist during the evaluation of `exp`. Already existant bindings outside the `let` scope will be shadowed by these bindings.

Example:

    (let ((x 2) (y 3))
      (+ x y))                ; x and y are only bound in this expression

#### `do` or `:` 

`(do exp1 exp2 ...)` evaluates the expressions `exp1`, `exp2`, ... in order, and uses the last evaluated value as its value.

Example:

    (do (print 'hi)
      (print 'there) 
      1234)

This will print "HI THERE" on the screen, and the expression will evaluate to 1234.

### Primitives

### An example

lispc does not support tail call optimization. A sometimes useful technique to do efficient recursion on trees, in absence of this, is to keep a dictionary of already calculated values. This examples shows how a function for calculating the nth Fibonacci number can be implemented in lispc:

```
(:
  (def empty? (\ (x) (= x ())))
  (def list? (\ (x) (= (type x) 'list)))
  (def second (\ (x) (first (rest x))))

  (def find-by (\ (f element lst)
    (if (empty? lst)
      nil
      (if (= element (f (first lst)))
        (first lst)
        (find-by f element (rest lst))))))

  (def fib (\ (n)
    (: (def memo '((0 1) (1 1)))
       (def memo-add! (\ (index value)
         (: (set! memo (push (list index value) memo))
            value)))

       (def fibc (\ (n)
         (let ((memo-val (find-by first n memo)))
           (if memo-val
             (second memo-val) 
             (memo-add! n 
                        (if (< n 2) 
                          n
                          (+ (fibc (- n 1)) (fibc (- n 2)))))))))
       (fibc n)))))
````

### Generic functions

The standard library defines the functions `new-generic` and `implement` for using generic functions.

`(new-generic name)` creates a new generic function called `name` and returns a procedure which can call it.

`(implement name fn type)` defines a specific implementation of the generic function. `name` is the symbol of the generic function, `fn` is the implementing procedure, and `type` is a list or a symbol denoting the argument types the implementation handles. If `type` is a single symbol, the implementation is assumed to take a variable number of arguments, and every argument has to match this type. If `type` is a list, its items denote the argument types in order. 

#### Example

The function + might be useful to overload for certain types. The standard library implements + for integers as follows:

    (def + (new-generic '+))
    (implement '+ _+ 'integer)

Here, `_+` denotes the built in primitive procedure for adding integers. The symbolic type argument makes this implementation accept a variable number of arguments, all of integer type. 

To add a new implementation, for example for a type `matrix`, we can do

    (implement '+ add-matrix '(matrix matrix))
    
Here, the type argument is a list of two elements, meaning that this implementation will take two arguments, both of type `matrix`. The function `add-matrix` must be a function taking at least two such arguments.
