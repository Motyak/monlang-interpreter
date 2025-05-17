
var tern (cond, if_false, if_true):{
    var res $nil
    cond || {res := if_false}
    cond && {res := if_true}
    res
}

var *' (*', lhs, rhs):{
    tern(lhs, 0, {
        tern(lhs + -1, rhs, {
            rhs + *'(*', lhs + -1, rhs)
        })
    })
}

var * (lhs, rhs):{
    *'(*', lhs, rhs)
}

print(2 * 3 * 4)
