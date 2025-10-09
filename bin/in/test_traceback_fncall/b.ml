
var fn (x):{
    x()
}

```
    passed argument is suppose to be a function..
    ..returning another function but we make it fail..
    ..on purpose
```
fn(die())

```
    should output:
    die() with no message
        at fn(b.ml:11:4)
        at fn(b.ml:3:5)
        at <program>(b.ml:11:1)
```
