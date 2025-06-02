

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

var merge {
    var merge _

    merge := (list1, list2):{
        tern(none?(list1), list2, {
            list1 := some(list1)
            Pair?(left(list1), merge(right(list1), list2))
        })
    }

    merge
}

var List {
    var List _

    var List-1+ (x, xs...):{
        Pair?(x, List(xs...))
    }

    List := (xs...):{
        !tern($#varargs, END, {
            List-1+(xs...)
        })
    }

    List
}

var foreach {
    var foreach _

    foreach := (list, do):{
        none?(list) || {
            do(left(some(list)))
            foreach(right(some(list)))
        }
    }

    (foreach)
}

var mylist List(1, 2, 3)
foreach(mylist, (cur):{print(cur)})

'===range================================

var while {
    var while _

    while := (cond, do):{
        cond() && {
            do()
            while(cond, do)
        }
    }

    (while)
}

var - {
    var * _
    * := (lhs, rhs):{
        !tern(rhs, 0, {
            !tern(rhs + -1, lhs, {
                lhs + *(lhs, rhs + -1)
            })
        })
    }

    var - (n):{
        n + -2 * n
    }

    -
}

var Range (from, to):{
    var res List()
    while(():{from + -(to)}, ():{
        res := merge(res, from)
        from += 1
    })
}

foreach(Range(1, 5), (_):{
    print("hello")
})

{
    var sum 0
    foreach(Range(1, 5), (cur):{
        sum += cur
    })

    print('sum, sum)
}

