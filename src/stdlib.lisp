(do
    (def empty? (\ (x) (= x ())))

    (def count
	 (\ (list)
	  (if (empty? list)
	      0
	    (+ 1 (count (rest list))))))

    (def map 
	 (\ (f list)
	  (if (empty? list)
	      ()
	    (push (f (first list))
		  (map f (rest list))))))

    (def filter
	 (\ (pred list)
	  (if (empty? list)
	      ()
	    (let ((f (first list)))
	      (if (pred f)
		  (push f (filter pred (rest list)))
		(filter pred (rest list)))))))

    (def reduce
	 (\ (f list)
	  (if (= (count list) 1)
	      (first list)
	    (f (first list)
	       (reduce f (rest list)))))))

