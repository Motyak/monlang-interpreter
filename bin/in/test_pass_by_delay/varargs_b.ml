```
    test PassByDelay memoization through variadic arguments.
    two functions call with the same variadic arguments.

    Output should be:
    proc_a
    proc_b
    id
    proc_c
```

var proc_b (x):{
    print('proc_b)
    x
    ;
}

var proc_c (x):{
    print('proc_c)
    x
    ;
}

var proc_a (xs...):{
    print('proc_a)
    proc_b(xs...)
    proc_c(xs...)
    ;
}

var id (x):{
    print('id)
    x
}

proc_a(id(_))
