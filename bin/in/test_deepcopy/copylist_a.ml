
```
    VarStatement should deepcopy() the value

    Output should be:
    [1, 2, 3]
    [1, 2, 3]
```

var list [1, 2, 3]
var list2 list

print(list2)
list[#1] := 0
print(list2)
