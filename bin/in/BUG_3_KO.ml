```
    # (doesn't stop at 92, like it should)
    bin/main.elf bin/in/varargs/test2.ml | head -n10
```

var tern (cond, if_true, if_false):{
    var res _
    cond && {res := if_true}
    cond || {res := if_false}
    res
}

var !tern (cond, if_false, if_true):{
    tern(cond, if_true, if_false)
}

var while _
while := (cond, do):{
    cond() && {
        do()
        while(cond, do)
    }
}

var - {
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

var main (out):{
    while(():{
        print('cond, out)
        out + -93
    }, ():{
        print('do, out)
        out += 1
    })
}

main(91)
