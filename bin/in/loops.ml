

var while _
while := (cond, do):{
    cond() && {
        do()
        while(cond, do)
    }
}

var - {
    var tern (cond, if_true, if_false):{
        var res _
        cond && {res := if_true}
        cond || {res := if_false}
        res
    }

    var !tern (cond, if_false, if_true):{
        tern(cond, if_true, if_false)
    }

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

var n_times (n, do):{
    var i 0
    while(():{i + -(n)}, ():{
        i += 1
        do()
    })
}

n_times(5, ():{print("hello")})
