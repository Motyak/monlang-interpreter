var fn (OUT x):{
    print(x)
    x := 13
    print(x)
}

var &_ _

fn((&_))
```
    prints:
    $nil
    13
```

print("---")

fn(&_)
```
    prints:
    $nil
    $nil (because PassByRef to `_` Symbol..
          ..will call evaluateValue on `_` Symbol..
          ..and it always evaluates to $nil (its a special name))
```
