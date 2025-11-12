
```
    deepcopy() on Map should deepcopy the keys as well
    (not just the values)

    Output should be:
    ["a":1, "b":2]
    ["a":1, "b":2]
```

var key1 'a
var key2 'b
var map [key1:1, key2:2]
var map2 map

print(map2)
key1[#1] := 'b
print(map2)
