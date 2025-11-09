```
    test PassByDelay memoization through variadic arguments.
    passing twice the same variadic arguments

    Output should be:
    unary_proc
    binary_proc
    id
```

var binary_proc (a, b):{
    print('binary_proc)
    a
    b
    ;
}

var unary_proc (xs...):{
    print('unary_proc)
    binary_proc(xs..., xs...)
    ;
}

var id (x):{
    print('id)
    x
}

unary_proc(id(_))
