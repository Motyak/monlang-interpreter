
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
    left
    right
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
    some?
    val

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
    left
    right
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

'===Queue=============================

var Queue ():{
    var list END

    var front ():{
        print('front)
        none?(list) && {
            print("ERR calling front() on empty Queue")
            exit(1)
        }
        left(list)
    }

    var push (x):{
        print('push)
        list := merge(list, Pair?(x, END))
    }

    var pop ():{
        print('pop)
        tern(none?(list), {}, {
            list := right(some(list))
        })
    }

    '---------------------

    var dispatcher (msg_id):{
        print('dispatcher)
        !tern(msg_id, front, {
            !tern(msg_id + -1, push, {
                !tern(msg_id + -2, pop, {
                    print("ERR invalid msg_id in dispatcher: `" + msg_id + "`")
                    exit(1)
                })
            })
        })
    }

    dispatcher
}

var front (queue):{queue(0)()}
var push (queue, x):{queue(1)(x)}
var pop (queue):{queue(2)()}

var queue Queue()
push(queue, 14)
push(queue, 28)
push(queue, 57)
print(14, front(queue))

```
    Runtime error: Bool() arg cannot be a Lambda (src/builtin/prim_ctors/Bool.cpp:50)
        at tern(bin/in/BUG.ml:4:10)
        at !tern(bin/in/BUG.ml:10:5)
        at pair(bin/in/BUG.ml:54:9)
        at left(bin/in/BUG.ml:26:5)
        at front(bin/in/BUG.ml:115:9)
        at front(bin/in/BUG.ml:147:20)
        at print(bin/in/BUG.ml:155:11) <------------------ ça devrait pas afficher ça ?
        at <program>(bin/in/BUG.ml:155:1)
```
