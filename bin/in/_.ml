
var Pair (left, right):{
    (selector):{selector(left, right)}
}

var left (pair):{
    var left-selector (left, right):{left}
    pair(left-selector)
}

var right (pair):{
    var right-selector (left, right):{right}
    pair(right-selector)
}

var tern (cond, if_false, if_true):{
    var res $nil
    cond || {res := if_false}
    cond && {res := if_true}
    res
}

var Variable (start):{
    var val start

    var get ():{
        val
    }

    var set (new_val):{
        val := new_val
    }

    '--------------------

    var dispatcher (msg_id):{
        tern(msg_id, get, {
            tern(msg_id + -1, set, {
                print("ERR invalid msg_id in dispatcher: `" + msg_id + "`")
                exit(1)
            })
        })
    }

    dispatcher
}

var get 0
var set 1

var myvar Variable(Pair(13, 37))
left(myvar(get)())
right(myvar(get)())

myvar(set)(Pair(37, 13))
left(myvar(get)())
right(myvar(get)())

