```
    Should output:
    ["a":["a":"xxx"], "x":"fds"]
```

{
    var context [
        'x => 'fds
    ]

    let a context['a]
    a['a] := 'xxx
    print(context)
}
