{
    var fn (x):{}
    fn(print("<not printed>"))
}

{
    var fn (x):{
        print('local91, x)
        x += 1
        print('local92, x)
    }
    var somevar 91
    fn(somevar)
    print('somevar91, somevar)
}
