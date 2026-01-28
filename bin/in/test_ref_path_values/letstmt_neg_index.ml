```
    test path values are delayed and memoized
    when aliasing a variable (let stmt)

    Output should be:
    3
    3
```

var list [1, 2]
let x list[#-1]

list := [1, 2, 3]
print(x) -- path values are evaluated here (delayed)
list := [1, 2, 3, 4]
print(x) -- path values are not re-evaluated (memoized)
