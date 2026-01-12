

var map ['a:$nil]

map['a]['b] := 123

print(map)

```
    should output:
    Runtime error: lvaluing a $nil subscript array (src/interpret.cpp:985)
        at <program>(BUG_autovivification_map.ml:4:1)
```
