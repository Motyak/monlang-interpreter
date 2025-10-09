```
    test environment forking (symbols + variables)
    when aliasing a variable (let stmt)

    Output should be:
    2
    [1:["field":123], 2:["field":2]]
```

var map [1:['field:1], 2:['field:2]]
var i 1

let var map[i].field

var := 123
i := 2
var := 123

print(i)
print(map)
