print("start")

var a ():{
    print("we are in a()")
}

var b ():{
    print("we are in b()")
    print("calling a")
    a(1)
}

print("calling b")
b()

print("end")
