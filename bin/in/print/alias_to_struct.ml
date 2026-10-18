struct Person {
    Int age
    Str _name
}

type Employee Person

var e Employee(29)
print(e) -- ```
    should print:
    Employee{29, ""}
```
