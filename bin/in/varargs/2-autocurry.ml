var curry (fn):{
    var curried _

    curried := (args...):{
        !tern($#varargs + -len(fn), fn(args...), {
            (args2...):{curried(args..., args2...)}
        })
    }

    curried
}

var add (a, b, c):{
    a + b + c
}

var curried_add curry(add)
curried_add(1)(2)(3)
curried_add(1, 2)(3)
curried_add(1)(2, 3)
curried_add(1, 2, 3)
