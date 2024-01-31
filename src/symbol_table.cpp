/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "symbol_table.hpp"
#include "enums.hpp"
#include "lexer.hpp"
#include "sequence_resources.hpp"
#include <stdexcept>

MultiSymbolTable table;

void parse_assert(const bool &what)
{
    if (!what)
    {
        throw parse_error("Parse assertion failed.");
    }
}

Type toType(const std::vector<std::string> &WhatIn)
{
    Token templ;
    templ.file = curFile;
    templ.line = curLine;
    templ.pos = 0;
    templ.state = alpha_state;

    std::vector<Token> temp;
    temp.reserve(WhatIn.size());

    for (const auto &item : WhatIn)
    {
        temp.push_back(item);
    }

    return toType(temp);
}

// Converts lexed symbols into a type
Type toType(const std::vector<Token> &WhatIn)
{
    if (WhatIn.size() == 0)
    {
        return Type(atomic, "NULL");
    }

    std::vector<Token> What;
    for (Token s : WhatIn)
    {
        What.push_back(s);
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
        std::string cur = What[i];

        if (cur == "^" || cur == "@")
        {
            out.append(pointer);
        }
        else if (cur == "<")
        {
            if (out == nullType)
            {
                throw std::runtime_error("Malformed generic statement.");
            }

            // Append to back

            // Collect generics
            std::vector<std::string> curGen;
            std::vector<std::vector<std::string>> generics;
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

                    if (count == 0)
                    {
                        if (curGen.size() > 0)
                        {
                            generics.push_back(curGen);
                        }

                        curGen.clear();
                    }
                    else
                    {
                        curGen.push_back(What[i]);
                    }
                }
                else if (What[i] == "," && count == 1)
                {
                    if (curGen.size() > 0)
                    {
                        generics.push_back(curGen);
                    }

                    curGen.clear();
                }
                else
                {
                    curGen.push_back(What[i]);
                }

                i++;
            } while (i < What.size() && count != 0);
            i--;

            // At this point, will only ever be a struct
            std::vector<std::string> temp;
            temp.push_back("struct");

            out[out.size() - 1].name = instantiateGeneric(out[out.size() - 1].name, generics, temp);
        }
        else if (cur == ",")
        {
            out.append(join);
        }
        else if (cur == "->")
        {
            out.append(maps);
        }
        else if (cur == "[]")
        {
            out.append(arr);
        }
        else if (cur == "[")
        {
            if (i + 1 >= What.size())
            {
                throw sequencing_error("'[' must be followed by a number or ']'");
            }

            try
            {
                long long result = stoi(What[i + 1].text);
                sm_assert(result > 0, "");
            }
            catch (...)
            {
                throw sequencing_error("Array size must be a compile-time constant positive integer, instead '" +
                                       What[i + 1].text + "' (" + curFile + ":" + std::to_string(What[i + 1].line) +
                                       ")");
            }

            out.append(Type(sarr, What[i + 1]));
            i++;
        }
        else if (cur == "]")
        {
            ;
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
        else if (What.size() > i + 1 && What[i + 1] == ":")
        {
            out.append(var_name, cur);
        }
        else
        {
            out.append(atomic, cur);
        }
    }

    for (const auto &what : out.internal)
    {
        if (what.info == atomic)
        {
            sm_assert(structData.count(what.name) != 0 || enumData.count(what.name) || atomics.count(what.name) != 0 ||
                          what.name == "struct" || what.name == "enum",
                      "Type '" + what.name + "' does not exist.");
            sm_assert(atomics.count(what.name) != 0 || structData[what.name].members.size() != 0 ||
                          enumData[what.name].options.size() != 0,
                      "Non-atomic struct with zero members may not be instantiated. You are likely trying to "
                      "instantiate a unit generic (used for traits), which is not valid usage.");
        }
    }

    return out;
}

// Can throw errors (IE malformed definitions)
void addStruct(const std::vector<Token> &From)
{
    // Assert the expression can be properly-formed
    parse_assert(From.size() >= 4);

    // Get name and check against malformations
    int i = 0;

    parse_assert(From[i] == "let");
    i++;

    std::string name = From[i];

    // Scrape generics here (and mangle)
    std::vector<std::vector<std::string>> generics;
    std::vector<std::string> curGen;

    i++;
    int count = 0;
    while (i < From.size() && From[i] != ":" && From[i] != "{")
    {
        if (From[i] == "<")
        {
            count++;

            if (count == 1)
            {
                i++;
                continue;
            }
        }

        else if (From[i] == ">")
        {
            count--;

            if (count == 0)
            {
                generics.push_back(curGen);
            }
        }

        else if (From[i] == "<")
        {
            count++;
        }

        curGen.push_back(From[i]);
        i++;
    }

    if (generics.size() != 0)
    {
        name = mangleStruct(name, generics);
    }

    if (structData.count(name) != 0)
    {
        std::cout << tags::yellow_bold << "Warning! Redefinition of struct '" << name << "'.\n" << tags::reset;
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

    ASTNode s;
    s.info = code_scope;
    s.type = Type(atomic, "void");
    s.items.push_back(ASTNode{});
    s.items.back().info = atom;
    s.items.back().raw = "//AUTOGEN";

    // Ensure these keys exist
    table["New"];
    table["Del"];

    table["New"].push_back(MultiTableSymbol{s, t});
    table["Del"].push_back(MultiTableSymbol{s, t});

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
            // name , name2 , name3 : type < std::string , hi > , name4 : type2 ,
            std::vector<Token> names, lexedType;

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
            std::vector<std::string> genericHolder;

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
                        std::string toAdd = mangle(genericHolder);
                        genericHolder.clear();

                        if (toAdd != "")
                        {
                            lexedType.back().text += "_" + toAdd;
                        }
                    }
                }

                i++;
            }

            Type toAdd = toType(lexedType);
            for (std::string varName : names)
            {
                structData[name].members[varName] = toAdd;
                structData[name].order.push_back(varName);

                // Add semicolon
                table["New"].back().seq.items.push_back(ASTNode{nullType, std::vector<ASTNode>(), atom, ";"});

                ASTNode toAppend;
                toAppend.info = atom;
                toAppend.type = nullType;
                toAppend.raw = getMemberNew("(*what)", varName, toAdd);

                table["New"].back().seq.items.push_back(toAppend);
            }
        }
    }
    else if (From[4] != ";")
    {
        throw parse_error("Malformed struct definition; Expected ';' or '{'.");
    }

    return;
}

/*
Erases any non-function symbols which were not present
in the original table. However, skips all functions.
If not contradicted by the above rules, bases off of
the current table (not backup).
*/
std::vector<std::pair<std::string, std::string>> restoreSymbolTable(MultiSymbolTable &backup)
{
    std::vector<std::pair<std::string, std::string>> out;

    MultiSymbolTable newTable;
    for (auto p : table)
    {
        for (auto s : p.second)
        {
            // Functions are always added- the logic for
            // this is handled elsewhere.
            if (s.type[0].info == function)
            {
                // Add to new table for sure
                newTable[p.first];
                newTable[p.first].push_back(s);
            }

            // Variables are more complicated
            else
            {
                // Check for presence in backup
                bool present = false;
                for (auto cand : backup[p.first])
                {
                    if (cand.type == s.type)
                    {
                        present = true;
                        break;
                    }
                }

                // If was present in backup, add for sure
                if (present)
                {
                    // Add to table for sure
                    newTable[p.first];
                    newTable[p.first].push_back(s);
                }

                // Otherwise, do not add (do destructor literal check)
                else
                {
                    // Variable falling out of scope
                    // Do not call Del if is atomic literal
                    if (!(s.type[0].info == atomic &&
                          (s.type[0].name == "u8" || s.type[0].name == "i8" || s.type[0].name == "u16" ||
                           s.type[0].name == "i16" || s.type[0].name == "u32" || s.type[0].name == "i32" ||
                           s.type[0].name == "u64" || s.type[0].name == "i64" || s.type[0].name == "u128" ||
                           s.type[0].name == "i128" || s.type[0].name == "f32" || s.type[0].name == "f64" ||
                           s.type[0].name == "f128" || s.type[0].name == "bool" || s.type[0].name == "str")) &&
                        s.type[0].info != function && s.type[0].info != pointer && s.type[0].info != arr &&
                        s.type[0].info != sarr && p.first != "")
                    {
                        // Del_FN_PTR_typename_MAPS_void
                        out.push_back(
                            std::make_pair(p.first, "Del_FN_PTR_" + s.type[0].name + "_MAPS_void(&" + p.first + ");"));
                    }
                }
            }
        }
    }

    table = newTable;

    return out;
}
