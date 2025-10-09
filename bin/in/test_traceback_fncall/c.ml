
```
    supposed to be returning a function..
    ..but we make it fail on purpose
```
var fn2 ():{
    die()
}

var fn (x):{
    x()
}

fn(fn2())

```
    should output:
    die() with no message
        at fn2(c.ml:7:5)
        at fn(c.ml:14:4)
        at fn(c.ml:11:5)
        at <program>(c.ml:14:1)
```
