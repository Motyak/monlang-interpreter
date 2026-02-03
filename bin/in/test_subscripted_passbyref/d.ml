```
    Should output:
    ["a":["a":"xxx"], "x":"fds"]
```

var fn1 (a):{
    a['a] := 'xxx
}

var evalProgram (a):{
    fn1(&a['a])
    print(a)
    -- fn1(&a)
}

{
    var context [
        'x => 'fds
    ]
    evalProgram(context) -- PASSED BY DELAY HERE
}
