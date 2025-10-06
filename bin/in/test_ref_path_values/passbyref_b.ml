
```
    test environment forking (symbols + variables),
    as well as path values memoization,
    when passing by ref

    Output should be:
    0
    [1:["field":123], 2:["field":2]]
```

var fn (x):{
    x := 123
    x := 123
}

var map [1:['field:1], 2:['field:2]]

var i 0

fn(&map[{
    i += 1
    i
}].field)

print(i)
print(map)
