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

'===main==============================

var Stream Pair?

var stream-filter {
    var stream-filter _

    stream-filter := (pred, stream):{
        tern(none?(stream), END, {
            var stream some(stream)
            !tern(pred(left(stream)), stream-filter(pred, right(stream)), {
                Stream(left(stream), stream-filter(pred, right(stream)))
            })
        })
    }

    stream-filter
}

var subscript {
    var err ():{
        print("ERR not enough elements")
        exit(1)
    }

    var subscript _
    subscript := (stream, nth):{
        tern(none?(stream), err(), {
            var stream some(stream)
            !tern(nth + -1, left(stream), {
                subscript(right(stream), nth + -1)
            })
        })
    }

    subscript
}

var ints_starting_from_n {
    var ints_starting_from_n _

    ints_starting_from_n := (n):{
        Stream(n, ints_starting_from_n(n + 1))
    }

    ints_starting_from_n
}

var ints ints_starting_from_n(1)
var not3 stream-filter((x):{x + -3}, ints)
subscript(not3, 2)
subscript(not3, 3)
