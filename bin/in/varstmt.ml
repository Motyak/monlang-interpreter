
```
    in var stmt,
    when evaluating lvalue before value,
    this program will print `something went wrong`

    you can test this program with
    git checkout b7945464c702ed40f685c0a8d7d4980bec8f5699
```

var check-nil (x):{
    x || {
        print("something went wrong")
        exit(1)
    }
}

var fn (x):{
    ```
        discards x,
        transforms the PassByDelayed into a local Variable
        (this is why we need to evaluate value before lvalue)
    ```
    x := check-nil(x)
}

fn(13)
