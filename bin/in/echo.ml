"interactive mode enabled"

print("args=" + $args)
print("len(args)=" + len($args))

var tern (cond, if_true, if_false):{
    var res $nil
    cond && {res := if_true}
    cond || {res := if_false}
    res
}

var until _
until := (cond, do):{
    cond() || {
        do()
        until(cond, do)
    }
}

_ := ```
    this function slightly differs from builtin stdin()
    in that the last trailing newline isn't part of the input,
    and there is no way to know whether the last getline line
    had a trailing newline or if it just hit EOF
```
var getlines ():{
    var res ""
    var line getline()

    var first_it $true
    until(():{line == $nil}, ():{
        first_it || {res += "\n"}
        res += line
        line := getline()
        first_it := $false
    })

    res
}

var echo ():{
    print(getlines())
}

echo()
