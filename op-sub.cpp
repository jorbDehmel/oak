/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "op-sub.hpp"

void doSub(vector<string> &From, const int &i, const string &SubName, const bool &IsConst)
{
    // Everything before, after, and what to replace all this with
    vector<string> prevGroup, postGroup, toAdd;

    // Construct groups
    for (int j = 0; j < i; j++)
    {
        prevGroup.push_back(From[j]);
    }
    for (int j = i + 1; j < From.size(); j++)
    {
        postGroup.push_back(From[j]);
    }

    // Do operator substitutions on both sides
    operatorSub(prevGroup);
    operatorSub(postGroup);

    // Reconstruct
    toAdd.push_back(SubName);
    toAdd.push_back("(");
    if (!IsConst)
    {
        toAdd.push_back("@");
    }

    for (auto s : prevGroup)
    {
        toAdd.push_back(s);
    }

    toAdd.push_back(",");

    for (auto s : postGroup)
    {
        toAdd.push_back(s);
    }

    toAdd.push_back(")");

    From = toAdd;
    return;
}

void parenSub(vector<string> &From)
{
    if (From.size() < 3)
    {
        return;
    }

    // Pre-level A: Parenthesis and commas
    for (int i = 0; i < From.size(); i++)
    {
        string cur = From[i];
        if (cur == "(")
        {
            int startI = i, endI = i + 1;
            vector<string> contents, finalContents;

            int count = 1;
            while (count != 0 && endI < From.size())
            {
                if (From[endI] == "(")
                {
                    count++;
                }
                else if (From[endI] == ")")
                {
                    count--;
                    if (count == 0)
                    {
                        break;
                    }
                }
                else if ((From[endI] == "," || From[endI] == ";") && count == 1)
                {
                    parenSub(contents);
                    operatorSub(contents);

                    for (auto s : contents)
                    {
                        finalContents.push_back(s);
                    }
                    contents.clear();
                }

                contents.push_back(From[endI]);

                endI++;
            }

            parenSub(contents);
            operatorSub(contents);

            for (auto s : contents)
            {
                finalContents.push_back(s);
            }
            contents.clear();

            // Force it to skip over arguments in function definitions
            if (endI + 1 < From.size() && From[endI + 1] == "->")
            {
                continue;
            }

            // Replace contents with good ones

            // Erase old stuff
            for (int j = startI; j <= endI; j++)
            {
                From.erase(From.begin() + startI);
            }

            // Insert new stuff
            finalContents.insert(finalContents.begin(), "(");
            finalContents.push_back(")");

            for (int j = finalContents.size() - 1; j >= 0; j--)
            {
                From.insert(From.begin() + startI, finalContents[j]);
            }

            i = startI + finalContents.size();
        }
        else if (From[i] == "=" || From[i] == "+=" || From[i] == "-=" || From[i] == "*=" ||
                 From[i] == "/=" || From[i] == "%=" || From[i] == "|=" || From[i] == "&=" ||
                 From[i] == "++" || From[i] == "--")
        {
            int begin = i, end = i, count = 0;

            // Iterate back until beginning of expression
            while (begin > 0)
            {
                begin--;

                if (From[begin] == "(")
                {
                    count++;
                }
                else if (From[begin] == ")")
                {
                    count--;
                }

                if (count == 0)
                {
                    if (From[begin] == ";" || From[begin] == "}" || From[begin] == "{")
                    {
                        begin++;
                        break;
                    }
                }
            }

            // Iterate forward until end of expression
            while (end + 1 < From.size())
            {
                end++;

                if (From[end] == "(")
                {
                    count++;
                }
                else if (From[end] == ")")
                {
                    count--;
                }

                if (count == 0)
                {
                    if (From[end] == ";" || From[end] == "}" || From[end] == "{")
                    {
                        end--;
                        break;
                    }
                }
            }

            vector<string> contents, finalContents;

            for (int j = begin; j <= end; j++)
            {
                contents.push_back(From[j]);
            }

            // Stolen from above
            operatorSub(contents);

            for (auto s : contents)
            {
                finalContents.push_back(s);
            }
            contents.clear();

            // Replace contents with good ones

            // Erase old stuff
            for (int j = begin; j <= end; j++)
            {
                From.erase(From.begin() + begin);
            }

            // Insert new stuff
            finalContents.insert(finalContents.begin(), "(");
            finalContents.push_back(")");

            for (int j = finalContents.size() - 1; j >= 0; j--)
            {
                From.insert(From.begin() + begin, finalContents[j]);
            }

            i = begin + finalContents.size();
        }
    }

    return;
}

void operatorSub(vector<string> &From)
{
    // Level 1: Assignment
    for (int i = 0; i < From.size(); i++)
    {
        string cur = From[i];

        if (cur == "=")
        {
            doSub(From, i, "Copy", false);
            break;
        }
        else if (cur == "+=")
        {
            doSub(From, i, "AddEq", false);
            break;
        }
        else if (cur == "-=")
        {
            doSub(From, i, "SubEq", false);
            break;
        }
        else if (cur == "*=")
        {
            doSub(From, i, "MultEq", false);
            break;
        }
        else if (cur == "/=")
        {
            doSub(From, i, "DivEq", false);
            break;
        }
        else if (cur == "%=")
        {
            doSub(From, i, "ModEq", false);
            break;
        }
        else if (cur == "++")
        {
            throw_assert(false);
            break;
        }
        else if (cur == "--")
        {
            throw_assert(false);
            break;
        }
        else if (cur == "&=")
        {
            doSub(From, i, "AndEq", false);
            break;
        }
        else if (cur == "|=")
        {
            doSub(From, i, "OrEq", false);
            break;
        }
    }

    // Level 2: Booleans
    for (int i = 0; i < From.size(); i++)
    {
        string cur = From[i];

        if (cur == "!")
        {
            throw_assert(false);
        }
        else if (cur == "&&")
        {
            doSub(From, i, "Andd", true);
            break;
        }
        else if (cur == "||")
        {
            doSub(From, i, "Orr", true);
            break;
        }
    }

    // Level 3: Comparisons
    for (int i = 0; i < From.size(); i++)
    {
        string cur = From[i];

        if (cur == "<")
        {
            doSub(From, i, "Less", true);
            break;
        }
        else if (cur == ">")
        {
            doSub(From, i, "Great", true);
            break;
        }
        else if (cur == "<=")
        {
            doSub(From, i, "Leq", true);
            break;
        }
        else if (cur == ">=")
        {
            doSub(From, i, "Greq", true);
            break;
        }
        else if (cur == "==")
        {
            doSub(From, i, "Eq", true);
            break;
        }
        else if (cur == "!=")
        {
            doSub(From, i, "Neq", true);
            break;
        }
    }

    // Level 4: Multiplication, division and modulo
    for (int i = 0; i < From.size(); i++)
    {
        string cur = From[i];

        if (cur == "*")
        {
            doSub(From, i, "Mult", true);
            break;
        }
        else if (cur == "/")
        {
            doSub(From, i, "Div", true);
            break;
        }
        else if (cur == "%")
        {
            doSub(From, i, "Mod", true);
            break;
        }
    }

    // Level 5: Addition and subtraction
    for (int i = 0; i < From.size(); i++)
    {
        string cur = From[i];
        if (cur == "+")
        {
            doSub(From, i, "Add", true);
            break;
        }
        else if (cur == "-")
        {
            doSub(From, i, "Sub", true);
            break;
        }
    }

    // Level 6: Bitwise
    for (int i = 0; i < From.size(); i++)
    {
        string cur = From[i];

        if (cur == "<<")
        {
            doSub(From, i, "Lbs", true);
            break;
        }
        else if (cur == ">>")
        {
            doSub(From, i, "Rbs", true);
            break;
        }
        else if (cur == "&")
        {
            doSub(From, i, "And", true);
            break;
        }
        else if (cur == "|")
        {
            doSub(From, i, "Or", true);
            break;
        }
    }

    return;
}
