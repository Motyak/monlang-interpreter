


var Pair (left, right):{
    (selector):{selector(left, right)}
}

var Pair::left (pair):{
    var left-selector (left, right):{left}
    pair(left-selector)
}

var Pair::right (pair):{
    var right-selector (left, right):{right}
    pair(right-selector)
}

'---------------------------------------

var tern (cond, if_false, if_true):{
    var res $nil
    cond || {res := if_false}
    cond && {res := if_true}
    res
}

var List Pair

var Queue ():{
    var left Pair::left
    var right Pair::right

    var m_back List($nil, $nil)

    var push (x):{
        print('push)
        var merge (list1, list2):{
            List(list1, 37)
        }
        print('ok)
        m_back := merge(m_back, List(13, 37))
    }

    var front ():{
        print('front)
        left(m_back)
    }

    var pop ():{
        tern(left(m_back), $nil, {
            var res left(m_back)
            m_back := right(m_back)
            res
        })
    }

    '-----------------------

    var dispatcher (msg_id):{
        tern(msg_id, push, {
            tern(msg_id + -1, front, {
                tern(msg_id + -2, pop, {
                    print("ERR invalid msg_id in dispatcher: `" + msg_id + "`")
                    exit(1)
                })
            })
        })
    }
    
    dispatcher
}

'---------------------------------------

var push 0
var front 1
var pop 2
var queue Queue()

print(queue(front)())

queue(push)(91)
print("test")
print(queue(front)())
exit(0)
print(res)



