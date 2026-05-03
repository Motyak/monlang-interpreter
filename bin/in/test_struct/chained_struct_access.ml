```
    Dude(Name("Tommy"), UInt(29))
    Dude(Name("fds"), UInt(29))
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

print(tommy)


var fn (OUT x):{
    x := "fds"
}

fn(&tommy.name.name)

print(tommy)
