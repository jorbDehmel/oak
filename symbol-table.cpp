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
    if (WhatIn.size() == 0)
    {
        return Type(atomic, "NULL");
    }

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
        while (i < What.size() && What[i] != ">")
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

            // Collect generics
            vector<string> generics, curGen;
            int count = 0;
            do
            {
                if (What[i] == "<")
                {
                    count++;

                    if (count > 1)
                    {
                        curGen.push_back(What[i]);
                    }
                }
                else if (What[i] == ">")
                {
                    count--;

                    if (count > 0)
                    {
                        curGen.push_back(What[i]);
                    }

                    generics.push_back(mangle(curGen));
                    curGen.clear();
                }
                else if (What[i] == "," && count == 1)
                {
                    generics.push_back(mangle(curGen));
                    curGen.clear();
                }
                else
                {
                    curGen.push_back(What[i]);
                }

                i++;
            } while (i < What.size() && count != 0);
            i--;

            cursor->name = instantiateGeneric(cursor->name, generics);
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

    // Scrape generics here (and mangle)
    vector<string> generics;

    i++;
    while (i < From.size() && From[i] != ":" && From[i] != "{")
    {
        generics.push_back(From[i]);
        i++;
    }

    if (generics.size() != 0 && generics.front() == "<")
    {
        generics.erase(generics.begin());
    }

    if (generics.size() != 0 && generics.back() == ">")
    {
        generics.pop_back();
    }

    if (generics.size() != 0)
    {
        name = mangleStruct(name, generics);
    }

    if (structData.count(name) != 0)
    {
        cout << tags::yellow_bold
             << "Warning! Redefinition of struct '" << name << "'.\n"
             << tags::reset;
    }

    // Ensures unit structs still get added
    structData[name];
    structOrder.push_back(name);

    // Auto-create unit New and Del
    Type t;
    t.append(function);
    t.append(var_name, "what");
    t.append(pointer);
    t.append(atomic, name);
    t.append(maps);
    t.append(atomic, "void");

    sequence s;
    s.info = code_scope;
    s.type = Type(atomic, "void");
    s.items.push_back(sequence{});
    s.items.back().info = atom;
    s.items.back().raw = "//AUTOGEN";

    // Ensure these keys exist
    table["New"];
    table["Del"];

    table["New"].push_back(__multiTableSymbol{s, t, false});
    table["Del"].push_back(__multiTableSymbol{s, t, false});

    parse_assert(i < From.size() && From[i] == ":");
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
            vector<string> genericHolder;

            while (i < From.size() && !(templCount == 0 && From[i] == ","))
            {
                if (templCount == 0 && From[i] != "<")
                {
                    lexedType.push_back(From[i]);
                }
                else
                {
                    genericHolder.push_back(From[i]);
                }

                if (From[i] == "<")
                {
                    templCount++;
                }
                else if (From[i] == ">")
                {
                    templCount--;

                    if (templCount == 0)
                    {
                        string toAdd = mangle(genericHolder);
                        genericHolder.clear();

                        if (toAdd != "")
                        {
                            lexedType.back().append("_" + toAdd);
                        }
                    }
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
