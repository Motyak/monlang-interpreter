var fn (out):{
    print(out)
    (():{out += 1})()
    (():{
        (():{out += 1})()
    })()
    print(out)
}

fn(91)
```
    prints 93
```
