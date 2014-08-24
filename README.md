### Clisp
---------------------
Lisp interpreter with a subset dialect of Scheme.
I was inspired to make this after watching all the [SICP](http://ocw.mit.edu/courses/electrical-engineering-and-computer-science/6-001-structure-and-interpretation-of-computer-programs-spring-2005/video-lectures/) lectures.
Part of this exercise was to explore how code and data are really the same

Example:

```
(define compose (lambda (f g)
        (lambda (x)
                (f (g x)))))

(define expt (lambda (x n)
              (cond ((= n 1) x)
                    (else (* x 
                             (expt x (- n 1)))))))

(define (nth-power n)
        (lambda (x)
                (expt x n))))

(define square (nth-power 2))

(cat 'something 'somethingelse)

(square 5)
```

Tips:
 - build by typing "make" in the same directory
 - interpret files with `./clisp [filename] [-p]`, add -p or -print option to force printing of file evaluation, silent by default (assumes a lot of definitions)
 - build debug information (with step by step info) by changing build target and source in makefile to "testing" and "testing.cpp" respectively
 - requires a compiler supporting C++11
 - uses boost::variant (comes with this build)
 - keywords (so far): define, lambda, cond, cons, cdr, list, else, and, or, not
 - use 'quote to signify string
     - `string` will raise an error if it's not defined, but `'string` will return string
 - use cat primitive instead of + to concatenate strings
 - expressions can extend over different lines, terminated by appropriate )!
