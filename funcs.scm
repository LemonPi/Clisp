;; comments use semicolons, use double semicolon ;; for start of line comments
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

(define (fib x)                 ; inefficient recursive fib
        (cond ((= x 1) 1)
              ((= x 2) 1)
              (else (+ (fib (- x 1)) 
                       (fib (- x 2))))))

(define (add x)
        (cond ((empty? x) 0)
              (else (+ (car x) (add (cdr x))))))
