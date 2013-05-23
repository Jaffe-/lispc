lispc
=====

A simple Lisp interpreter written in C. It implements the most basic Lisp special forms (called operators here) as well as a small number of primitive procedures (arithmetic, cons/car/cdr and so on). It evaluates expressions according to the environment model described in SICP, so closures are supported. Tail recursion is however not optimized.

## Building

lispc should build out of the box on any system. Using gcc, simply do

> `gcc *.c -o lisp -std=c99`

Running the resulting executable will start the lispc interpreter. It does not parse files at the moment, and the REPL is very simple.

## Language

lispc is a minimalistic Lisp language.
