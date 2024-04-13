/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author

Does binary operator substitutions on an Oak input token stream.
Only binary operators are supported in Oak by default: Unaries
and the ternary must be added via rules, if at all.
*/

#include "oakc_fns.hpp"
#include "options.hpp"

// Moves pre and post to include the operands to a binary
// operator
// Assumes that pre = i - 1, post = i + 1, i = index of bin op
void getOperands(std::list<Token> &from, std::list<Token>::iterator &pre, std::list<Token>::iterator &post,
                 const bool &useLine = false)
{
    int count = 0;

    // Decrement pre to point to the beginning of the left
    // operand
    do
    {
        if (*pre == "(")
        {
            count++;

            if (count == 0 && OPERATORS.count(itGet(from, pre, -1)) == 0)
            {
                if (OPERATORS.count(itGet(from, pre, -1)) == 0)
                {
                    // Is function call
                    pre--;
                }
                else if (itCmp(from, pre, -1, ">"))
                {
                    // Possible templated function call

                    // Scan backwards: If "<" occurs before ";", ")", then templating.
                    // Else, not templating.

                    bool isTemplating = false;
                    auto i = pre;
                    i--;

                    if (i != from.begin())
                    {
                        i--;
                    }

                    while (i != from.begin())
                    {
                        if (*i == ")" || *i == ";")
                        {
                            isTemplating = false;
                            break;
                        }
                        else if (*i == "<")
                        {
                            isTemplating = true;
                            break;
                        }

                        i--;
                    }

                    if (isTemplating)
                    {
                        pre = i;
                    }
                }
            }
        }
        else if (*pre == ")")
        {
            count--;
        }

        pre--;
    } while (pre != from.begin() && count != 0);
    pre++;

    // Handle member access, refs and dereferences
    while (itCmp(from, pre, -1, "."))
    {
        pre--;
        pre--;
    }
    while (itCmp(from, pre, -1, "^") || itCmp(from, pre, -1, "@"))
    {
        pre--;
    }

    // Increment post to point to the end of the right operand
    // If useLine, increment until a semicolon.
    if (useLine)
    {
        while (post != from.end() && *post != ";")
        {
            post++;
        }
    }
    else
    {
        count = 0;
        do
        {
            if (*post == "(")
            {
                count++;
            }
            else if (*post == ")")
            {
                count--;
            }

            while (post != from.end() && (*post == "^" || *post == "@"))
            {
                post++;
            }

            if (count == 0)
            {
                // Function call
                if (OPERATORS.count(*post) == 0 && itCmp(from, post, 1, "("))
                {
                    post++;
                    count = 1;
                }
            }

            post++;
        } while (post != from.end() && count != 0);

        // Member access, references and dereferences
        while (itIsInRange(from, post, 2) && *post == ".")
        {
            post++;
            post++;
        }
    }

    return;
}

// Moves post to include the operand to a binary
// operator. Oak only has prefix unaries.
// Assumes that pre = i - 1, post = i + 1, i = index of bin op
void getOperandUnary(std::list<Token> &from, const std::list<Token>::iterator &pre, std::list<Token>::iterator &post)
{
    int count = 0;

    // Increment post to point to the end of the right operand
    // If useLine, increment until a semicolon.
    count = 0;
    do
    {
        if (*post == "(")
        {
            count++;
        }
        else if (*post == ")")
        {
            count--;
        }

        while (post != from.end() && (*post == "^" || *post == "@"))
        {
            post++;
        }

        if (count == 0)
        {
            // Function call
            if (OPERATORS.count(*post) == 0 && itCmp(from, post, 1, "("))
            {
                post++;
                count = 1;
            }
        }

        post++;
    } while (post != from.end() && count != 0);

    // Member access, references and dereferences
    while (itIsInRange(from, post, 2) && *post == ".")
    {
        post++;
        post++;
    }

    return;
}

// Substitute a single binary operation as identified
void doSub(std::list<Token> &from, std::list<Token>::iterator &pos, const std::string &name)
{
    auto pre = pos, post = pos;
    pre--;
    post++;

    bool fullLine = false;
    std::list<std::string> toAdd;

    if (name == "Copy" || (name != "Eq" && name.find("Eq") != std::string::npos))
    {
        fullLine = true;
    }

    // Identify operands
    getOperands(from, pre, post, fullLine);

    // Reconstruct into valid output
    auto start = pre, end = pos;

    while (itCmp(from, start, 0, "(") && itCmp(from, end, -1, ")"))
    {
        start++;
        end--;
    }

    toAdd = {name, "("};
    for (auto i = start; i != end; i++)
    {
        toAdd.push_back(*i);
    }

    toAdd.push_back(",");

    start = pos;
    start++;

    end = post;
    while (itCmp(from, start, 0, "(") && itCmp(from, end, -1, ")"))
    {
        start++;
        end--;
    }

    for (auto i = start; i != end; i++)
    {
        toAdd.push_back(*i);
    }
    toAdd.push_back(")");

    // Erase old (all items pre <= i < post)
    Token templ = *pre; // Ensures line, file info are copied over
    while (pre != post)
    {
        pre = from.erase(pre);
    }

    // Insert new
    for (const auto &item : toAdd)
    {
        templ.text = item;
        pre = from.insert(pre, templ);
        pre++;
    }

    pos = pre;
    pos--;
}

// Substitute a single unary operation as identified
void doSubUnary(std::list<Token> &from, std::list<Token>::iterator &pos, const std::string &name)
{
    auto pre = pos, post = pos;
    pre--;
    post++;

    std::list<std::string> toAdd;

    // Identify operands
    getOperandUnary(from, pre, post);
    pre++;

    // Reconstruct into valid output
    auto start = pre, end = pos;

    while (itCmp(from, start, 0, "(") && itCmp(from, end, -1, ")"))
    {
        start++;
        end--;
    }

    toAdd = {name, "("};

    start = pos;
    start++;

    end = post;
    while (itCmp(from, start, 0, "(") && itCmp(from, end, -1, ")"))
    {
        start++;
        end--;
    }

    for (auto i = start; i != end; i++)
    {
        toAdd.push_back(*i);
    }
    toAdd.push_back(")");

    // Erase old (all items pre <= i < post)
    Token templ = *pre; // Ensures line, file info are copied over

    while (pre != post)
    {
        pre = from.erase(pre);
    }

    // Insert new
    for (const auto &item : toAdd)
    {
        templ.text = item;
        pre = from.insert(pre, templ);
        pre++;
    }

    pos = pre;
    pos--;
}

// Fixes method call notation.
// All iterators within the given range will be invalidated.
void fixMethod(std::list<Token> &from, std::list<Token>::iterator &beginObj, std::list<Token>::iterator &beginCall,
               Token &fnName)
{
    // beginCall points to "("

    /*
    a.b.c.d.e.f.g(
    ^            ^ fnName = "g"
    */

    // Erase beginCall, beginCall - 1
    // a.b.c.d.e.f
    beginCall--;
    beginCall = from.erase(beginCall);
    beginCall = from.erase(beginCall);

    // Put "," at end
    // g(a.b.c.d.e.f,
    if (*beginCall != ")")
    {
        *std::prev(beginCall) = Token(",");
    }
    else
    {
        from.erase(std::prev(beginCall));
    }

    // Insert fnName, "(" at beginning
    // g(a.b.c.d.e.f
    from.emplace(beginObj, fnName);
    from.emplace(beginObj, "(");

    // Return
    return;
}

// O(n)
// These are some common rule-like token stream manipulations
// which would be too hard (or perhaps impossible) to implement
// with the rule system.
void operatorSub(std::list<Token> &From)
{
    // Level -1: Method resolution
    // This one works via DFA
    {
        int state = 0;
        std::list<Token>::iterator startOfObj;
        Token methodName;

        for (auto it = From.begin(); it != From.end(); it++)
        {
            std::string cur = it->text;

            if (state == 0)
            {
                if (OPERATORS.count(cur) == 0)
                {
                    startOfObj = it;
                    state = 1;
                }
            }
            else if (state == 1)
            {
                if (cur == ".")
                {
                    state = 2;
                }
                else if (OPERATORS.count(cur) != 0)
                {
                    state = 0;
                }
            }
            else if (state == 2)
            {
                if (OPERATORS.count(cur) == 0)
                {
                    methodName = cur;
                    state = 3;
                }
                else
                {
                    state = 0;
                }
            }
            else if (state == 3)
            {
                if (cur == ".")
                {
                    state = 2;
                }
                else if (cur == "(")
                {
                    // Match; Do replacement here
                    auto startOfCall = it;
                    it++; // So that it does not get invalidated
                    fixMethod(From, startOfObj, startOfCall, methodName);
                    it--; // So it is in the right spot
                    state = 0;
                }
                else if (OPERATORS.count(cur) != 0)
                {
                    state = 0;
                }
            }
        }
    }

    // Level 0: Prefix unaries
    for (auto it = From.begin(); it != From.end(); it++)
    {
        std::string cur = it->text;

        if (cur == "++")
        {
            doSubUnary(From, it, "Incr");
        }
        else if (cur == "--")
        {
            doSubUnary(From, it, "Decr");
        }
        else if (cur == "!")
        {
            doSubUnary(From, it, "Not");
        }
    }

    // Level 1: Multiplication, division and modulo
    for (auto it = From.begin(); it != From.end(); it++)
    {
        std::string cur = it->text;

        if (cur == "*")
        {
            doSub(From, it, "Mult");
        }
        else if (cur == "/")
        {
            doSub(From, it, "Div");
        }
        else if (cur == "%")
        {
            doSub(From, it, "Mod");
        }
    }

    // Level 2: Addition and subtraction
    for (auto it = From.begin(); it != From.end(); it++)
    {
        std::string cur = *it;

        if (cur == "+")
        {
            doSub(From, it, "Add");
        }
        else if (cur == "-")
        {
            doSub(From, it, "Sub");
        }
    }

    // Level 3: Bitwise
    for (auto it = From.begin(); it != From.end(); it++)
    {
        std::string cur = *it;

        if (cur == "<<")
        {
            doSub(From, it, "Lbs");
        }
        else if (cur == ">>")
        {
            doSub(From, it, "Rbs");
        }
        else if (cur == "&")
        {
            doSub(From, it, "And");
        }
        else if (cur == "|")
        {
            doSub(From, it, "Or");
        }
    }

    // Level 4: Comparisons
    for (auto it = From.begin(); it != From.end(); it++)
    {
        std::string cur = *it;

        if (cur == "<")
        {
            /*
            If a ) or ; occurs before the next >, do sub.
            Otherwise, is templating; Advance i past all of this.
            */

            bool isTemplating = false;
            auto j = it;
            j++;
            int depth = 1;
            int parenDepth = 0;

            for (; j != From.end(); j++)
            {
                if (*j == ";")
                {
                    isTemplating = false;
                    break;
                }
                else if (*j == "(")
                {
                    parenDepth++;
                }
                else if (*j == ")")
                {
                    if (parenDepth == 0)
                    {
                        isTemplating = false;
                        break;
                    }

                    parenDepth--;
                }
                else if (*j == ">")
                {
                    depth--;

                    if (depth == 0)
                    {
                        isTemplating = true;
                        break;
                    }
                }
                else if (*j == "<")
                {
                    depth++;
                }
            }

            if (isTemplating)
            {
                it = j;
            }
            else
            {
                doSub(From, it, "Less");
            }
        }
        else if (cur == ">")
        {
            doSub(From, it, "Great");
        }

        else if (cur == "<=")
        {
            doSub(From, it, "Leq");
        }
        else if (cur == ">=")
        {
            doSub(From, it, "Greq");
        }
        else if (cur == "==")
        {
            doSub(From, it, "Eq");
        }
        else if (cur == "!=")
        {
            doSub(From, it, "Neq");
        }
    }

    // Level 5: Booleans
    for (auto it = From.begin(); it != From.end(); it++)
    {
        std::string cur = *it;

        if (cur == "&&")
        {
            doSub(From, it, "Andd");
        }
        else if (cur == "||")
        {
            doSub(From, it, "Orr");
        }
    }

    // Level 6: Assignment
    for (auto it = From.begin(); it != From.end(); it++)
    {
        std::string cur = *it;

        if (cur == "=")
        {
            doSub(From, it, "Copy");
        }
        else if (cur == "+=")
        {
            doSub(From, it, "AddEq");
        }
        else if (cur == "-=")
        {
            doSub(From, it, "SubEq");
        }
        else if (cur == "*=")
        {
            doSub(From, it, "MultEq");
        }
        else if (cur == "/=")
        {
            doSub(From, it, "DivEq");
        }
        else if (cur == "%=")
        {
            doSub(From, it, "ModEq");
        }
        else if (cur == "&=")
        {
            doSub(From, it, "AndEq");
        }
        else if (cur == "|=")
        {
            doSub(From, it, "OrEq");
        }
    }
}
