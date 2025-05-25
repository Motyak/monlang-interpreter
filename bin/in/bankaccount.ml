
'===Pair==============================

var Pair (left, right):{
    (selector):{selector(left, right)}
}

var left (pair):{
    pair((left, right):{left})
}

var right (pair):{
    pair((left, right):{right})
}

'=====================================

var tern (cond, if_false, if_true):{
    var res $nil
    cond || {res := if_false}
    cond && {res := if_true}
    res
}

var -' (-', lhs, rhs):{
    tern(rhs, lhs, {
        -1 + -'(-', lhs, rhs + -1)
    })
}

var - (lhs, rhs):{
    -'(-', lhs, rhs)
}

var make-account (initial):{
    var bank initial

    var balance ():{
        left(bank)
    }

    var deposit (n):{
        bank := Pair(left(bank) + n, _)
    }

    var withdraw (n):{
        bank := Pair(left(bank) - n, _)
    }

    '-----------------------

    var dispatcher (msg_id):{
        tern(msg_id, balance, {
            tern(msg_id - 1, deposit, {
                tern(msg_id - 2, withdraw, {
                    print("ERR invalid msg_id in dispatcher: `" + msg_id + "`")
                    exit(1)
                })
            })
        })
    }

    dispatcher
}

```
    using OOP-like paradigm
```
{
    var balance 0
    var deposit 1
    var withdraw 2

    var acc make-account(Pair(10, _))

    print('acc10, acc(balance)())

    acc(deposit)(3)
    print('acc13, acc(balance)())

    acc(withdraw)(10)
    print('acc3, acc(balance)())
}
exit(0)

```
    using procedural-like paradigm
```
{
    var balance (acc):{acc(0)()}
    var deposit (acc, n):{acc(1)(n)}
    var withdraw (acc, n):{acc(2)(n)}

    var acc make-account(10)

    print('acc10, balance(acc))

    deposit(acc, 3)
    print('acc13, balance(acc))

    withdraw(acc, 10)
    print('acc3, balance(acc))
}

