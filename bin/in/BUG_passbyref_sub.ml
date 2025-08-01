
```
    When lvaluing a subcript, we should
    immediatly evaluate the value of any
    index/range that refer to a symbol.
    Same for preceding subscript (in case
    of nested subscript).
```

var swap (OUT a, OUT b):{
    a := b
    b := $old
}

var i 1
var list [3, 2, 2]
swap(&i, &list[#i])

print('i, i)
print('list, list)

```
    this outputs:
    `i 3`
    `list [3 2 3]`

    instead of:
    `i 3`
    `list [1 2 2]`
```

```
    one possible workaround, in order to
    immediatly evaluate the index, is to create
    a new variable and capture this variable instead
    (here the variable is nth):

    var i 1
    var list [3, 2, 2]
    {
        var nth i
        swap(&i, &list[#nth])
    }
```
