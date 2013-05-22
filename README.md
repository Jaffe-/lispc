lispc
=====

A simple Lisp interpreter written in C a couple of years ago. It implements the most basic Lisp special forms (called operators here) as well as a small number of primitive procedures (arithmetic, cons/car/cdr and so on). It evaluates expressions according to the environment model described in SICP, so closures are supported. Tail recursion is however not optimized.
