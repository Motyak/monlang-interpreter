
var swap (OUT a, OUT b):{
    a
    b
    a := b
    b := $old
}

"both passed by ref"
{
    var a 13
    var b 37
    swap(&a, &b)
    print(a, b)
}

"none passed by ref"
{
    var a 13
    var b 37
    swap(a, b)
    print(a, b)
}

"only `a` passed by ref"
{
    var a 13
    var b 37
    swap(&a, b)
    print(a, b)
}

"only `b` passed by ref"
{
    var a 13
    var b 37
    swap(a, &b)
    print(a, b)
}
