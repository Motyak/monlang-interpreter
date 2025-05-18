
var tern (cond, if_false, if_true):{
    var res $nil
    cond || {res := if_false}
    cond && {res := if_true}
    res
}

var * {
    var * (lhs, rhs):{}

    * := (lhs, rhs):{
        tern(lhs, 0, {
            tern(lhs + -1, rhs, {
                rhs + *(lhs + -1, rhs)
            })
        })
    }

    *
}

var fact {
    var fact (n):{}

    fact := (n):{
        tern(n + -1, 1, {
            n * fact(n + -1)
        })
    }

    fact
}

print(fact(5))
