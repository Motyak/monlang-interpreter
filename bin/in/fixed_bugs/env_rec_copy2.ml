
var delay (OUT x):{
    var delayed ():{x := 10}
    delayed
}

var delayed {
    delay(&a)
}
var a 0

delayed()
print(a)


```
    This used to not work (`a` would be accessible from `delayed`)
    if not performing rec_copy() on PassByRef thunkEnv
```
