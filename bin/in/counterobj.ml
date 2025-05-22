
var make-counter (start):{
    var i start

    var inc ():{
        i += 1
        i
    }

    inc
}

var count make-counter(0)
var count' make-counter(99)

print('count1, count())
print('count'100, count'())
print('count2, count())
print('count'101, count'())
