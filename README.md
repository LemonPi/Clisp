### Clisp
---------------------

A lightweight (~550 lines of code) Lisp interpreter with Scheme dialect.
I was inspired to make this after watching all the [SICP](http://ocw.mit.edu/courses/electrical-engineering-and-computer-science/6-001-structure-and-interpretation-of-computer-programs-spring-2005/video-lectures/) lectures.

Features:
 - file inclusion e.g. (include test.scm), can be nested inside files 
 - first class procedures and by extension higher order procedures
 - lexical scoping so you don't have to worry about local variables clashing in called procedures
 - free typed arguments, e.g. (define (add x) ...) x is expected by the semantics to be a list, but not enforced
 - ;; and ; comments, use double for start of line comments


<a href="http://www.boost.org/users/download/"><img alt="Get boost" src="http://www.boost.org/style-v2/css_0/get-boost.png"></a> <br>
I originally intended to include the boost parts required, but as you can see from include\_list, 
you're better off getting the proper boost distribution. 
To compensate, this repository contains
a binary built on Ubuntu 14.04 which should work on most Unix based machines.


Example:

```
;; comments use semicolons, use double semicolon ;; for start of line comments

(include funcs.scm)     ; don't do this! recursive inclusion is bad D:

(define compose (lambda (f g)   ; fundamental higher order procedure
        (lambda (x)
                (f (g x)))))

(define expt (lambda (x n)      ; exponential 
              (cond ((= n 1) x)
                    (else (* x 
                             (expt x (- n 1)))))))

(define nth-power (lambda (n)
        (lambda (x)
                (expt x n))))

(define square (nth-power 2))

(define cube (nth-power 3))

(define square_and_cube (compose square cube))

(define (add x)
        (cond ((empty? x) 0)
              (else (+ (car x) (add (cdr x))))))


(cat 'something 'somethingelse)

(square 5)

(add (1 2 3 4))
```

Tips:
 - build by typing "make" in the same directory
 - list parameters (such as x for add above) treated same as 'normal' parameters
 - interpret files with `./clisp [filename] [-p]`, add -p or -print option to force printing of file evaluation, silent by default (assumes a lot of definitions)
    - include files with (include filename), which can be nested
 - build debug information (with step by step info) by changing build target and source in makefile to "testing" and "testing.cpp" respectively
 - requires a compiler supporting C++11
 - uses boost::variant (link above)
 - keywords (so far): define, lambda, cond, cons, cdr, list, else, and, or, not, empty?, include
 - use 'quote to signify string
     - `string` will raise an error if it's not defined, but `'string` will return string
 - use cat primitive instead of + to concatenate strings
 - expressions can extend over different lines, terminated by appropriate )!
