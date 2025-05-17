
print("start")

var a ():{
    print("we are in a()")
    print("calling +")
    ():{} + 1 + 2
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
    Runtime error: +() first arg cannot be Lambda (src/builtin/operators/plus.cpp:51)
        at +(bin/in/stacktrace.ml:7:11)
        at +(bin/in/stacktrace.ml:7:15)
        at a(bin/in/stacktrace.ml:12:5)
        at b(bin/in/stacktrace.ml:16:1)
```
