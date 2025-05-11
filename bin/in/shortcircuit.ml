
var call_binary_op (op, lhs, rhs):{
    op(lhs, rhs)
}

call_binary_op(&&, $false, print("<not printed>"))
&&($false, print("<not printed>"))
$false && print("<not printed>")

call_binary_op(||, $true, print("<not printed>"))
||($true, print("<not printed>"))
$true || print("<not printed>")
