
var tern (cond, if_false, if_true):{
    var res $nil
    cond || {res := if_false}
    cond && {res := if_true}
    res
}

var not (bool):{
    tern(bool, $true, $false)
}

var is_even' (is_even', is_odd', n):{
    tern(n, $true, not(is_odd'(is_even', is_odd', n)))
}

var is_odd' (is_even', is_odd', n):{
    is_even'(is_even', is_odd', n + -1)
}

var is_even (n):{
    is_even'(is_even', is_odd', n)
}

var is_odd (n):{
    is_odd'(is_even', is_odd', n)
}

print(is_odd(5))
