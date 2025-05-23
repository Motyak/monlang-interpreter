
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

var myvar Variable(13)
myvar(get)()
myvar(set)(37)
myvar(get)()

