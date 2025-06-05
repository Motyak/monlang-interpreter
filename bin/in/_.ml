
var eval (lambda):{
    lambda()
}

var fn (arg):{
    eval(():{arg := 0})
    print(arg)
}

fn(91)

exit(0)

var mutate (OUT x):{x := 0}

var fn (arg):{
    mutate(&arg)
    print(arg)
}

fn(91)

exit(0)

var eval (lambda):{
    lambda()
}

var fn (arg):{
    eval(():{arg})
}

fn(10)

exit(0)

var eval (lambda):{
    var a 91
    lambda()
}

var a 10
eval(():{a})
