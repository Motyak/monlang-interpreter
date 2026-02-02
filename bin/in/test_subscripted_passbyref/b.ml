```
    Shouldn't report any error
```

var fn1 (a):{
    a['a] := 'xxx -- "autovivification (.a doesn't exist yet)"
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
