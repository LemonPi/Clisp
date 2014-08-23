### Clispp
---------------------
Lisp interpreter with a subset dialect of Scheme

Example:

`
(define compose (lambda (f g)
        (lambda (x)
                (f (g x)))))

(define expt (lambda (x n)
              (cond ((= n 1) x)
                    (else (* x 
                             (expt x (- n 1)))))))

(define nth-power (lambda (n)
        (lambda (x)
                (expt x n))))

(define square (nth-power 2))

(square 5)
`

Tips:
 - build by typing "make" in the same directory
 - requires a compiler supporting C++11
 - uses boost::variant (comes with this build)
 - keywords (so far): define, lambda, cond, cons, cdr, list
 - use quotes to signify string
     - (string) will raise an error if it's not defined, but ('string) will return string
 - use cat primitive instead of + to concatenate strings
 - expressions can extend over different lines, terminated by appropriate )!
