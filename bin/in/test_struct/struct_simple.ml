```
    Dude("Tommy", UInt(29))
    name Tommy
    age UInt(29)
    $true
    ["age":UInt(29), "name":"Tommy"]
    Dude("fds", UInt(29))
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

struct Person {
    Str name
    UInt age
}

type Dude Person

var tommy Dude("Tommy", UInt(29))

print(tommy)
print('name, tommy.name)
print('age, tommy.age)

print(tommy is 'Dude)

print([:] | tommy)

tommy.name := "fds"
-- tommy.age := 123
-- tommy.age := UInt(-123)
-- tommy.age := UInt(123)
-- tommy.age += 1

print(tommy)
