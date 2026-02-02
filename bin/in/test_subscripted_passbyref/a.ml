```
    Shouldn't report any error
```

var fn1 (a):{
    -- a.a
    a.a := 'xxx
}

var evalProgram (a):{
    fn1(&a)
    -- a.a := 'xxx
}

{
    var context [
        'a => 'fds
    ]
    evalProgram(context) -- PASSED BY DELAY HERE
}
