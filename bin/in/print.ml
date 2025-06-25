var tern (cond, if_true, if_false):{
    var res _
    cond && {res := if_true}
    cond || {res := if_false}
    res
}

var !tern (cond, if_false, if_true):{
    tern(cond, if_true, if_false)
}

var -len {
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

    var -len (n):{
        -(len(n))
    }

    -len
}

var curry (fn):{
    var curried _

    curried := (args...):{
        !tern($#varargs + -len(fn), fn(args...), {
            (args2...):{curried(args..., args2...)}
        })
    }

    curried
}


var |> (input, fn):{
    fn(input)
}

var print {
    var builtin_print print
    curry((x):{builtin_print(x)})
}

123 |> print
123 |> print()
print |> print
