```
    MyStruct([])
    MyStruct([MyStruct([7, 7, 7])])
```

struct MyStruct {
    List things
}

var somestruct MyStruct([
    MyStruct([])
])

var fn (OUT x):{
    x.things := [7, 7, 7]
    x
}

print(somestruct.things[#1])
fn(&somestruct.things[#1])
print(somestruct)
