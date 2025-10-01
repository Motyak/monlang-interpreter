
var delay (x):{
    var delayed ():{x}
    delayed
}

var delayed {
    delay(a)
}
var a 0

var res delayed()
print(res)


```
    This used to not work (`a` would be accessible from `delayed`)
    if not performing rec_copy() on PassByDelayed thunkEnv
```
