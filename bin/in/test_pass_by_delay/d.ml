


var fn2 (OUT x):{
    print('fn2)
    ```
        discard x,
        transform it to a nonlocal variable
    ```
    print(x)
    x := 0
}

var fn1 (x):{
    print('fn1)
    ```
        uses x indirectly
    ```
    fn2(&x)
    print(x)
}

var id (x):{
    print('id)
    x
}

fn1(id(91))
```
    `id` is never printed
    =>
    id() is never evaluated
    =>
    pass by delay + pass by ref works
```
