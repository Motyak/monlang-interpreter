
var tern (cond, if_true, if_false):{
    var res _
    cond && {res := if_true}
    cond || {res := if_false}
    res
}

var !tern (cond, if_false, if_true):{
    tern(cond, if_true, if_false)
}

var not (bool):{
    tern(bool, $false, $true)
}

var * {
    var * _

    * := (lhs, rhs):{
        !tern(rhs, 0, {
            !tern(rhs + -1, lhs, {
                lhs + *(lhs, rhs + -1)
            })
        })
    }

    *
}

'===Pair==============================

var Pair (left, right):{
    (selector):{selector(left, right)}
}

var left (pair):{
    pair((left, right):{left})
}

var right (pair):{
    pair((left, right):{right})
}

'===Optional==========================

var Optional (some?, val):{
    var none? ():{
        not(some?)
    }

    var some ():{
        some? || {
            print("ERR calling some() on empty Optional")
            exit(1)
        }
        val
    }

    '----------------

    var dispatcher (msg_id):{
        !tern(msg_id, none?, {
            !tern(msg_id + -1, some, {
                print("ERR invalid msg_id in dispatcher: `" + msg_id + "`")
                exit(1)
            })
        })
    }

    dispatcher
}

var none? (opt):{
    opt(0)()
}

var some (opt):{
    opt(1)()
}

'===List==============================

_ := ```
    Lists are nested Optional Pairs.
    As opposed to a Pair, a List can be empty (0 element)..
      -> END
    it can also contain only 1 element..
      -> Pair?(x, END)
    or 2+ elements as such..
      -> Pair?(a, Pair?(b, END))
      -> Pair?(a, Pair?(b, Pair?(c, END)))
      -> ...
```

var Pair? (left, right):{
    Optional($true, Pair(left, right))
}
var END {
    Optional($false, _)
}

var List {
    var List-1+ _

    var List (xs...):{
        !tern($#varargs, END, {
            List-1+(xs...)
        })
    }

    List-1+ := (x, xs...):{
        Pair?(x, List(xs...))
    }

    List
}

var foreach {
    var foreach _

    foreach := (list, do):{
        tern(none?(list), {}, {
            do(left(some(list)))
            foreach(right(some(list)), do)
        })
    }

    (foreach)
}

'===main==============================

'---reduce--------------------

var reduce (op, init, list):{
    var acc init

    foreach(list, (cur):{
        ```
            we need to immediatly evaluate `acc` value
        ```
        var acc' acc
        acc := op(acc', cur)
    })

    acc
}

reduce(+, 0, List(1, 2, 3))
reduce(+, 'T, List('o, 'm, 'm, 'y))

'---pipe----------------------

var pipe (fns...):{
    var compose (fn1, fn2):{
        (x):{fn2(fn1(x))}
    }

    reduce(compose, (x):{x}, List(fns...))
}

var inc (x):{x + 1}
var double (x):{x * 2}
var pipeline pipe(inc, double)
pipeline(10)
