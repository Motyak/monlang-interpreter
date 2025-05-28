
var tern (cond, if_true, if_false):{
    var res $nil
    cond && {res := if_true}
    cond || {res := if_false}
    res
}

var !tern (cond, if_false, if_true):{
    tern(cond, if_true, if_false)
}

var - _
- := (lhs, rhs):{
    !tern(rhs, lhs, {
        -(lhs, rhs + -1) + -1
    })
}

var make-account (initial):{
    var bank initial

    var balance ():{
        bank
    }

    var deposit (n):{
        bank += n
    }

    var withdraw (n):{
        bank -= n
    }

    '-----------------------

    var dispatcher (msg_id):{
        !tern(msg_id, balance, {
            !tern(msg_id - 1, deposit, {
                !tern(msg_id - 2, withdraw, {
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

    var acc make-account(10)

    print('acc10, acc(balance)())

    acc(deposit)(3)
    print('acc13, acc(balance)())

    acc(withdraw)(10)
    print('acc3, acc(balance)())
}

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

