/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "symbol-table.hpp"

multiTemplTable templTable;
multiSymbolTable table;

// Converts lexed symbols into a type
Type toType(const vector<string> &What, const set<string> &Local)
{
    if (What.size() == 0)
    {
        return Type(atomic, "NULL");
    }

    Type out;
    for (int i = 0; i < What.size(); i++)
    {
        string cur = What[i];

        if (cur == "^" || cur == "&")
        {
            out.append(pointer);
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
        else if (modifiers.count(cur) != 0)
        {
            out.append(modifier, cur);
        }
        else if (atomics.count(cur) != 0)
        {
            out.append(atomic, cur);
        }
        else if (What.size() > i && What[i + 1] == ":")
        {
            out.append(var_name, cur);
        }
        else if (Local.count(cur) != 0)
        {
            out.append(atomic, cur);
        }
    }

    return out;
}

// Can throw errors (IE malformed definitions)
void addStruct(const vector<string> &From)
{
    // let *name*: struct
    // {
    //      a: type,
    //      b: type,
    //      c: type,
    // }

    // let *name* : struct { a : type , b : type , c : type , }

    // Assert the expression can be properly-formed
    parse_assert(From.size() >= 4);

    // Get name and check against malformations
    int i = 0;

    parse_assert(From[i] == "let");
    i++;

    string name = From[i];
    i++;

    // Check for templating here
    if (From[i] == "<")
    {
        int count = 1;
        i++;

        while (count != 0 && i < From.size())
        {
            if (From[i] == "<")
            {
                count++;
            }
            else if (From[i] == ">")
            {
                count--;
            }
            else if (From[i] == ",")
            {
                ;
            }
            else
            {
                structData[name].templatedTypes.insert(From[i]);
            }

            i++;
        }
    }

    parse_assert(From[i] == ":");
    i++;

    vector<string> lexedType;
    while (From[i] != "{" && From[i] != ";")
    {
        lexedType.push_back(From[i]);
        i++;
    }
    structData[name].type = toType(lexedType, structData[name].templatedTypes);

    atomics.insert(name);

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
                lexedType.push_back(From[i]);

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

            Type toAdd = toType(lexedType, structData[name].templatedTypes);
            for (string varName : names)
            {
                structData[name].members[varName] = toAdd;
            }
        }
    }
    else if (From[4] != ";")
    {
        throw parse_error("Malformed struct definition; Expected ';' or '{'.");
    }

    return;
}

Type toType(const string &What)
{
    vector<string> lexed = lex(What);

    set<string> local;
    if (!lexed.empty() && lexed.front() == "let")
    {
        lexed.erase(lexed.begin()); // let
        lexed.erase(lexed.begin()); // name
    }

    return toType(lexed, local);
}

void addStruct(const string &From)
{
    vector<string> lexed = lex(From);
    addStruct(lexed);

    return;
}

void addSymb(const string &Name, const vector<string> &From, const sequence &Seq)
{
    // Scan for templating
    vector<string> toParse;
    vector<string> generics;

    int count = 0;
    for (int i = 0; i < From.size() && From[i] != ";" && From[i] != "{"; i++)
    {
        if (From[i] == "<")
        {
            count++;
        }
        else if (From[i] == ">")
        {
            count--;
        }

        if (count == 0)
        {
            toParse.push_back(From[i]);
        }
        else if (count == 1)
        {
            if (From[i] != "," && From[i] != "<" && From[i] != ">")
            {
                generics.push_back(From[i]);
            }
        }
    }

    if (generics.size() == 0)
    {
        table[Name].push_back(__multiTableSymbol{Seq, toType(toParse)});
    }
    else
    {
        templTable[Name].push_back(__template_info{generics, Seq, toType(toParse)});
    }

    return;
}
