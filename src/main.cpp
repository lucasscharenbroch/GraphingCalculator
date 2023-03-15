#include "parser.h"
// #include "calculator.h"

int main() { // TODO remove (test method)
    init_math_constants();
    init_math_functions();

    vector<string> testcases = {
        "1.5 + 2.5",
        "-4 * 5 / 4",
        "-(1 + 2) + 5 * 2",
        "((((((5)+4) / 3 * 2) + 9 ) + 1) + 1) * 0",
        "-(-(111.00))",
        "123 - 123.000 * 0",
        "5 % 4.3",
        "5 // 4",
        "5 // 4 + 1",
        "5 // 4 % 4",
        "2 ^ .5",
        "-1 ^ 3",
        "5 ^ 1 * 2",
        "1.3*1.3*.6",
        "1 ^ 1 ^ 1 ^ 1 ^ 2",
        "1 ^ 5 / 3",
        "2 ^ 2 * 3 ^ 2 + 5",
        "-5 * -4 - 6 / -6",
        "-(-4 / -5 * -1 * -(1/ -5) - 1) - 0",
        "0 / 0",
        "1e6",
        "11 / --(3)",
        "(2.0e2) * .5",
        "1E0",
        "5E7 / 10 ^ 7",
        "5 == 5",
        "3 != 5 / 4 * 2 + 2",
        "12 > 1 + 10",
        "var + 4",
        "hello / 8",
        "my_var = 3 * 3",
        "my_var = 4^5 == 6",
        "fun()",
        "fun() + call()",
        "result = fun() + call() / 2",
        "foo(z)",
        "foo(a, b, c)",
        "foo(a ^ 4, b = c == 4 != 6 % 1.44e99, 32432432, k)",
        "0xabbABFFE032185843B",
        "0b1010010101011010101010101",
        "0B0000111 - 7",
        "0xF + 0b1 / 0X0F * 0b01 ^ 0b0 / 0x0F",
        "y(x) = x^2 + 2",
        "a(b, c, d, a) = 2 * a / 5 * 7 ^ 8 ^ b / c * d == 9 + (my_var = 5)",
        "a(b) = c(d)",


        // INVALID inputs:
        "(",
        "(1 / 2",
        "1 / 2)",
        "(1 / 2).",
        "(1.2 / .5)2",
        "(1.2 / .5)+",
        "(1.2 / .5)-",
        "-",
        "+",
        "2 % * 2",
        "1-2-3*3.2.1",
        "2..3",
        "123*1++",
        "11e5"
        "1 = 1",
        "func() = 2",
        "",
        "()",
        "5()",
        "123abc()",
        "123abc = 3",
        "foo(a, b,)",
        "foo(,)",
        "foo(2 3)",
        "0b00010102",
        "0bx1",
        "0xg",
        "0x",
        "0b",

        // Variable / Function test inputs:
        " a = 1",
        "a",
        "a + b",
        "b = a = 2",
        "a",
        "b",
        "a / b",
        "c(a) = a + b",
        "c(0)",
        "b = 0",
        "c(0)",
        "b = -2",
        "c(2)",
        "f(x) = 2^x",
        "f(0)",
        "f(1)",
        "f(2)",
        "f(3)",
        "f(4)",
        "f()",
        "f(1, 2)",
        "f(1, 2, 3)",
        "g() = 5",
        "g()",
        "h(-1) = 0",
        "PI",
        "PI * 0",
        "PI * 2",
        "E",
        "E = 0",
        "E",
        "print_tree(1 + 2 + 3, E ^ 5 / (f = 666), f(-454) = 3)"


    };

    for(string test_expr : testcases) {
        cout << "result for " << test_expr << " : " << endl;
        vector<unique_ptr<Token>> token_vec = tokenize(test_expr);

        int ptr;
        unique_ptr<TreeNode> tree = parseS(token_vec);
        if(tree != nullptr) cout << tree->to_string() << endl;

        if(tree == nullptr) cout << "tree is nullptr." << endl;
        else cout << tree->eval() << endl;
    }

    return 0;
}
