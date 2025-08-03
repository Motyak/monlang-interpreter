```
    UPDATE: maybe its intended after all
```


var binary_proc (a, b):{
    print('binary_proc)
    ```
        uses both a and b
    ```
    a
    b
    $nil
}

var unary_proc (x...):{
    print('unary_proc)
    ```
        uses x indirectly
    ```
    binary_proc(x..., x...)
    $nil
}

var id (x):{
    print('id)
    x
}

unary_proc(id(_))
```
    `id` is printed only once
    =>
    id() is evaluated only once,
    in binary_proc, despite having two
    different names (parameters) bound to it
    =>
    pass by delay works
```
