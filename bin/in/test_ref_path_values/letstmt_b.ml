```
    test environment forking (symbols + variables)
    when aliasing a variable (let stmt)

    Output should be:
    0
    [1:["field":123], 2:["field":2]]
```

var map [1:['field:1], 2:['field:2]]
var i 0

let var map[{
    i += 1
    i
}].field

var := 123
var := 123

print(i)
print(map)
