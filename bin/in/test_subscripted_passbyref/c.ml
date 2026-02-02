```
    Should report err "Subscript key not found"
```

var fn1 (a):{
    a['a] -- ERR
}

var evalProgram (a):{
    fn1(&a)
}

{
    var context [
        'x => 'fds
    ]
    evalProgram(context) -- PASSED BY DELAY HERE
}
