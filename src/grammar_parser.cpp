#include "parser.h"

/* ~ ~ ~ ~ ~ Gramatical Parsing ~ ~ ~ ~ ~ */

/*
 * Grammar outline:
 *
 * Terminal symbols:
 *
 * "Literals":
 * - Variables
 * - Numeric Literals (decimal (floating-point and scientific notation) 
 *      - binary (integer) and hex (integer) literals) (TODO add these?)
 *
 * "Operations"
 * - "+" addition
 * - "-" subtraction
 * - "-" unary negation
 * - "*" multiplication
 * - "/" float division
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
 * ARGS means "Argument tuple"
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
 *   -> T == E
 *   -> T != E
 *   -> T < E
 *   -> T <= E
 *   -> T > E
 *   -> T >= E
 *   -> T {(+|-) T}
 *
 * T -> -F
 *   -> F {(*|/|//|%|^) F}
 *
 * F -> (E)
 *   -> NUM
 *   -> VAR
 *   -> VAR(ARGS)
 *
 * ARGS -> E, ARGS
 *      -> E
 *      -> ""
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
    int j = i;
    if(j == tokens.size()) return nullptr; // reached end of tokens

    // VAR = E
    if(tokens[j]->type == TokenType::VAR && j + 1 != tokens.size() && 
       tokens[j + 1]->type == TokenType::OP && tokens[j + 1]->val.s == "=") {
        string var_id = tokens[j]->val.s;
        j += 2; // jump over VAR and =
        unique_ptr<TreeNode> rhs = parseE(tokens, j);

        if(rhs != nullptr) {
            i = j;
            return unique_ptr<TreeNode>(new AssignmentNode(tokens[i]->val.s, move(rhs)));
        } else j = i;
    }

    // -> T == E
    // -> T != E
    // -> T < E
    // -> T <= E
    // -> T > E
    // -> T >= E
    // -> T ^ E
    // -> T {(+|-) T}
    unique_ptr<TreeNode> lhs = parseT(tokens, j);
    if(lhs == nullptr) return nullptr; // failed to read term

    // T$ or T? (where ? isn't one of the above options) => skip loop
    // ==, !=, <, <=, >, >=, +, or -                     => loop
    while(j != tokens.size() && tokens[j]->type == TokenType::OP) {
        string op = tokens[j++]->val.s;
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
 * T -> -F
 *   -> F {(*|/|//|%|^) F}
 */
unique_ptr<TreeNode> parseT(vector<unique_ptr<Token>>& tokens, int& i) {
    int j = i;
    if(j == tokens.size()) return nullptr; // reached end of tokens

    // T -> -F
    if(tokens[j]->type == TokenType::OP && tokens[j]->val.s == "-") {
        j++;
        unique_ptr<TreeNode> f = parseF(tokens, j);
        if(f == nullptr) return nullptr;
        i = j;
        return unique_ptr<TreeNode>(new NegationNode(move(f)));
    }

    // -> F {(*|/|//|%|^) F}
    unique_ptr<TreeNode> lhs = parseF(tokens, j);
    if(lhs == nullptr) return nullptr;

    while(j != tokens.size() && tokens[j]->type == TokenType::OP) {
        string op = tokens[j]->val.s;
        if(op == "*" || op == "/" || op == "//" || op == "%" || op == "^") j++;
        else break;

        unique_ptr<TreeNode> rhs = parseF(tokens, j);
        if(rhs == nullptr) return nullptr;

        if(op == "*") lhs = unique_ptr<TreeNode>(new MultiplicationNode(move(lhs), move(rhs)));
        else if(op == "/") lhs = unique_ptr<TreeNode>(new DivisionNode(move(lhs), move(rhs)));
        else if(op == "//") lhs = unique_ptr<TreeNode>(new IntDivisionNode(move(lhs), move(rhs)));
        else if(op == "%") lhs = unique_ptr<TreeNode>(new ModulusNode(move(lhs), move(rhs)));
        else if(op == "^") lhs = unique_ptr<TreeNode>(new PowerNode(move(lhs), move(rhs)));
    }

    i = j;
    return lhs;
}


/*
 * F -> (E)
 *   -> NUM
 *   -> VAR
 *   -> VAR(ARGS)
 */
unique_ptr<TreeNode> parseF(vector<unique_ptr<Token>>& tokens, int& i) {
    int j = i;
    if(j == tokens.size()) return nullptr; // reached end of tokens

    // F -> (E)
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

    // -> VAR
    // -> VAR(ARGS)
    if(tokens[j]->type == TokenType::VAR) {
        string var_id = tokens[j++]->val.s;

        // -> VAR(ARGS)
        if(j != tokens.size() && tokens[j]->type == TokenType::OP && tokens[j]->val.s == "(") {
            vector<unique_ptr<TreeNode>> args = parseARGS(tokens, j);
            if(j == tokens.size() || tokens[j]->type != TokenType::OP || tokens[j]->val.s != ")") 
                j = i + 1; // point to token after VAR
            else {
                i = j + 1;
                return unique_ptr<TreeNode>(new FunctionCallNode(var_id, move(args)));
            }
        }

        // -> VAR
        i = j;
        return unique_ptr<TreeNode>(new VariableNode(var_id));
    }

    return nullptr;

}

/*
 * ARGS -> E, ARGS
 *      -> E
 *      -> ""
 */
vector<unique_ptr<TreeNode>> parseARGS(vector<unique_ptr<Token>>& tokens, int& i) {
    int j = i;
    vector<unique_ptr<TreeNode>> args;

    unique_ptr<TreeNode> current;

    while((current = parseE(tokens, j)) != nullptr) {
        args.push_back(move(current));
        i = j;
        if(j == tokens.size() || tokens[j]->type != TokenType::OP || tokens[j]->val.s != ",") break;
        j++; // skip over comma, and try to parse another.
    }

    return args;
}

int main() { // TODO remove (test method)
    vector<string> testcases = {
        "2 ^ .5",
        "-1 ^ 3",
        "5 ^ 1 * 2"
        /*
        "(
        "(1 / 2"
        */
    };

    for(string test_expr : testcases) {
        cout << "result for " << test_expr << " : " << endl;
        vector<unique_ptr<Token>> token_vec = tokenize(test_expr);

        int ptr;
        cout << token_vec.size() << endl;
        unique_ptr<TreeNode> tree = parseS(token_vec);

        if(tree == nullptr) cout << "tree is nullptr :(" << endl;
        else cout << tree->eval() << endl;
    }

    return 0;
}
