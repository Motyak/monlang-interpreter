```
    var fds (x):{
        print("lamb")
        fds(x)
    }

    fds(0)

    this program would trigger, as expected, unbound
    symbol `fds` when evaluating the function call.
    BUT...
```

var fds {
    print("begin")
    var fds (x):{
        print("lamb")
        fds(x)
    }
    fds
}

fds(0)

```
    we successfully bypassed another way to do self-reference
    without forward declaration and without passing self as
    parameter.
```
