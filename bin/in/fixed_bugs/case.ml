
```
    memory corruption induced by delete
```

var CaseAnalysis ():{
    var end $false
    var fn (cond, do):{
        $true == $true && {
            die("additional case succeeding a fallthrough case")
        }
    }
    fn
}

var case CaseAnalysis()

case(eq(1, 1), {
    print("A")
})
