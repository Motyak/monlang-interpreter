```
    Used to have a segfault or undefined behavior, such as:
    terminate called after throwing an instance of 'std::bad_alloc'
      what():  std::bad_alloc

    because we were passing the address of a local variable, stored in the stack,
    but then later on we re-use this same stack space for another stackframe
    and so assigning to a local variable (paramtersBinding in that case)
    was resulting in overwriting the stack space that was pointed to
    (and that originally contained the correct data) and now we have
    trash data interpreted as an Environment (std::map with millions of entries, etc..)

    The solution was to allocate on the heap.
    We never encountered the issue before because it was working accidentally,
    the stack memory was luckily never overwritten.
```

var not (bool):{
    ==($false, bool)
}

var <> (a, b):{
    a == b == $false
}

var <= (a, b):{
    a > b == $false
}

var >= (a, b):{
    a > b || a == b
}

var < (a, b):{
    (a > b || a == b) == $false
}

var tern (cond, if_true, if_false):{
    var res _
    cond && {res := if_true}
    cond || {res := if_false}
    res
}

var !tern (cond, if_false, if_true):{
    tern(cond, if_true, if_false)
}

var CaseAnalysis (pred):{
    $type(pred) == 'Lambda || die("CaseAnalysis pred isn't a Lambda")
    var end $false
    var fn (val, do):{
        end <> $nil || die("additional case succeeding a fallthrough case")
        end ||= pred(val) && {
            _ := do
            $true
        }
        "NOTE: don't eval val if end"
        end == $false && val == $nil && {
            _ := do
            end := $nil
        }
        ;
    }
    fn
}

var until (cond, do):{
    var 1st_it? $true
    var loop _
    loop := ():{
        cond() || {
            do(1st_it?)
            1st_it? := $false
            _ := loop()
        }
    }
    loop()
    $nil
}

var do_while (do, cond):{
    var 1st_it? $true
    do(1st_it?)
    1st_it? := $false

    var loop _
    loop := ():{
        cond() && {
            do(1st_it?)
            _ := loop()
        }
    }
    loop()
    ;
}

var - {
    var sub-1 (n):{
        n + n * -2
    }
    var sub-2 (a, b):{
        a + b + b * -2
    }
    var - (varargs...):{
        tern($#varargs == 1, sub-1(varargs...), {
            tern($#varargs == 2, sub-2(varargs...), {
                die("-() takes either 1 or 2 args")
            })
        })
    }
    -
}

var .. (from, to):{
    $type(from) == 'Str && {from := Byte(from)}
    $type(to) == 'Str && {to := Byte(to)}
    var dispatcher (msg):{
        tern(msg == 'from, from, {
            tern(msg == 'to, to, {
                die("unknown range msg: `" + msg + "`")
            })
        })
    }
    dispatcher
}

var foreach {
    var Container::foreach (OUT container, fn):{
        var nth 1
        until(():{nth > len(container)}, (_):{
            fn(&container[#nth])
            nth += 1
        })
        container
    }

    var Range::foreach (range, fn):{
        var i range('from)
        var to range('to)
        until(():{i > to}, (_):{
            fn(i)
            i += 1
        })
    }

    var foreach (iterable, fn):{
        tern($type(iterable) == 'Lambda, Range::foreach(iterable, fn), {
            Container::foreach(&iterable, fn)
        })
    }

    (foreach)
}

var in {
    var Container::in (elem, container):{
        var nth 1
        var found $false
        until(():{found || nth > len(container)}, (_):{
            found := container[#nth] == elem
            nth += 1
        })
        found
    }

    var Range::in (elem, range):{
        elem >= range('from) && elem <= range('to)
    }

    var in (elem, iterable):{
        tern($type(iterable) == 'Lambda, Range::in(elem, iterable), {
            Container::in(elem, iterable)
        })
    }

    in
}

var !in (elem, container):{
    in(elem, container) == $false
}

"autocurries until the nb of required args has been reached"
var curry_required (requiredArgs, fn):{
    var curried _
    curried := (args...):{
        tern($#varargs >= requiredArgs, fn(args...), {
            (args2...):{curried(args..., args2...)}
        })
    }
    curried
}

"calling curry on a function with no required argument.."
"..has no effect => use curry_required instead"
var curry (fn, args...):{
    curry_required(len(fn), fn)(args...)
}

var curry_rhs (fn, rhs):{
    -- fn
    -- rhs
    var curried (lhs):{
        fn(lhs, rhs)
    }
    curried
}

var foreach_do {
    var foreach_do (fn, iterable):{
        foreach(iterable, fn)
    }
    curry(foreach_do)
}

var map {
    var map (fn, iterable):{
        var str? $type(iterable) == 'Str
        var res tern(str?, "", [])
        foreach(iterable, (x):{
            res += tern(str?, fn(x), [fn(x)])
        })
        res
    }
    curry(map)
}

var filter {
    var filter (pred, iterable):{
        var str? $type(iterable) == 'Str
        var res tern(str?, "", [])
        foreach(iterable, (x):{
            pred(x) && {
                res += tern(str?, x, [x])
            }
        })
        res
    }
    curry(filter)
}

var reduce {
    var reduce (acc, fn, iterable):{
        foreach(iterable, (curr):{
            acc := fn(acc, curr)
        })
        acc
    }
    curry(reduce)
}

var compose (fn1, fn2, fns...):{
    var compose (fn1, fn2):{
        fn1
        fn2
        (x):{fn2(fn1(x))}
    }
    reduce(fn1, compose, List(fn2, fns...))
}

var split {
    var split (sep, str):{
        len(sep) == 1 || die("only support 1 char sep atm")
        var res []
        var curr ""
        foreach(str, (c):{
            tern(c <> sep, {curr += c}, {
                res += [curr]
                curr := ""
            })
        })
        len(curr) > 0 && {res += [curr]}
        res
    }
    curry(split)
}

var join {
    var join (sep, list):{
        var res ""
        var first_it $true
        foreach(list, (str):{
            first_it || {res += sep}
            res += str
            first_it := $false
        })
        res
    }
    curry(join)
}

var any {
    var any (pred, container):{
        var any_true $false
        var nth 1
        until(():{any_true || nth > len(container)}, (_):{
            pred(container[#nth]) && {any_true := $true}
            nth += 1
        })
        any_true
    }
    curry(any)
}

var none {
    var none (pred, container):{
        not(any(pred, container))
    }
    curry(none)
}

var all {
    var UnaryPred::all (pred, container):{
        var any_false $false
        var nth 1
        until(():{any_false || nth > len(container)}, (_):{
            pred(container[#nth]) || {any_false := $true}
            nth += 1
        })
        not(any_false)
    }

    var at (container, nth):{
        container[#nth]
    }

    "for chained relational operator, e.g.: all(==, [1, 1.0, Byte(1)]) <=> 1 == 1.0 && 1.0 == Byte(1)"
    var BinPred::all (pred, container):{
        var any_false $false
        var nth 2
        until(():{any_false || nth > len(container)}, (_):{
            var lhs at(container, nth - 1)
            var rhs at(container, nth)
            pred(lhs, rhs) || {any_false := $true}
            nth += 1
        })
        not(any_false)
    }

    var all (pred, container):{
        tern(len(pred) == 1, UnaryPred::all(pred, container), {
            tern(len(pred) == 2, BinPred::all(pred, container), {
                die("all() pred param requires either 1 or 2 args")
            })
        })
    }
    curry(all)
}

var |> (input, fn):{
    fn(input)
}

'=============================



var fn (x):{
    var is_elemtype curry_rhs(is, x)
    all(is_elemtype)
}
fn("Str")("fds")
