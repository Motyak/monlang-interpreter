

var map [:]

map['a]['b]! := 123

print(map)

```
    should output:
    Runtime error: Subscript key not found (src/interpret.cpp:1046)
        at <program>(BUG_autovivification_map.ml:3:9)
        at <program>(BUG_autovivification_map.ml:3:1)
```
