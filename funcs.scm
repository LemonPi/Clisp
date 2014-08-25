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

(define cube (nth-power 3))

(define square_and_cube (compose square cube))

(define (fib x)
        (cond ((= x 1) 1)
              ((= x 2) 1)
              (else (+ (fib (- x 1)) 
                       (fib (- x 2))))))
