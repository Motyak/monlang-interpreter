


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
        var merge (merge, list1, list2):{
            tern(left(list1), list2, {
                List(left(list1), merge(merge, right(list1), list2))
            })
        }
        m_back := merge(merge, m_back, List(x, $nil))
    }

    var front ():{
        left(m_back)
    }

    var pop ():{
        tern(left(m_back), $nil, {
            var res left(m_back)
            m_back := right(m_back)
            res
        })
    }

    (msg_id):{
        tern(msg_id, push, {
            tern(msg_id + -1, front, {
                tern(msg_id + -2, pop)
            })
        })
    }
}

'---------------------------------------

var push 0
var front 1
var pop 2
var queue Queue()

print(queue(front)())

queue(push)(91)
queue(front)()
exit(0)
print(res)


