```
    ["fds":"sdf"]
```

struct MyStruct {
    Map map
}

var mystruct MyStruct([:])

let x mystruct.map
x['fds] := "sdf"
print(x)
