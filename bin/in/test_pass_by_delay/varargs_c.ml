```
    test PassByDelay memoization through variadic arguments.
    multiple layers of variadic arguments passing

    Output should be:
    proc_a
    proc_b
    proc_c
    id
    proc_c
```

var proc_c (x, y):{
    print('proc_c)
    x
    y
    ;
}

var proc_b (xs...):{
    print('proc_b)
    proc_c(xs..., xs...)
    ;
}

var proc_a (xs...):{
    print('proc_a)
    proc_b(xs...)
    proc_c(xs..., xs...)
    ;
}

var id (x):{
    print('id)
    x
}

proc_a(id(_))
