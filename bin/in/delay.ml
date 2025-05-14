
var delay (expr):{
    ():{expr}
}

print("<defining eval_a>")
var eval_a delay({
    print("hello")
    91
})

print("<calling eval_a>")
eval_a()

