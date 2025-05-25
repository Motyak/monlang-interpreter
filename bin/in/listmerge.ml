
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

var merge _
merge := (list1, list2):{
    tern(none?(list1), list2, {
        list1 := some(list1)
        Pair?(left(list1), merge(right(list1), list2))
    })
}

"list_a with no elem"
{
    var list_a END
    "list_a with no elem and list_b with no elem"
    {
        var list_b END
        var res merge(list_a, list_b)
        print('$true, none?(res))
    }
    "list_a with no elem and list_b with one elem"
    {
        var list_b Pair?(13, END)
        var res merge(list_a, list_b)
        print('$false, none?(res))
        print(13, left(some(res)))
        print('$true, none?(right(some(res))))
    }
    "list_a with no elem and list_b with two elems"
    {
        var list_b Pair?(13, Pair?(37, END))
        var res merge(list_a, list_b)
        print('$false, none?(res))
        print(13, left(some(res)))
        print('$false, none?(right(some(res))))
        print(37, left(some(right(some(res)))))
        print('$true, none?(right(some(right(some(res))))))
    }
}

"list_a with one elem"
{
    var list_a Pair?(13, END)
    "list_a with one elem and list_b with no elem"
    {
        var list_b END
        var res merge(list_a, list_b)
        print('$false, none?(res))
        print(13, left(some(res)))
        print('$true, none?(right(some(res))))
    }
    "list_a with one elem and list_b with one elem"
    {
        var list_b Pair?(37, END)
        var res merge(list_a, list_b)
        print('$false, none?(res))
        print(13, left(some(res)))
        print('$false, none?(right(some(res))))
        print(37, left(some(right(some(res)))))
        print('$true, none?(right(some(right(some(res))))))
    }
}

"list_a with two elems"
{
    var list_a Pair?(13, Pair?(37, END))
    "list_a with two elems and list_b with no elem"
    {
        var list_b END
        var res merge(list_a, list_b)
        print('$false, none?(res))
        print(13, left(some(res)))
        print('$false, none?(right(some(res))))
        print(37, left(some(right(some(res)))))
        print('$true, none?(right(some(right(some(res))))))
    }
    "list_a with two elems and list_b with two elems"
    {
        var list_b Pair?(73, Pair?(31, END))
        var res merge(list_a, list_b)
        print('$false, none?(res))
        print(13, left(some(res)))
        print('$false, none?(right(some(res))))
        print(37, left(some(right(some(res)))))
        print('$false, none?(right(some(right(some(res))))))
        print(73, left(some(right(some(right(some(res)))))))
        print('$false, none?(right(some(right(some(right(some(res))))))))
        print(31, left(some(right(some(right(some(right(some(res)))))))))
        print('$true, none?(right(some(right(some(right(some(right(some(res))))))))))
    }
}
