/*
Jordan Dehmel
2023 - present
jdehmel@outlook.com
*/

#include "oakc_fns.hpp"
#include "oakc_structs.hpp"
#include "options.hpp"

void sm_assert(const bool &expression, const std::string &message)
{
    if (!expression)
    {
        throw sequencing_error(std::string(message) + " (Failed assertion)");
    }
}

Type toType(const std::list<std::string> &What, AcornSettings &settings)
{
    Token templ;
    templ.file = settings.curFile;
    templ.line = settings.curLine;
    templ.pos = 0;
    templ.state = alpha_state;

    std::list<Token> temp;

    for (const auto &item : What)
    {
        temp.push_back(item);
    }

    return toType(temp, settings);
}

// Converts lexed symbols into a type
Type toType(const std::list<Token> &WhatIn, AcornSettings &settings)
{
    if (WhatIn.size() == 0)
    {
        return Type(atomic, "NULL");
    }

    std::list<Token> What;
    for (Token s : WhatIn)
    {
        What.push_back(s);
    }

    if (What.size() == 0)
    {
        return Type(atomic, "NULL");
    }

    auto it = What.begin();
    if (*it == "<")
    {
        while (it != What.end() && *it != ">")
        {
            it++;
        }
        it++;
    }

    Type out;
    for (; it != What.end(); it++)
    {
        std::string cur = *it;

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
            std::list<std::string> curGen;
            std::list<std::list<std::string>> generics;
            int count = 0;
            do
            {
                if (*it == "<")
                {
                    count++;

                    if (count > 1)
                    {
                        curGen.push_back(*it);
                    }
                }
                else if (*it == ">")
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
                        curGen.push_back(*it);
                    }
                }
                else if (*it == "," && count == 1)
                {
                    if (curGen.size() > 0)
                    {
                        generics.push_back(curGen);
                    }

                    curGen.clear();
                }
                else
                {
                    curGen.push_back(*it);
                }

                it++;
            } while (it != What.end() && count != 0);
            it--;

            // At this point, will only ever be a struct
            std::list<std::string> temp;
            temp.push_back("struct");

            out[out.size() - 1].name = instantiateGeneric(out[out.size() - 1].name, generics, temp, settings);
        }
        else if (cur == ",")
        {
            // Allows for the dummy comma
            if (!itCmp(What, it, 1, ")"))
            {
                out.append(join);
            }
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
            if (!itIsInRange(What, it, 1))
            {
                throw sequencing_error("'[' must be followed by a number or ']'");
            }

            try
            {
                long long result = stoi(itGet(What, it, 1).text);
                sm_assert(result > 0, "");
            }
            catch (...)
            {
                throw sequencing_error("Array size must be a compile-time constant positive integer, instead '" +
                                       itGet(What, it, 1).text + "' (" + settings.curFile.string() + ":" +
                                       std::to_string(itGet(What, it, 1).line) + ")");
            }

            out.append(Type(sarr, itGet(What, it, 1)));
            it++;
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
            it++;
        }
        else if (itCmp(What, it, 1, ":"))
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
            sm_assert(settings.structData.count(what.name) != 0 || settings.enumData.count(what.name) ||
                          atomics.count(what.name) != 0 || what.name == "struct" || what.name == "enum",
                      "Type '" + what.name + "' does not exist.");
            sm_assert(atomics.count(what.name) != 0 || settings.structData[what.name].members.size() != 0 ||
                          settings.enumData[what.name].options.size() != 0,
                      "Non-atomic struct with zero members may not be instantiated. You are likely trying to "
                      "instantiate a unit generic (used for traits), which is not valid usage.");
        }
    }

    return out;
}

void toCInternal(const ASTNode &What, std::list<std::string> &out, AcornSettings &settings)
{
    std::string temp;

    switch (What.info)
    {
    case code_line:
        for (auto it = What.items.begin(); it != What.items.end(); it++)
        {
            toCInternal(*it, out, settings);

            if (std::next(it) != What.items.end())
            {
                out.push_back(" ");
            }
        }

        break;

    case code_scope:
        out.push_back("{\n");

        for (auto &i : What.items)
        {
            if (i.info == keyword)
            {
                toCInternal(i, out, settings);
            }
            else if (i.type == nullType)
            {
                int old_size = out.size();
                toCInternal(i, out, settings);

                if (out.size() != old_size && !(old_size + 1 == out.size() && out.back() == ";"))
                {
                    out.push_back(temp);
                    out.push_back(";");
                    out.push_back("\n");
                }
            }
            else
            {
                // Janky return
                out.push_back("return ");
                toCInternal(i, out, settings);
                out.push_back(";\n");
            }
        }

        out.push_back("}");

        break;

    case atom:
        if (What.raw.size() > 0)
        {
            out.push_back(What.raw);
        }
        break;

    case keyword:
        out.push_back(What.raw);
        out.push_back(" ");

        for (auto child : What.items)
        {
            toCInternal(child, out, settings);
            out.push_back(" ");
        }

        break;

    case enum_keyword:
        // These get special treatment because they are heavily C-dependant.
        if (What.raw == "match")
        {
            // VERY different!
            // First item is object to capture.
            // Remaining items are case or defaults.

            Type type = What.items.front().type;
            std::string typeStr = toStrC(&type, settings);

            if (typeStr.substr(0, 7) == "struct ")
            {
                typeStr = typeStr.substr(7);
            }

            sm_assert(settings.enumData.count(typeStr) != 0,
                      "'match' may only be used on enums, not '" + typeStr + "'");
            std::list<std::string> options = settings.enumData[typeStr].order;

            std::map<std::string, bool> usedOptions;
            for (auto opt : settings.enumData[typeStr].order)
            {
                usedOptions[opt] = false;
            }

            // Skips any vestigial "Del" statements
            // COSTLY CALL:
            int max = std::next(What.items.begin())->items.size();
            while (max >= 0 &&
                   std::next(std::next(What.items.begin())->items.begin(), max - 1)->raw.substr(0, 3) == "Del")
            {
                max--;
            }

            int i = -1;
            for (auto &ind : std::next(What.items.begin())->items)
            {
                i++;

                if (ind.raw == "case")
                {
                    auto &cur = ind;

                    std::string optionName = cur.items.front().raw;

                    sm_assert(settings.enumData[typeStr].options.count(optionName) != 0,
                              "'" + optionName + "' is not an option for enum '" + typeStr + "'");
                    sm_assert(usedOptions.count(optionName) != 0,
                              "Option '" + optionName + "' cannot be used multiple times in a match statement.");
                    usedOptions.erase(optionName);

                    std::string captureName = std::next(cur.items.begin())->raw;

                    auto backupTable = settings.table;

                    if (i != 0)
                    {
                        out.push_back("else ");
                    }

                    out.push_back("if (");

                    toCInternal(What.items.front(), out, settings);
                    std::string itemStr = out.back();

                    out.push_back(".__info == ");
                    out.push_back(typeStr);
                    out.push_back("_OPT_");
                    out.push_back(optionName);
                    out.push_back(")\n{");

                    if (captureName != "NULL")
                    {
                        int numPtrs = 0;

                        Type clone = settings.enumData[typeStr].options[optionName];
                        while (clone.size() > 0 && clone[0].info == pointer)
                        {
                            numPtrs++;
                            clone.pop_front();
                        }

                        std::string captureType = mangleType(clone);

                        if (atomics.count(captureType) == 0)
                        {
                            out.push_back("struct ");
                        }

                        for (int l = 0; l < numPtrs; l++)
                        {
                            captureType += "*";
                        }

                        out.push_back(captureType);
                        out.push_back(" *");
                        out.push_back(captureName);
                        out.push_back(" = &");
                        out.push_back(itemStr);
                        out.push_back(".__data.");
                        out.push_back(optionName);
                        out.push_back("_data;\n");
                    }

                    // Add capture group to Oak table if needed

                    toCInternal(*std::next(cur.items.begin(), 2), out, settings);
                    out.push_back("\n}\n");
                }
                else if (ind.raw == "default")
                {
                    // Causes errors with destructor calls
                    // debugPrint(What.items[1]);

                    sm_assert(i + 2 >= std::next(What.items.begin())->items.size(),
                              "Default statement must be final branch of match statement.");
                    usedOptions.clear();

                    auto &cur = ind;

                    if (i == 0)
                    {
                        out.push_back("{\n");
                        toCInternal(cur.items.front(), out, settings);
                        out.push_back("\n}\n");
                    }
                    else
                    {
                        out.push_back("else\n{\n");
                        toCInternal(cur.items.front(), out, settings);
                        out.push_back("\n}\n");
                    }
                }
                else if (ind.raw != "")
                {
                    sm_assert(false, "Invalid option '" + ind.raw + "' in match statement.");
                }
            }

            if (usedOptions.size() != 0)
            {
                std::cout << tags::yellow_bold << "Warning: Match statement does not handle option(s) of enum '"
                          << typeStr << "':\n";

                for (auto opt : usedOptions)
                {
                    std::cout << '\t' << opt.first << '\n';
                }

                std::cout << tags::reset;
            }
        }
        else
        {
            out.push_back(What.raw);
            out.push_back(" ");
            for (auto child : What.items)
            {
                toCInternal(child, out, settings);
                out.push_back(" ");
            }
        }

        break;
    }

    return;
} // toCInternal

// Turn a .oak sequence into a .c one
std::string toC(const ASTNode &What, AcornSettings &settings)
{
    std::list<std::string> out;
    toCInternal(What, out, settings);

    int size = 0;
    for (const auto &what : out)
    {
        size += what.size();
    }

    std::string outStr;
    outStr.reserve(size);
    for (const auto &what : out)
    {
        outStr += what;
    }

    return outStr;
}

// Get the return type from a Type (of a function signature)
Type getReturnType(const Type &T, AcornSettings &settings)
{
    if (settings.getReturnTypeCache.count(T.ID) != 0)
    {
        return settings.getReturnTypeCache[T.ID];
    }

    Type temp(T);

    int count = 0;
    for (int cur = 0; cur < temp.size(); cur++)
    {
        if (temp[cur].info == function)
        {
            count++;
        }

        if (temp[cur].info == maps)
        {
            count--;

            if (count == 0)
            {
                Type out(temp, cur + 1);

                settings.getReturnTypeCache[T.ID] = out;

                return out;
            }
        }
    }

    if (settings.getReturnTypeCache.size() > 1000)
    {
        settings.getReturnTypeCache.clear();
    }

    settings.getReturnTypeCache[T.ID] = T;

    return T;
}

std::list<std::pair<std::string, Type>> getArgs(Type &type, AcornSettings &settings)
{
    // Check cache for existing value
    if (settings.cache.count(type.ID) != 0)
    {
        return settings.cache[type.ID];
    }

    // Get everything between final function and maps
    // For the case of function pointers, all variable names will be the unit string
    std::list<std::pair<std::string, Type>> out;
    std::string curName = "";
    Type curType = nullType;
    int count = 0;

    int cur = 0;

    // Dereference
    while (cur < type.size() && type[cur].info == pointer)
    {
        cur++;
    }

    // Should now point to first 'function'

    // Do iteration
    while (cur < type.size())
    {
        if (type[cur].info == function)
        {
            count++;
        }
        else if (type[cur].info == maps)
        {
            count--;

            if (count == 0)
            {
                // Final append
                if (curType != nullType)
                {
                    out.push_back(make_pair(curName, curType));
                }

                break;
            }

            // Skip to the end of the return type
            int subCount = 1;
            curType.append(type[cur].info);

            cur++;

            while (cur < type.size())
            {
                if (type[cur].info == function)
                {
                    subCount++;
                }
                else if (type[cur].info == maps)
                {
                    subCount--;
                }

                if (subCount == 0)
                {
                    if (type[cur].info == maps || type[cur].info == join)
                    {
                        break;
                    }
                }

                curType.append(type[cur].info, type[cur].name);

                cur++;
            }

            continue;
        }

        if (count == 1 && type[cur].info == var_name)
        {
            curName = type[cur].name;
        }
        else if (count == 1 && type[cur].info == join)
        {
            if (curType != nullType)
            {
                out.push_back(make_pair(curName, curType));
            }
            curName = "";
            curType = nullType;
        }
        else if (count != 1 || type[cur].info != function)
        {
            // Append to curType
            curType.append(type[cur].info, type[cur].name);
        }
        else if (type[cur].info == sarr)
        {
            throw sequencing_error("Sized arrays may not be used in argument types. Use unsized arrays ([]) instead.");
        }

        // Increment at end
        cur++;
    }

    // Copy into cache
    if (settings.cache.size() > 1000)
    {
        settings.cache.clear();
    }

    settings.cache[type.ID] = out;

    // Return
    return out;
}

// Can throw errors (IE malformed definitions)
void addStruct(const std::list<Token> &From, AcornSettings &settings)
{
    // Assert the expression can be properly-formed
    parse_assert(From.size() >= 4);

    // Get name and check against malformations
    auto it = From.begin();

    parse_assert(*it == "let");
    it++; // Now on 1st

    std::string name = *it;
    it++; // Now on 2nd

    // If unit, exit
    if (From.size() == 6)
    {
        bool isUnit = true;

        if (*it != ":")
        {
            isUnit = false;
        }

        it++; // Now on 3rd
        it++; // Now on 4th

        if (*it != "{")
        {
            isUnit = false;
        }

        it++; // Now on 5th

        if (*it != "}")
        {
            isUnit = false;
        }

        if (isUnit)
        {
            return;
        }
        else
        {
            it = std::prev(it, 3);
        }
    }

    // Scrape generics here (and mangle)
    std::list<std::list<std::string>> generics;
    std::list<std::string> curGen;

    int count = 0;
    while (it != From.end() && *it != ":" && *it != "{")
    {
        if (*it == "<")
        {
            count++;

            if (count == 1)
            {
                it++;
                continue;
            }
        }

        else if (*it == ">")
        {
            count--;

            if (count == 0)
            {
                generics.push_back(curGen);
            }
        }

        else if (*it == "<")
        {
            count++;
        }

        curGen.push_back(*it);
        it++;
    }

    if (generics.size() != 0)
    {
        name = mangleStruct(name, generics);
    }

    // Ensures unit structs still get added
    settings.structData[name];
    settings.structOrder.push_back(name);

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
    settings.table["New"];
    settings.table["Del"];
    settings.table["New"].push_back(MultiTableSymbol{s, t});
    settings.table["Del"].push_back(MultiTableSymbol{s, t});

    parse_assert(it != From.end() && *it == ":");
    it++;
    parse_assert(*it == "struct");
    it++;

    if (*it == "{")
    {
        it++;
        for (; it != From.end() && *it != "}"; it++)
        {
            // name : type ,
            // name , name2 , name3 : type < std::string , hi > , name4 : type2 ,
            std::list<Token> names, lexedType;

            while (std::next(it) != From.end() && *std::next(it) == ",")
            {
                names.push_back(*it);

                it++;
                it++;
            }

            names.push_back(*it);

            it++;

            parse_assert(it != From.end());
            parse_assert(*it == ":");

            it++;

            // Get lexed type (can be multiple symbols due to templating)
            int templCount = 0;
            std::list<std::string> genericHolder;

            while (it != From.end() && !(templCount == 0 && *it == ","))
            {
                if (templCount == 0 && *it != "<")
                {
                    lexedType.push_back(*it);
                }
                else
                {
                    genericHolder.push_back(*it);
                }

                if (*it == "<")
                {
                    templCount++;
                }
                else if (*it == ">")
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

                it++;
            }

            Type toAdd = toType(lexedType, settings);
            for (std::string varName : names)
            {
                settings.structData[name].members[varName] = toAdd;
                settings.structData[name].order.push_back(varName);

                // Add semicolon
                settings.table["New"].back().seq.items.push_back(ASTNode{nullType, std::list<ASTNode>(), atom, ";"});

                ASTNode toAppend;
                toAppend.info = atom;
                toAppend.type = nullType;
                toAppend.raw = getMemberNew("(*what)", varName, toAdd, settings);

                settings.table["New"].back().seq.items.push_back(toAppend);
            }
        }
    }
    else if (!itCmp(From, From.begin(), 4, ";"))
    {
        throw parse_error("Malformed struct definition; Expected ';' or '{'.");
    }

    return;
}

// Can throw errors (IE malformed definitions)
void addEnum(const std::list<Token> &From, AcornSettings &settings)
{
    // Assert the expression can be properly-formed
    parse_assert(From.size() >= 4);

    // Get name and check against malformations
    auto i = From.begin();

    parse_assert(*i == "let");
    i++;

    std::string name = *i;
    i++;

    parse_assert(i != From.end());

    // Scrape generics here (and mangle)
    std::list<std::string> curGen;
    std::list<std::list<std::string>> generics;

    int count = 0;
    while (i != From.end() && *i != ":" && *i != "{")
    {
        if (*i == "<")
        {
            count++;

            if (count == 1)
            {
                i++;
                continue;
            }
        }

        else if (*i == ">")
        {
            count--;

            if (count == 0)
            {
                generics.push_back(curGen);
            }
        }

        else if (*i == "<")
        {
            count++;
        }

        curGen.push_back(*i);
        i++;
    }

    if (generics.size() != 0)
    {
        name = mangleStruct(name, generics);
    }

    // Ensure for unit enums
    settings.structOrder.push_back(name);

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
    settings.table["New"];
    settings.table["Del"];

    settings.table["New"].push_back(MultiTableSymbol{s, t, false, settings.curFile});
    settings.table["Del"].push_back(MultiTableSymbol{s, t, false, settings.curFile});

    parse_assert(*i == ":");
    i++;
    parse_assert(*i == "enum");
    i++;

    if (*i == "{")
    {
        i++;
        for (; i != From.end() && *i != "}"; i++)
        {
            // name : type ,
            // name , name2 , name3 : type < string , hi > , name4 : type2 ,
            std::list<std::string> names;
            std::list<Token> lexedType;

            while (std::next(i) != From.end() && *std::next(i) == ",")
            {
                names.push_back(*i);

                i++;
                i++;
            }

            names.push_back(*i);

            i++;
            parse_assert(i != From.end());
            parse_assert(*i == ":");
            i++;

            // Get lexed type (can be multiple symbols due to templating)
            int templCount = 0;
            std::list<std::string> genericHolder;

            while (i != From.end() && !(templCount == 0 && *i == ","))
            {
                if (templCount == 0 && *i != "<")
                {
                    lexedType.push_back(*i);
                }
                else
                {
                    genericHolder.push_back(*i);
                }

                if (*i == "<")
                {
                    templCount++;
                }
                else if (*i == ">")
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

            Type toAdd = toType(lexedType, settings);
            for (std::string varName : names)
            {
                settings.enumData[name].options[varName] = toAdd;
                settings.enumData[name].order.push_back(varName);
            }
        }
    }
    else if (*std::next(From.begin(), 4) != ";")
    {
        throw parse_error("Malformed enum definition; Expected ';' or '{'.");
    }

    EnumLookupData &cur = settings.enumData[name];
    std::string enumTypeStr = name;
    for (auto optionName : cur.order)
    {
        if (cur.options[optionName][0].info == atomic && cur.options[optionName][0].name == "unit")
        {
            // Unit struct; Single argument constructor

            // Insert Oak version
            Type constructorType = nullType;

            constructorType.append(function);
            constructorType.append(var_name, "self");
            constructorType.append(pointer);
            constructorType.append(atomic, enumTypeStr);
            constructorType.append(maps);
            constructorType.append(atomic, "void");

            settings.table["wrap_" + optionName].push_back(
                MultiTableSymbol{ASTNode{}, constructorType, false, settings.curFile});
        }
        else
        {
            // Double argument constructor

            // Insert Oak version
            Type constructorType = nullType;

            constructorType.append(function);
            constructorType.append(var_name, "self");
            constructorType.append(pointer);
            constructorType.append(atomic, enumTypeStr);
            constructorType.append(join);
            constructorType.append(var_name, "data");
            constructorType.append(cur.options[optionName]);
            constructorType.append(maps);
            constructorType.append(atomic, "void");

            settings.table["wrap_" + optionName].push_back(
                MultiTableSymbol{ASTNode{}, constructorType, false, settings.curFile});
        }
    }

    return;
}

// Dump data to file
void dump(const std::list<Token> &Lexed, const std::string &Where, const std::string &FileName, const int &Line,
          const ASTNode &FileSeq, const std::list<Token> LexedBackup, const std::string &ErrorMsg,
          AcornSettings &settings)
{
    std::string sep;
    sep.reserve(50);
    for (int i = 0; i < 50; i++)
    {
        sep += '-';
    }
    sep += '\n';

    std::ofstream file(Where);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open dump file '" + Where + "'.");
    }

    auto curTime = time(NULL);

    file << sep << "Dump file for " << FileName << " generated by Acorn at " << ctime(&curTime);

    if (ErrorMsg != "")
    {
        file << sep << "Log was caused by error:\n" << ErrorMsg << '\n';
    }

    file << sep << "// Pre-everything lexed:\n";

    unsigned int currentLine = 0;
    for (auto s : LexedBackup)
    {
        if (currentLine + 20 >= s.line)
        {
            while (currentLine < s.line)
            {
                currentLine++;
                file << "\n" << currentLine << "\t|\t";
            }
        }
        else if (s.line - currentLine != 0)
        {
            file << "\n\n<omitted " << s.line - currentLine << " lines here>\n";
            currentLine = s.line;
            file << "\n" << currentLine << "\t|\t";
        }
        file << s.text << ' ';
    }
    file << '\n';

    file << sep << "// Post-substitution lexed:\n";

    currentLine = 0;
    for (auto s : Lexed)
    {
        if (currentLine + 20 >= s.line)
        {
            while (currentLine < s.line)
            {
                currentLine++;
                file << "\n" << currentLine << "\t|\t";
            }
        }
        else if (s.line - currentLine != 0)
        {
            file << "\n\n<omitted " << s.line - currentLine << " lines here>\n";
            currentLine = s.line;
            file << "\n" << currentLine << "\t|\t";
        }
        file << s.text << ' ';
    }
    file << '\n';

    file << sep << "// Symbols and their types:\n";

    for (auto p : settings.table)
    {
        file << p.first << ":\n";

        for (auto item : p.second)
        {
            file << '\t' << toStr(&item.type) << '\n';
        }
    }

    file << sep << "// Structs:\n";

    for (auto s : settings.structData)
    {
        file << s.first << "\n";

        if (s.second.members.size() == 0)
        {
            file << "\t<no members>\n";
        }
        else
        {
            for (auto m : s.second.members)
            {
                file << '\t' << m.first << '\t' << toStr(&m.second) << '\n';
            }
        }
    }

    file << sep << "// Enums:\n";

    for (auto e : settings.enumData)
    {
        file << e.first << "\n";

        for (auto m : e.second.options)
        {
            file << '\t' << m.first << '\t' << toStr(&m.second) << '\n';
        }
    }

    file << sep << "// Generics:\n";

    printGenericDumpInfo(file, settings);

    file << sep << "// Full anatomy:\n";

    debugPrint(FileSeq, 0, file);

    file << sep << "// All rules:\n";

    int i = 0;
    for (auto s : settings.dialectRules)
    {
        file << i << '\t' << s << '\n' << '\t';

        for (auto t : settings.rules[s].inputPattern)
        {
            file << t.text << ' ';
        }

        file << "\n\t";

        for (auto t : settings.rules[s].outputPattern)
        {
            file << t.text << ' ';
        }

        file << "\n";
        i++;
    }
    for (auto s : settings.activeRules)
    {
        file << i << '\t' << s << '\n' << '\t';

        for (auto t : settings.rules[s].inputPattern)
        {
            file << t.text << ' ';
        }

        file << "\n\t";

        for (auto t : settings.rules[s].outputPattern)
        {
            file << t.text << ' ';
        }

        file << "\n";
        i++;
    }

    file << sep << "// All bundles:\n";

    for (auto p : settings.bundles)
    {
        file << p.first << "\n\t";

        for (auto r : p.second)
        {
            file << r << ' ';
        }

        file << '\n';
    }

    file << sep << "// Active rules:\n";

    for (auto s : settings.activeRules)
    {
        file << s << '\n';
    }

    file.close();
    return;
}

////////////////////////////////////////////////////////////////

// Get all candidates which match exactly.
std::list<int> getExactCandidates(const std::list<std::list<Type>> &candArgs, const std::list<Type> &argTypes)
{
    std::list<int> out;
    bool isMatch;

    // Check for exact match for each candidate
    int j_ind = -1;
    for (auto &j : candArgs)
    {
        j_ind++;

        if (j.size() != argTypes.size())
        {
            // Cannot match if number of arguments is different
            continue;
        }

        isMatch = true;

        // Iterate over argument types
        auto l = j.begin();
        auto r = argTypes.begin();

        for (int k = 0; k < j.size(); k++)
        {
            if (!typesAreSameExact(&*l, &*r))
            {
                isMatch = false;
                break;
            }

            l++;
            r++;
        }

        if (isMatch)
        {
            out.push_back(j_ind);
        }
    }

    return out;
}

// Get all candidates which match with implicit casting.
std::list<int> getCastingCandidates(const std::list<std::list<Type>> &candArgs, const std::list<Type> &argTypes)
{
    std::list<int> out;
    bool isMatch;

    // Min number of conversions
    int cur;
    auto minIter = out.end();

    // Check for exact match for each candidate
    int j_ind = -1;
    for (auto j = candArgs.begin(); j != candArgs.end(); j++)
    {
        j_ind++;

        if (j->size() != argTypes.size())
        {
            // Cannot match if number of arguments is different
            continue;
        }

        isMatch = true;

        // Iterate over argument types
        cur = 0;

        auto l = j->begin();
        auto r = argTypes.begin();

        for (int k = 0; k < j->size(); k++)
        {
            if (!typesAreSameCast(&*r, &*l, cur))
            {
                isMatch = false;
                break;
            }

            l++;
            r++;
        }

        if (isMatch)
        {
            out.push_back(j_ind);
            if (minIter == out.end() || cur < *minIter)
            {
                minIter = std::prev(out.end());
            }
        }
    }

    // Make the occurrence with the fewest casts first
    if (minIter != out.end())
    {
        auto temp = out.front();
        out.front() = *minIter;
        *minIter = temp;
    }

    return out;
}

// Get all candidates which match with auto referencing and/or
// dereferencing.
std::list<int> getReferenceCandidates(const std::list<std::list<Type>> &candArgs, const std::list<Type> &argTypes)
{
    std::list<int> out;
    bool isMatch;
    int cur;
    auto minIter = out.end();

    // Check for exact match for each candidate
    int j_ind = -1;
    for (auto &j : candArgs)
    {
        j_ind++;

        if (j.size() != argTypes.size())
        {
            // Cannot match if number of arguments is different
            continue;
        }

        isMatch = true;

        // Iterate over argument types
        // Iterate over argument types
        cur = 0;

        auto l = j.begin();
        auto r = argTypes.begin();

        for (int k = 0; k < j.size(); k++)
        {
            if (!typesAreSame(&*l, &*r, cur))
            {
                isMatch = false;
                break;
            }

            l++;
            r++;
        }

        if (isMatch)
        {
            out.push_back(j_ind);
            if (minIter == out.end() || cur < *minIter)
            {
                minIter = std::prev(out.end());
            }
        }
    }

    // Make the occurrence with the fewest casts first
    if (minIter != out.end())
    {
        auto temp = out.front();
        out.front() = *minIter;
        *minIter = temp;
    }

    return out;
}

////////////////////////////////////////////////////////////////

// Prints the reason why each candidate was rejected
void printCandidateErrors(const std::vector<MultiTableSymbol> &candidates, const std::list<Type> &argTypes,
                          const std::string &name, AcornSettings &settings)
{
    std::cout << "--------------------------------------------------\n";

    std::list<std::list<Type>> candArgs;
    for (auto item : candidates)
    {
        Type curType = item.type;
        while (curType != nullType && curType[0].info == pointer)
        {
            curType.pop_front();
        }

        auto args = getArgs(item.type, settings);
        candArgs.push_back(std::list<Type>());

        for (auto arg : args)
        {
            if (arg.second == nullType)
            {
                continue;
            }

            candArgs.back().push_back(arg.second);
        }
    }

    auto s1 = getExactCandidates(candArgs, argTypes);
    auto s2 = getCastingCandidates(candArgs, argTypes);
    auto s3 = getReferenceCandidates(candArgs, argTypes);

    std::cout << "Passed s1: " << s1.size() << '\n'
              << "Passed s2: " << s2.size() << '\n'
              << "Passed s3: " << s3.size() << "\n\n";

    auto candArgIter = candArgs.begin();
    for (int i = 0; i < candidates.size(); i++, candArgIter++)
    {
        std::cout << std::left << name << std::setw(30) << toStr(&candidates[i].type) << "\tFrom "
                  << candidates[i].sourceFilePath << '\n';

        if (candidates[i].erased)
        {
            // Was erased
            std::cout << "\tWas erased.\n";
        }
        else
        {
            // Stage 1
            if (find(s1.begin(), s1.end(), i) == s1.end())
            {
                std::cout << "\tFailed exact match.\n";
            }
            else
            {
                std::cout << tags::green << "\tPassed exact match.\n" << tags::reset;
            }

            // Stage 2
            if (find(s2.begin(), s2.end(), i) == s2.end())
            {
                std::cout << "\tFailed casting match.\n";
            }
            else
            {
                std::cout << tags::green << "\tPassed casting match.\n" << tags::reset;
            }

            // Stage 3
            if (find(s3.begin(), s3.end(), i) == s3.end())
            {
                std::cout << "\tFailed ref/deref match.\n";
            }
            else
            {
                std::cout << tags::green << "\tPassed ref/deref match.\n" << tags::reset;
            }
        }

        std::cout << "\tArgs:\n";
        for (auto arg : *candArgIter)
        {
            std::cout << "\t\t" << toStr(&arg) << '\n';
        }
    }

    std::cout << "--------------------------------------------------\n"
              << "Provided argument types:\n";

    for (auto arg : argTypes)
    {
        std::cout << '\t' << toStr(&arg) << '\n';
    }

    std::cout << "--------------------------------------------------\n\n";

    return;
}

std::string cleanMacroArgument(const std::string &from)
{
    sm_assert(from.size() > 1 &&
                  ((from.front() == '"' && from.back() == '"') || (from.front() == '\'' && from.back() == '\'')),
              "Internal macro arguments must be string-enclosed.");

    std::string out;
    out.reserve(from.size() - 2);
    bool force = false;
    for (int l = 1; l + 1 < from.size(); l++)
    {
        if (!force)
        {
            if (from[l] == '\\')
            {
                force = true;
                continue;
            }

            out.push_back(from[l]);
        }
        else
        {
            if (from[l] == 'n')
            {
                out.push_back('\n');
            }
            else if (from[l] == 't')
            {
                out.push_back('\t');
            }
            else if (from[l] == 'b')
            {
                out.push_back('\b');
            }
            else
            {
                out.push_back(from[l]);
            }

            force = false;
        }
    }

    return out;
}

// Destroy all unit, temp, or autogen definitions matching a given type.
// Can throw errors if doThrow is true.
// Mostly used for New and Del, Oak ~0.0.14
void destroyUnits(const std::string &name, const Type &type, const bool &doThrow, AcornSettings &settings)
{
    // Safety check
    if (settings.table.count(name) != 0 && settings.table[name].size() != 0)
    {
        // Iterate over items
        for (auto i = settings.table[name].begin(), end = settings.table[name].end(); i != end; i++)
        {
            if (i->type == type)
            {
                // If is unit, erase
                if (i->seq.items.size() == 0 || (i->seq.items.size() >= 1 && i->seq.items.front().raw.size() > 8 &&
                                                 i->seq.items.front().raw.substr(0, 9) == "//AUTOGEN"))
                {
                    i = settings.table[name].erase(i);
                }

                // Else if doThrow, throw redef error
                else if (doThrow)
                {
                    throw sequencing_error("Function " + name + toStr(&type) +
                                           "' cannot have multiple explicit definitions.");
                }
            }
        }
    }

    return;
}

// Actually used in dump files, not just for debugging.
void debugPrint(const ASTNode &What, int spaces, std::ostream &to)
{
    for (int i = 0; i < spaces; i++)
    {
        to << "| ";
    }

    switch (What.info)
    {
    case code_scope:
        to << "code_scope";
        break;
    case code_line:
        to << "code_line";
        break;
    case atom:
        to << "atom";
        break;
    case keyword:
        to << "keyword";
        break;
    case enum_keyword:
        to << "enum_keyword";
        break;
    }

    to << " w/ raw " << What.raw << ", type " << toStr(&What.type) << ":\n";
    for (auto s : What.items)
    {
        debugPrint(s, spaces + 1, to);
    }

    return;
}

// Assumes self is NOT a pointer
std::string getMemberNew(const std::string &selfName, const std::string &varName, const Type &varType,
                         AcornSettings &settings)
{
    if (varType[0].info == atomic)
    {
        // Atomic

        // Ensure function exists
        bool isValid = false;
        auto candidates = settings.table["New"];
        for (auto candidate : candidates)
        {
            if (candidate.type[0].info != function || candidate.type.size() < 4 || candidate.type[1].info != var_name ||
                candidate.type[2].info != pointer || candidate.type[3].info != atomic ||
                candidate.type[3].name != varType[0].name)
            {
                continue;
            }

            isValid = true;
            break;
        }

        sm_assert(isValid, "Cannot call constructor on member variable '" + varName + "' of type '" + toStr(&varType) +
                               "': No constructor exists.");

        return "New_FN_PTR_" + mangleType(varType) + "_MAPS_void(&" + selfName + "." + varName + ")";
    }
    else if (varType[0].info == sarr)
    {
        // Sized array
        return "for (i32 _i = 0; _i < " + varType[0].name + "; _i++) " +
               getMemberNew(selfName, varName + "[_i]", Type(varType, 1), settings) + ";";
    }
    else
    {
        // Pointer or equivalent
        return selfName + "." + varName + " = 0";
    }
}

std::string getMemberDel(const std::string &selfName, const std::string &varName, const Type &varType,
                         AcornSettings &settings)
{
    if (varType[0].info == atomic)
    {
        // Atomic

        // Ensure function exists
        bool isValid = false;
        auto candidates = settings.table["Del"];
        for (auto candidate : candidates)
        {
            if (candidate.type[0].info != function || candidate.type.size() < 4 || candidate.type[1].info != var_name ||
                candidate.type[2].info != pointer || candidate.type[3].info != atomic ||
                candidate.type[3].name != varType[0].name)
            {
                continue;
            }

            isValid = true;
            break;
        }

        sm_assert(isValid, "Cannot call destructor on member variable '" + varName + "' of type '" + toStr(&varType) +
                               "': No denstructor exists.");

        return "Del_FN_PTR_" + mangleType(varType) + "_MAPS_void(&" + selfName + "." + varName + ")";
    }
    else if (varType[0].info == sarr)
    {
        // Sized array
        return "for (i32 _i = 0; _i < " + varType[0].name + "; _i++) " +
               getMemberDel(selfName, varName + "[_i]", Type(varType, 1), settings) + ";";
    }
    else
    {
        // Pointer or equivalent
        // This is where memory leaks come from
        return selfName + "." + varName + " = 0";
    }
}

// Returns an item to insert before a given sequence
std::string insertDestructorsRecursive(ASTNode &what, const std::list<std::pair<std::string, std::string>> &destructors,
                                       AcornSettings &settings)
{
    if ((what.info == code_line || what.info == atom) && what.type != nullType)
    {
        // Janky return

        std::string out, against;
        against = toC(what, settings);

        for (const auto &p : destructors)
        {
            if (p.first == against)
            {
                continue;
            }
            else
            {
                out += p.second + ";";
            }
        }

        return out;
    }
    else if (what.info == keyword && what.items.front().raw == "return")
    {
        // Keyword return

        std::string out, against;
        against = toC(*std::next(what.items.begin()), settings);

        for (const auto &p : destructors)
        {
            if (p.first == against || p.second == "")
            {
                continue;
            }
            else
            {
                out += p.second + ";";
            }
        }

        return out;
    }
    else if (what.info == code_scope)
    {
        // Recursive
        std::string out;
        for (auto i = what.items.begin(); i != what.items.end(); i++)
        {
            out = insertDestructorsRecursive(*i, destructors, settings);

            if (out != "")
            {
                // Insert the thing returned

                i = what.items.insert(i, ASTNode{nullType, std::list<ASTNode>(), atom, out});
                i++;
            }
        }

        return "";
    }

    return "";
}

// Insert destructors before any return statement that DOES NOT return the item in question
// Recursive
// Will only be called upon exiting a function
void insertDestructors(ASTNode &what, const std::list<std::pair<std::string, std::string>> &destructors,
                       AcornSettings &settings)
{
    insertDestructorsRecursive(what, destructors, settings);
}
