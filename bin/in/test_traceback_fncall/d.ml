
```
    this time fn2 succeeds, but return a function..
    ..that will fail
```
var fn2 ():{
    var fn ():{die()}
    fn
}

var fn (x):{
    x()
}

fn(fn2())

```
    should output:
    die() with no message
        at x(d.ml:7:16)
        at fn(d.ml:12:5)
        at <program>(d.ml:15:1)
```
