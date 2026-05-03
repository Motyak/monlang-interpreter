```
    [Dude(Name("fds"), UInt(29))]
```

var >= (a, b):{
    a > b || a == b
}

type UInt Int
{
    var tag UInt
    UInt := (n):{
        n >= 0 || die()
        tag(n)
    }
}

struct Name {
    Str name
}

struct Person {
    Name name
    UInt age
}

type Dude Person

var tommy Dude(Name("Tommy"), UInt(29))

var dudes [tommy]


var i 1
let x dudes[#i].name.name
x := "fds"
print(dudes)
