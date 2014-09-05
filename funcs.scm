; useful scheme functions
; core functions
(define (if test a b)
        (cond ((test) a) (else b)))

(define compose (lambda (f g)   ; fundamental higher order procedure
        (lambda (x)
                (f (g x)))))

(define expt (lambda (x n)      ; exponential 
              (cond ((= n 1) x)
                    (else (* x 
                             (expt x (- n 1)))))))

; demonstrative functions
(define nth-power (lambda (n)
        (lambda (x)
                (expt x n))))

(define square (nth-power 2))

(define cube (nth-power 3))

(define square_and_cube (compose square cube))

(define (fib-naive x)                 ; inefficient recursive fib, stack usage grows per recursive call
        (cond ((= x 1) 1)
              ((= x 2) 1)
              (else (+ (fib (- x 1)) 
                       (fib (- x 2))))))

(define (fib-iter last current counter) ; iterative algorithms are much better, stack usage remains constant
        (cond ((< counter 3) current)   ; 2 or lower is just 1
              (else (fib-iter current (+ last current) (- counter 1)))))

(define (fib n) (fib-iter 1 1 n))

(define (add x)                 ; x is semantically a list
        (cond ((empty? x) 0)
              (else (+ (car x) (add (cdr x))))))

(define (sum inc accumulate current upper)
        (cond ((= current upper) (+ accumulate current))
              (else (sum inc
                         (+ accumulate current)
                         (inc current)
                         upper))))

(define (inc x) (+ x 1))
(define (linear-sum lower upper) (sum inc 0 lower upper))

(define (factorial n)
        (cond ((< n 2) 1) (else (* n (factorial (- n 1))))))

(define (add4 n)
        (let ((x 4)) (+ x n)))  ; demonstrates local variables
