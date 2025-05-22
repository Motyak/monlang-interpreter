
print("start")

var a ():{
    print("we are in a()")
    print("calling +")
    fds + 1 + 2
}
var b ():{
    print("we are in b()")
    print("calling a")
    a()
}

print("calling b")
b()

print("end")

```
    start
    calling b
    we are in b()
    calling a
    we are in a()
    calling +
    Runtime error: Unbound symbol `fds` (src/interpret.cpp:417)
        at +(bin/in/unbound.ml:7:5)
        at +(bin/in/unbound.ml:7:9)
        at a(bin/in/unbound.ml:7:13)
        at b(bin/in/unbound.ml:12:5)
        at <program>(bin/in/unbound.ml:16:1)
```
