#include "parser.h"

/* ~ ~ ~ ~ ~ Gramatical Parsing ~ ~ ~ ~ ~ */

/*
 * Grammar outline:
 *
 * Terminal symbols:
 *
 * "Literals":
 * - Variables
 * - Numeric Literals
 *      - decimal (floating-point and scientific notation)
 *      - binary (integer) and hex (integer) literals) (leading "0b" and "0x", respectively)
 *
 * "Operations"
 * - "+" addition
 * - "-" subtraction
 * - "-" unary negation
 * - "*" multiplication
 * - "/" float division
 * - "//" int division
 * - "%" modulus (converts arguments to int)
 * - "^" exponentiation
 * - "(" ")" grouping, function calls
 * - "=" assignment
 * - "==" equality
 * - "!=" inequality
 * - "<" less than
 * - "<=" less than or equal to
 * - ">" greater than
 * - ">=" greater than or equal to
 *
 * Nonterminal Symbols:
 *
 * S means "Statement"
 * E means "Expression" (parsed before () and comparison operators)
 * T means "Term" (parsed before + and -)
 * F means "Factor" (parsed before * and /)
 * X means "Exponential" (parsed before ^)
 * FN means "Function Call"
 * ARGS means "Argument Tuple"
 *
 * Terminal Symbols:
 * VAR means "Variable name" (this also encompasses function names)
 * NUM means "Numeric literal"
 * "" refers to an empty string
 * $ refers to the end of input
 *
 * Grammar Rules:
 *
 * S -> E$
 *
 * E -> VAR = E
 *   -> FN = E
 *   -> T == E
 *   -> T != E
 *   -> T < E
 *   -> T <= E
 *   -> T > E
 *   -> T >= E
 *   -> T {(+|-) T}
 *
 * T -> F {(*|/|//|%|^) F}
 *
 * F -> X {^ X}
 *
 * X -> -X
 *   -> (E)
 *   -> NUM
 *   -> VAR
 *   -> FN
 *
 * FN -> VAR(ARGS)
 *
 * ARGS -> {E {, E {, ...}}}
 */

/* ~ ~ ~ ~ ~ Grammar Parsing Functions ~ ~ ~ ~ ~ */

/*
 * NOTE: it is critical that the i reference returns to its original position
 * when parsing a nonterminal fails (as errors in parsing is regular behavior,
 * and no tokens shold be skipped). To avoid this, i's value is copied into j
 * at the beginning of each parsing function, and is only copied back before a
 * successful return.
 */

/*
 * S -> E$
 */
unique_ptr<TreeNode> parseS(vector<unique_ptr<Token>>& tokens, int i = 0) {
    int j = i;
    if(j == tokens.size()) return nullptr; // reached end of tokens

    unique_ptr<TreeNode> tree = parseE(tokens, j);

    if(j != tokens.size()) return nullptr; // expression doesn't span all tokens

    i = j;
    return tree;
}

/*
 * E -> VAR = E
 *   -> FN = E
 *   -> T == E
 *   -> T != E
 *   -> T < E
 *   -> T <= E
 *   -> T > E
 *   -> T >= E
 *   -> T ^ E
 *   -> T {(+|-) T}
 */
unique_ptr<TreeNode> parseE(vector<unique_ptr<Token>>& tokens, int& i) {
    //cout << "parse e " << i << endl; // TODO remove
    int j = i;
    if(j == tokens.size()) return nullptr; // reached end of tokens

    // VAR = E
    if(tokens[j]->type == TokenType::VAR && j + 1 != tokens.size() && 
       tokens[j + 1]->type == TokenType::OP && tokens[j + 1]->val.s == "=") {
        string var_id = tokens[j]->val.s;
        j += 2; // jump over VAR and =
        unique_ptr<TreeNode> rhs = parseE(tokens, j);
        //cout << " got rhs " << endl; // TODO remove

        if(rhs != nullptr) {
            i = j;
            return unique_ptr<TreeNode>(new AssignmentNode(var_id, move(rhs)));
        } else j = i; // reset j to i, and try other rules
    }

    // -> FN = E
    unique_ptr<TreeNode> fn = parseFN(tokens, j);
    if(fn != nullptr && j != tokens.size() && tokens[j]->type == TokenType::OP &&
                                              tokens[j]->val.s == "=") {
        j++; // jump over =
        unique_ptr<TreeNode> e = parseE(tokens, j);

        if(e != nullptr){
            i = j;
            return unique_ptr<TreeNode>(new FunctionAssignmentNode(move(fn), move(e)));
        }
    }
    j = i; // reset j to i, and try other rules

    // -> T == E
    // -> T != E
    // -> T < E
    // -> T <= E
    // -> T > E
    // -> T >= E
    // -> T {(+|-) T}
    unique_ptr<TreeNode> lhs = parseT(tokens, j);
    if(lhs == nullptr) return nullptr; // failed to read term

    // T$ or T? (where ? isn't one of the above options) => skip loop
    // ==, !=, <, <=, >, >=, +, or -                     => loop
    while(j != tokens.size() && tokens[j]->type == TokenType::OP) {
        string op = tokens[j]->val.s;
        if(op == "==" || op == "!=" || op == "<" || op == "<=" ||
           op == ">" || op == ">=" || op == "+" || op == "-") j++;
        else break;

        unique_ptr<TreeNode> rhs = (op == "+" || op == "-") ? parseT(tokens, j) : parseE(tokens, j);
        if(rhs == nullptr) {
            i = j - 1; // token before op
            return lhs;
        }

        // continue loop for + and - to maintain order of operations
        if(op == "+") lhs = unique_ptr<TreeNode>(new AdditionNode(move(lhs), move(rhs)));
        else if(op == "-") lhs = unique_ptr<TreeNode>(new SubtractionNode(move(lhs), move(rhs)));
        else {
            i = j;
            if(op =="==") return unique_ptr<TreeNode>(new EqualityNode(move(lhs), move(rhs)));
            else if(op =="!=") return unique_ptr<TreeNode>(new InequalityNode(move(lhs), move(rhs)));
            else if(op =="<") return unique_ptr<TreeNode>(new LessThanNode(move(lhs), move(rhs)));
            else if(op ==">") return unique_ptr<TreeNode>(new GreaterThanNode(move(lhs), move(rhs)));
            else if(op =="<=") return unique_ptr<TreeNode>(new LessOrEqualNode(move(lhs), move(rhs)));
            else if(op ==">=") return unique_ptr<TreeNode>(new GreaterOrEqualNode(move(lhs), move(rhs)));
        }
    }

    i = j;
    return lhs;
}

/*
 * T -> F {(*|/|//|%) F}
 */
unique_ptr<TreeNode> parseT(vector<unique_ptr<Token>>& tokens, int& i) {
    //cout << "parse t " << i << endl; // TODO remove
    int j = i;
    if(j == tokens.size()) return nullptr; // reached end of tokens

    // -> F {(*|/|//|%) F}
    unique_ptr<TreeNode> lhs = parseF(tokens, j);
    if(lhs == nullptr) return nullptr;

    while(j != tokens.size() && tokens[j]->type == TokenType::OP) {
        string op = tokens[j]->val.s;
        if(op == "*" || op == "/" || op == "//" || op == "%") j++;
        else break;

        unique_ptr<TreeNode> rhs = parseF(tokens, j);
        if(rhs == nullptr) return nullptr;

        if(op == "*") lhs = unique_ptr<TreeNode>(new MultiplicationNode(move(lhs), move(rhs)));
        else if(op == "/") lhs = unique_ptr<TreeNode>(new DivisionNode(move(lhs), move(rhs)));
        else if(op == "//") lhs = unique_ptr<TreeNode>(new IntDivisionNode(move(lhs), move(rhs)));
        else if(op == "%") lhs = unique_ptr<TreeNode>(new ModulusNode(move(lhs), move(rhs)));
    }

    i = j;
    //cout << " returning t " << i << endl; // TODO remove
    return lhs;
}

/*
 * F -> X {^ X}
 */
unique_ptr<TreeNode> parseF(vector<unique_ptr<Token>>& tokens, int& i) {
    //cout << "parse f " << i << endl; // TODO remove
    int j = i;
    if(j == tokens.size()) return nullptr;

    unique_ptr<TreeNode> base = parseX(tokens, j);
    if(base == nullptr) return nullptr;

    while(j != tokens.size() && tokens[j]->type == TokenType::OP && tokens[j]->val.s == "^") {
        j++;
        unique_ptr<TreeNode> exp = parseX(tokens, j);
        if(exp == nullptr) {
            j--; // set j back to '^' token
            break;
        }

        base = unique_ptr<TreeNode>(new PowerNode(move(base), move(exp)));
    }

    i = j;
    //cout << " returning f " << endl; // TODO remove
    return base;
}

/*
 * X -> -X
 *   -> (E)
 *   -> NUM
 *   -> FN
 *   -> VAR
 */
unique_ptr<TreeNode> parseX(vector<unique_ptr<Token>>& tokens, int& i) {
    //cout << "parse x " << i << endl; // TODO remove
    int j = i;
    if(j == tokens.size()) return nullptr; // reached end of tokens

    // X -> -X
    if(tokens[j]->type == TokenType::OP && tokens[j]->val.s == "-") {
        j++;
        unique_ptr<TreeNode> x = parseX(tokens, j);
        if(x == nullptr) return nullptr;
        i = j;
        return unique_ptr<TreeNode>(new NegationNode(move(x)));
    }


    // -> (E)
    if(tokens[j]->type == TokenType::OP && tokens[j]->val.s == "(") {
        unique_ptr<TreeNode> e = parseE(tokens, ++j);
        if(j == tokens.size() || tokens[j]->type != TokenType::OP || tokens[j]->val.s != ")")
            return nullptr;
        i = j + 1;
        return e;
    }

    // -> NUM
    if(tokens[j]->type == TokenType::NUM) {
        i = j + 1;
        return unique_ptr<TreeNode>(new NumberNode(tokens[j]->val.d));
    }

    // -> FN
    unique_ptr<TreeNode> fn = parseFN(tokens, j);
    if(fn != nullptr) {
        i = j;
        return fn;
    }

    // -> VAR
    if(tokens[j]->type == TokenType::VAR) {
        i = j + 1;
        return unique_ptr<TreeNode>(new VariableNode(tokens[j]->val.s));
    }

    return nullptr;
}

/*
 * FN -> VAR(ARGS)
 */
unique_ptr<TreeNode> parseFN(vector<unique_ptr<Token>>& tokens, int& i) {
    int j = i;
    if(j == tokens.size()) return nullptr; // reached end of tokens

    if(tokens[j]->type != TokenType::VAR) return nullptr;
    string var_id = tokens[j++]->val.s;

    if(j == tokens.size() || tokens[j]->type != TokenType::OP || tokens[j]->val.s != "(") return nullptr;
    vector<unique_ptr<TreeNode>> args = parseARGS(tokens, ++j);
    if(j == tokens.size() || tokens[j]->type != TokenType::OP || tokens[j]->val.s != ")") return nullptr;


    i = j + 1; // i = char after ')'
    return unique_ptr<TreeNode>(new FunctionCallNode(var_id, move(args)));
}

/*
 * ARGS -> {E {, E {, ...}}}
 */
vector<unique_ptr<TreeNode>> parseARGS(vector<unique_ptr<Token>>& tokens, int& i) {
    int j = i;
    vector<unique_ptr<TreeNode>> args;

    unique_ptr<TreeNode> current;

    while((current = parseE(tokens, j)) != nullptr) {
        args.push_back(move(current));
        i = j; // i is only updated after adding valid expresion so trailing commas aren't allowed
        if(j == tokens.size() || tokens[j]->type != TokenType::OP || tokens[j]->val.s != ",") break;
        j++; // skip over comma, and try to parse another.
    }

    return args;
}

int main() { // TODO remove (test method)
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
        "0b"
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
