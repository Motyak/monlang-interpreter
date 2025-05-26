
var proc_b (x):{
    print('proc_b)
    ```
        discards x,
        transforms it to a local variable
    ```
    x := _
    $nil
}

var proc_c (x):{
    print('proc_c)
    ```
        uses x
    ```
    x
    $nil
}

var proc_a (x):{
    print('proc_a)
    ```
        uses x indirectly
    ```
    proc_b(x)
    proc_c(x)
    $nil
}

var id (x):{
    print('id)
    x
}

proc_a(id(_))
```
    `id` is printed only once
    =>
    id() is evaluated only once,
    in proc_c() (not proc_b)
    =>
    pass by delay works
```
