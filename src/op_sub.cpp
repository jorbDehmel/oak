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

// Moves pre and post to include the operands to a binary
// operator
// Assumes that pre = i - 1, post = i + 1, i = index of bin op
void getOperands(std::vector<Token> &from, int &pre, int &post, const bool &useLine = false)
{
    /*
    Example cases:

    i = (1 * 2) + (3 / 4);
    c = b() + 4;
    f = a.b + c.d
    */

    int count;

    // int begin = pre;

    // Decrement pre to point to the beginning of the left
    // operand
    count = 0;
    do
    {
        if (from[pre] == "(")
        {
            count++;

            if (count == 0 && pre != 0 && operators.count(from[pre - 1]) == 0)
            {
                if (pre != 0 && operators.count(from[pre - 1]) == 0)
                {
                    // Is function call
                    pre--;
                }
                else if (from[pre - 1] == ">")
                {
                    // Possible templated function call

                    // Scan backwards: If "<" occurs before ";", ")", then templating.
                    // Else, not templating.

                    bool isTemplating = false;
                    int i = pre - 2;
                    while (i >= 0)
                    {
                        if (from[i] == ")" || from[i] == ";")
                        {
                            isTemplating = false;
                            break;
                        }
                        else if (from[i] == "<")
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
        else if (from[pre] == ")")
        {
            count--;
        }

        pre--;
    } while (pre > 0 && count != 0);
    pre++;

    // Handle member access, refs and dereferences
    while (pre >= 2 && from[pre - 1] == ".")
    {
        pre -= 2;
    }
    while (pre > 0 && (from[pre - 1] == "^" || from[pre - 1] == "@"))
    {
        pre--;
    }

    // Increment post to point to the end of the right operand
    // If useLine, increment until a semicolon.
    if (useLine)
    {
        while (post < from.size() && from[post] != ";")
        {
            post++;
        }
    }
    else
    {
        count = 0;
        do
        {
            if (from[post] == "(")
            {
                count++;
            }
            else if (from[post] == ")")
            {
                count--;
            }

            while (post < from.size() && (from[post] == "^" || from[post] == "@"))
            {
                post++;
            }

            if (count == 0)
            {
                // Function call
                if (operators.count(from[post]) == 0 && from[post + 1] == "(")
                {
                    post++;
                    count = 1;
                }
            }

            post++;
        } while (post < from.size() && count != 0);

        // Member access, references and dereferences
        while (post + 2 < from.size() && from[post] == ".")
        {
            post += 2;
        }
    }

    // std::cout << "Operands:\n";
    // for (int i = pre; i < post; i++)
    // {
    //     if (i == begin)
    //     {
    //         std::cout << tags::green_bold;
    //     }

    //     std::cout << from[i].text << ' ';

    //     if (i == begin)
    //     {
    //         std::cout << tags::reset;
    //     }
    // }
    // std::cout << '\n';

    return;
}

// Substitute a single operation as identified
void doSub(std::vector<Token> &from, int &pos, const std::string &name)
{
    int pre = pos - 1, post = pos + 1;
    bool fullLine = false;
    std::vector<std::string> toAdd;

    if (name == "Copy" || (name != "Eq" && name.find("Eq") != std::string::npos))
    {
        fullLine = true;
    }

    // Identify operands
    getOperands(from, pre, post, fullLine);

    // Reconstruct into valid output
    toAdd.reserve(post - pre + 2);

    int start = pre, end = pos;
    while (from[start] == "(" && from[end - 1] == ")")
    {
        start++;
        end--;
    }

    toAdd = {name, "("};
    for (int i = start; i < end; i++)
    {
        toAdd.push_back(from[i]);
    }

    toAdd.push_back(",");

    start = pos + 1;
    end = post;
    while (from[start] == "(" && from[end - 1] == ")")
    {
        start++;
        end--;
    }

    for (int i = start; i < end; i++)
    {
        toAdd.push_back(from[i]);
    }
    toAdd.push_back(")");

    // Erase old (all items pre <= i < post)
    Token templ = from[pre];
    for (int i = pre; i < post; i++)
    {
        from.erase(from.begin() + pre);
    }

    // Insert new
    for (int j = toAdd.size() - 1; j >= 0; j--)
    {
        templ.text = toAdd[j];
        from.insert(from.begin() + pre, templ);
    }

    pos--;
    return;
}

void operatorSub(std::vector<Token> &From)
{
    // Level 2: Multiplication, division and modulo
    for (int i = 0; i < From.size(); i++)
    {
        std::string cur = From[i];

        if (cur == "*")
        {
            doSub(From, i, "Mult");
        }
        else if (cur == "/")
        {
            doSub(From, i, "Div");
        }
        else if (cur == "%")
        {
            doSub(From, i, "Mod");
        }
    }

    // Level 3: Addition and subtraction
    for (int i = 0; i < From.size(); i++)
    {
        std::string cur = From[i];
        if (cur == "+")
        {
            doSub(From, i, "Add");
        }
        else if (cur == "-")
        {
            doSub(From, i, "Sub");
        }
    }

    // Level 4: Bitwise
    for (int i = 0; i < From.size(); i++)
    {
        std::string cur = From[i];

        if (cur == "<<")
        {
            doSub(From, i, "Lbs");
        }
        else if (cur == ">>")
        {
            doSub(From, i, "Rbs");
        }
        else if (cur == "&")
        {
            doSub(From, i, "And");
        }
        else if (cur == "|")
        {
            doSub(From, i, "Or");
        }
    }

    // Level 5: Comparisons
    for (int i = 0; i < From.size(); i++)
    {
        std::string cur = From[i];

        if (cur == "<")
        {
            /*
            If a ) or ; occurs before the next >, do sub.
            Otherwise, is templating; Advance i past all of this.
            */

            bool isTemplating = false;
            int j = i + 1;
            int depth = 1;

            // std::cout << "Scanning for templating:\n";
            for (; j < From.size(); j++)
            {
                if (From[j] == ";" || From[j] == ")")
                {
                    isTemplating = false;
                    break;
                }
                else if (From[j] == ">")
                {
                    depth--;

                    if (depth == 0)
                    {
                        isTemplating = true;
                        break;
                    }
                }
                else if (From[j] == "<")
                {
                    depth++;
                }

                // std::cout << From[j].text << ' ';
            }
            // std::cout << '\n';

            if (isTemplating)
            {
                i = j;
            }
            else
            {
                doSub(From, i, "Less");
            }
        }
        else if (cur == ">")
        {
            doSub(From, i, "Great");
        }

        else if (cur == "<=")
        {
            doSub(From, i, "Leq");
        }
        else if (cur == ">=")
        {
            doSub(From, i, "Greq");
        }
        else if (cur == "==")
        {
            doSub(From, i, "Eq");
        }
        else if (cur == "!=")
        {
            doSub(From, i, "Neq");
        }
    }

    // Level 6: Booleans
    for (int i = 0; i < From.size(); i++)
    {
        std::string cur = From[i];

        if (cur == "&&")
        {
            doSub(From, i, "Andd");
        }
        else if (cur == "||")
        {
            doSub(From, i, "Orr");
        }
    }

    // Level 1: Assignment
    for (int i = 0; i < From.size(); i++)
    {
        std::string cur = From[i];

        if (cur == "=")
        {
            doSub(From, i, "Copy");
        }
        else if (cur == "+=")
        {
            doSub(From, i, "AddEq");
        }
        else if (cur == "-=")
        {
            doSub(From, i, "SubEq");
        }
        else if (cur == "*=")
        {
            doSub(From, i, "MultEq");
        }
        else if (cur == "/=")
        {
            doSub(From, i, "DivEq");
        }
        else if (cur == "%=")
        {
            doSub(From, i, "ModEq");
        }
        else if (cur == "&=")
        {
            doSub(From, i, "AndEq");
        }
        else if (cur == "|=")
        {
            doSub(From, i, "OrEq");
        }
    }

    return;
}
