

type Fds Int

struct Person {
    Str name
    Int age
}

var fds Fds(29)
-- var p Person("Tommy", fds) -- "ERR: age need to be an `Int` strictly, no aliasing allowed"
var p Person("Tommy", Int(fds))
print(p)
