
```
    chaining function call
```
var fn ():{
    var fn ():{die()}
    fn
}

fn()()

```
    should output:
    die() with no message
        at <program>(e.ml:6:16)
        at <program>(e.ml:10:1)
```
