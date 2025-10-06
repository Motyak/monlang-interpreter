```
    test environment forking (symbols + variables)
    when passing by ref

    Output should be:
    i 3
    list [1, 2, 2]
```

var swap (OUT a, OUT b):{
    var tmp a
    a := b
    b := tmp
}

var i 1
var list [3, 2, 2]
swap(&i, &list[#i])

print('i, i)
print('list, list)
