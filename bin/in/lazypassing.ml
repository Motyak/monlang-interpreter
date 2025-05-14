{
    var fn (x):{}
    fn(print("<not printed>"))
}

{
    var somevar 91
    var fn (x):{
        somevar += 1
        print('local93, x)
        x += 1
        print('local94, x)
    }
    print('somevar91, somevar)
    fn(somevar + 1)
    print('somevar92, somevar)
}
