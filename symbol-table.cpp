/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "symbol-table.hpp"

multiSymbolTable table;

// Converts lexed symbols into a type
Type toType(const vector<string> &WhatIn)
{
    vector<string> What;
    for (string s : WhatIn)
    {
        if (s.size() < 2 || s.substr(0, 2) != "//")
        {
            What.push_back(s);
        }
    }

    if (What.size() == 0)
    {
        return Type(atomic, "NULL");
    }
    int i = 0;

    if (What[i] == "<")
    {
        while (What[i] != ">")
        {
            i++;
        }
        i++;
    }

    Type out;
    for (; i < What.size(); i++)
    {
        string cur = What[i];

        if (cur == "^" || cur == "@")
        {
            out.append(pointer);
        }
        else if (cur == "<")
        {
            throw_assert(out != nullType);

            // Append to back
            Type *cursor = &out;
            while (cursor->next != nullptr)
            {
                cursor = cursor->next;
            }

            cursor->name += "_";

            int count = 0;
            while (!(count == 1 && What[i] == ">"))
            {
                if (What[i] == "<")
                {
                    count++;
                    cursor->name += "_";
                }
                else if (What[i] == ">")
                {
                    count--;
                }
                else if (What[i] == ",")
                {
                    cursor->name += "_";
                }
                else
                {
                    cursor->name += What[i] + "_";
                }

                i++;
                if (i > What.size())
                {
                    break;
                }
            }
        }
        else if (cur == ",")
        {
            out.append(join);
        }
        else if (cur == "->")
        {
            out.append(maps);
        }
        else if (cur == ")" || cur == ":" || cur == ";")
        {
            ;
        }
        else if (cur == "(")
        {
            out.append(function);
        }
        else if (cur == "let")
        {
            i++;
        }
        else if (What.size() > i && What[i + 1] == ":")
        {
            out.append(var_name, cur);
        }
        else
        {
            out.append(atomic, cur);
        }
    }

    return out;
}

// Can throw errors (IE malformed definitions)
void addStruct(const vector<string> &FromIn)
{
    vector<string> From;

    // Clean input of any special symbols
    for (auto f : FromIn)
    {
        if (f.size() < 2 || f.substr(0, 2) != "//")
        {
            From.push_back(f);
        }
    }

    // Assert the expression can be properly-formed
    parse_assert(From.size() >= 4);

    // Get name and check against malformations
    int i = 0;

    parse_assert(From[i] == "let");
    i++;
    string name = From[i];
    i++;
    parse_assert(From[i] == ":");
    i++;
    parse_assert(From[i] == "struct");
    i++;

    if (From[i] == "{")
    {
        i++;
        for (; i < From.size() && From[i] != "}"; i++)
        {
            // name : type ,
            // name , name2 , name3 : type < string , hi > , name4 : type2 ,
            vector<string> names, lexedType;

            while (i + 1 < From.size() && From[i + 1] == ",")
            {
                names.push_back(From[i]);

                i += 2;
            }

            names.push_back(From[i]);

            parse_assert(i + 1 < From.size());
            parse_assert(From[i + 1] == ":");

            i += 2;

            // Get lexed type (can be multiple symbols due to templating)
            int templCount = 0;
            while (i < From.size() && !(templCount == 0 && From[i] == ","))
            {
                if (templCount == 0 && From[i] != "<")
                {
                    lexedType.push_back(From[i]);
                }
                else
                {
                    throw_assert(!lexedType.empty());

                    if (From[i] == "<")
                    {
                        lexedType.back().append("__");
                    }
                    else if (From[i] == "^" || From[i] == "@")
                    {
                        lexedType.back().append("ptr_");
                    }
                    else if (From[i] == ">")
                    {
                        ;
                    }
                    else
                    {
                        lexedType.back().append(From[i] + "_");
                    }
                }

                if (From[i] == "<")
                {
                    templCount++;
                }
                else if (From[i] == ">")
                {
                    templCount--;
                }

                i++;
            }

            Type toAdd = toType(lexedType);
            for (string varName : names)
            {
                structData[name].members[varName] = toAdd;
                structData[name].order.push_back(varName);
            }
        }
    }
    else if (From[4] != ";")
    {
        throw parse_error("Malformed struct definition; Expected ';' or '{'.");
    }

    return;
}
