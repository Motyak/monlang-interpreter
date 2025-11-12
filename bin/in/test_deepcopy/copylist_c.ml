
```
    PassByDelay thunk (with memoization) should deepcopy()
    the evaluated result (only the first time)

    Output should be:
    [1, 2, 3]
    [1, 2, 3]
```

var delay (x):{
    var delayed ():{x}
    delayed
}

var list [1, 2, 3]
var list2 delay(list)
print(list2())
list[#1] := 0
print(list2())
