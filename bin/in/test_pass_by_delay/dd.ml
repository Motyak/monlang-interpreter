


var fn2 (OUT x):{
    print('fn2)
    ```
        uses x indirectly (gets evaluated),
        mutates it
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
    `id` is printed once
    =>
    id() is evaluated only once,
    when printing it in fn(2)
    =>
    pass by delay + pass by ref works
```
