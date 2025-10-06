
```
    test environment forking (symbols + variables)
    when passing by ref

    Output should be:
    2
    [1:["field":123], 2:["field":2]]
```

var i 1

var fn (x):{
    x := 123
    i := 2
    x := 123
}

var map [1:['field:1], 2:['field:2]]

fn(&map[i].field)

print(i)
print(map)
