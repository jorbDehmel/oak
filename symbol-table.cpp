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
Type toType(const vector<string> &WhatIn, const set<string> &Local)
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

    set<string> localClone(Local);
    int i = 0;

    if (What[i] == "<")
    {
        while (What[i] != ">")
        {
            if (What[i] != "<" && What[i] != ">" && What[i] != ",")
            {
                localClone.insert(What[i]);
            }

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
        else if (localClone.count(cur) != 0)
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
    for (auto f : FromIn)
    {
        if (f.size() < 2 || f.substr(0, 2) != "//")
        {
            From.push_back(f);
        }
    }

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

    set<string> genericsSet;

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
        else
        {
            if (From[i] != "," && From[i] != "<" && From[i] != ">")
            {
                generics.push_back(From[i]);
                genericsSet.insert(From[i]);
            }
        }
    }

    if (toParse[0] == ">")
    {
        toParse.erase(toParse.begin());
    }

    if (generics.size() == 0)
    {
        table[Name].push_back(__multiTableSymbol{Seq, toType(toParse, genericsSet)});
    }
    else
    {
        templTable[Name].push_back(__template_info{generics, Seq, toType(toParse, genericsSet)});
    }

    return;
}

void replaceInSequence(sequence &In, const string &What, const Type &With)
{
    for (int i = 0; i < In.items.size(); i++)
    {
        if (In.items[i].info == atom && In.items[i].raw == What)
        {
            In.items[i].type = With;
        }
        else
        {
            replaceInSequence(In.items[i], What, With);
        }
    }
}

__multiTableSymbol *instantiateTemplate(const string &Name, const vector<Type> &GenericReplacements)
{
    // Examine candidates
    auto candidates = templTable[Name];
    if (candidates.size() == 0)
    {
        throw parse_error("No candidates exist for templated function '" + Name + "'");
    }

    bool found = false;
    __template_info toUse;
    for (int i = 0; i < candidates.size(); i++)
    {
        if (candidates[i].generics.size() == GenericReplacements.size())
        {
            found = true;
            toUse = candidates[i];
            break;
        }
    }

    if (!found)
    {
        for (auto c : candidates)
        {
            cout << "Candidate '" << Name << "' had " << c.generics.size() << " generics:\n\t";
            for (auto g : c.generics)
            {
                cout << '\t' << g;
            }
            cout << '\n';
        }

        cout << "Required generic count: " << GenericReplacements.size() << '\n';

        throw parse_error("No template candidates match generics count for function '" + Name + "'");
    }

    // At this point we can be assured that our template is legit

    // Do replacing
    for (int i = 0; i < GenericReplacements.size(); i++)
    {
        // Replace in sequence
        replaceInSequence(toUse.seq, toUse.generics[i], GenericReplacements[i]);

        // Replace in typing
        for (Type *cur = &toUse.type; cur != nullptr; cur = cur->next)
        {
            if (cur->info == atomic && cur->name == toUse.generics[i])
            {
                if (cur->next != nullptr)
                {
                    // Copy next
                    Type next = *cur->next;

                    // Copy in new
                    (*cur) = GenericReplacements[i];

                    // Append old suffix
                    cur->append(next);
                }
                else
                {
                    *cur = GenericReplacements[i];
                }
            }
        }
    }

    // Scan for existing instantiation and, if it exists, return it
    auto instantCandidates = table[Name];
    for (int i = 0; i < instantCandidates.size(); i++)
    {
        if (instantCandidates[i].type == toUse.type)
        {
            cout << tags::yellow_bold
                 << "Warning! Generic function "
                 << Name
                 << toStr(&toUse.type)
                 << " has already been instantiated;\n"
                 << "Repeated instantiation calls are redundant.\n"
                 << tags::reset;

            return &table[Name][i];
        }
    }

    table[Name].push_back(__multiTableSymbol{toUse.seq, toUse.type});

    return &table[Name].back();
}
