(:
  (def empty? (\ (x) (= x ())))
  (def symbol? (\ (x) (= (type x) 'symbol)))
  (def second (\ (x) (_first (_rest x))))
  (def list? (\ (x) (= (type x) 'list)))

    (def count
	 (\ (list)
	  (if (empty? list)
	      0
	    (_+ 1 (count (_rest list))))))

    (def map 
	 (\ (f list)
	  (if (empty? list)
	      ()
	    (_push (f (_first list))
		  (map f (_rest list))))))

    (def filter
	 (\ (pred list)
	  (if (empty? list)
	      ()
	    (let ((f (_first list)))
	      (if (pred f)
		  (_push f (filter pred (_rest list)))
		(filter pred (_rest list)))))))

    (def reduce
	 (\ (f list)
	  (if (= (count list) 1)
	      (_first list)
	    (f (_first list)
	       (reduce f (_rest list))))))

  (def every? (\ (pred lst)
    (if (empty? lst)
      'true
      (if (pred (_first lst))
        (every? pred (_rest lst))
        nil))))

  (def join (\ (lsta lstb)
    (if (empty? lsta)
      lstb
      (join (_rest lsta) (_push (_first lsta) lstb)))))

  (def find-by (\ (f element lst)
    (if (empty? lst)
      nil
      (if (= element (f (_first lst)))
        (_first lst)
        (find-by f element (_rest lst))))))

    (def _generic-table '())
    (def _generic-name _first)
    (def _generic-fn _first)
    (def _generic-imps second)
    (def _generic-types second)

    (def _table-update-dispatch (\ (op fn types)
      (list (_generic-name op) (_push (list fn types) (_generic-imps op)))))

    (def _table-add! (\ (operation)
      (if (find-by _first operation _generic-table)
        nil
        (set! _generic-table
              (_push (list operation '()) _generic-table)))))

    (def _table-add-dispatch! (\ (name fn types)
      (if (find-by _generic-name name _generic-table)
        (: (set! _generic-table
                (map (\ (generic)
                       (if (= (_generic-name generic) name)
                         (_table-update-dispatch generic fn types)
                         generic))
                     _generic-table))
           name)
         'no-generic)))

    (def _apply-generic (\ (name .. args)
      (let ((generic (find-by _generic-name name _generic-table)))
        (if generic
          (let ((imps (_generic-imps generic))
                (arg-types (map type args)))
            (let ((imp (find-by _generic-types arg-types imps)))
              (if imp
                (apply (_generic-fn imp) args)
                (let ((imp (filter (\ (x) (symbol? (_generic-types x))) imps)))
                  (if (empty? imp)
                    'error-no-match
                    (if (every? (\ (x) (= x (_generic-types (_first imp)))) arg-types)
                      (apply (_generic-fn (_first imp)) args)
                      'error-not-all-same-type))))))
          'error-no-oper))))

    (def new-generic (\ (name)
      (: (_table-add! name)
         (\ (.. x) (apply _apply-generic (_push name x))))))

    (def implement (\ (name fn types)
      (_table-add-dispatch! name fn types)))

    (def + (new-generic '+))
    (def - (new-generic '-))
    (def * (new-generic '*))
    (def < (new-generic '<))
    (def > (new-generic '>))
    (def <= (new-generic '<=))
    (def >= (new-generic '>=))
    (def % (new-generic '%))
    (def first (new-generic 'first))
    (def rest (new-generic 'rest))
    (def print (new-generic 'print))
    (implement '+ _+ 'integer)
    (implement '- _- 'integer)
    (implement '* _* 'integer)
    (implement '< _< '(integer integer))
    (implement '> _> '(integer integer))
    (implement '<= _<= '(integer integer))
    (implement '>= _>= '(integer integer))
    (implement '% _% '(integer integer))
    (implement 'first _first '(list))
    (implement 'rest _rest '(list))
    (implement 'print _print '(list))
    (implement 'print _print '(integer))
    (implement 'print _print '(procedure))
    (implement 'print _print '(symbol)))
