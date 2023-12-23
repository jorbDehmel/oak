#include "sequence_resources.hpp"
#include "symbol_table.hpp"
#include "type_builder.hpp"

void toCInternal(const sequence &What, vector<string> &out)
{
    string temp;

    switch (What.info)
    {
    case code_line:
        for (int i = 0; i < What.items.size(); i++)
        {
            toCInternal(What.items[i], out);

            if (i + 1 != What.items.size())
            {
                out.push_back(" ");
            }
        }

        break;

    case code_scope:
        out.push_back("{\n");

        for (int i = 0; i < What.items.size(); i++)
        {
            if (What.items[i].info == keyword)
            {
                toCInternal(What.items[i], out);
            }
            else if (What.items[i].type == nullType)
            {
                temp = toC(What.items[i]);

                if (temp != "" && temp != ";")
                {
                    out.push_back(temp);

                    if (!(temp.size() > 1 && temp.substr(0, 2) == "//"))
                    {
                        out.push_back(";");
                    }

                    out.push_back("\n");
                }
            }
            else
            {
                // Janky return
                out.push_back("return ");
                toCInternal(What.items[i], out);
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
            toCInternal(child, out);
            out.push_back(" ");
        }

        break;

    case enum_keyword:
        // These get special treatment because they are heavily C++-dependant.
        if (What.raw == "match")
        {
            // VERY different!
            // First item is object to capture.
            // Remaining items are case or defaults.

            Type type = What.items[0].type;
            string typeStr = toStrC(&type);

            if (typeStr.substr(0, 7) == "struct ")
            {
                typeStr = typeStr.substr(7);
            }

            sm_assert(enumData.count(typeStr) != 0, "'match' may only be used on enums, not '" + typeStr + "'");
            vector<string> options = enumData[typeStr].order;

            map<string, bool> usedOptions;
            for (auto opt : enumData[typeStr].order)
            {
                usedOptions[opt] = false;
            }

            // Skips any vestigial "Del" statements
            int max = What.items[1].items.size();
            while (max >= 0 && What.items[1].items[max - 1].raw.substr(0, 3) == "Del")
            {
                max--;
            }

            for (int ind = 0; ind < max; ind++)
            {
                if (What.items[1].items[ind].raw == "case")
                {
                    auto &cur = What.items[1].items[ind];

                    string optionName = cur.items[0].raw;

                    sm_assert(enumData[typeStr].options.count(optionName) != 0,
                              "'" + optionName + "' is not an option for enum '" + typeStr + "'");
                    sm_assert(usedOptions.count(optionName) != 0,
                              "Option '" + optionName + "' cannot be used multiple times in a match statement.");
                    usedOptions.erase(optionName);

                    string captureName = cur.items[1].raw;

                    auto backupTable = table;

                    if (ind != 0)
                    {
                        out.push_back("else ");
                    }

                    out.push_back("if (");

                    toCInternal(What.items[0], out);
                    string itemStr = out.back();

                    out.push_back(".__info == ");
                    out.push_back(typeStr);
                    out.push_back("_OPT_");
                    out.push_back(optionName);
                    out.push_back(")\n{");

                    if (captureName != "NULL")
                    {
                        int numPtrs = 0;

                        Type clone = enumData[typeStr].options[optionName];
                        while (clone.size() > 0 && clone[0].info == pointer)
                        {
                            numPtrs++;
                            clone.pop_front();
                        }

                        string captureType = mangleType(clone);

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

                    toCInternal(cur.items[2], out);
                    out.push_back("\n}\n");
                }
                else if (What.items[1].items[ind].raw == "default")
                {
                    // Causes errors with destructor calls
                    // debugPrint(What.items[1]);

                    sm_assert(ind + 2 >= What.items[1].items.size(),
                              "Default statement must be final branch of match statement.");
                    usedOptions.clear();

                    auto &cur = What.items[1].items[ind];

                    if (ind == 0)
                    {
                        out.push_back("{\n");
                        toCInternal(cur.items[0], out);
                        out.push_back("\n}\n");
                    }
                    else
                    {
                        out.push_back("else\n{\n");
                        toCInternal(cur.items[0], out);
                        out.push_back("\n}\n");
                    }
                }
                else if (What.items[1].items[ind].raw != "")
                {
                    sm_assert(false, "Invalid option '" + What.items[1].items[ind].raw + "' in match statement.");
                }
            }

            if (usedOptions.size() != 0)
            {
                cout << tags::yellow_bold << "Warning! Match statement does not handle option(s) of enum '" << typeStr
                     << "':\n";

                for (auto opt : usedOptions)
                {
                    cout << '\t' << opt.first << '\n';
                }

                cout << tags::reset;
            }
        }
        else
        {
            cout << tags::yellow_bold << "Warning! Unknown enum keyword '" << What.raw
                 << "'. Treating as regular keyword.\n"
                 << tags::reset;

            out.push_back(What.raw);
            out.push_back(" ");
            for (auto child : What.items)
            {
                toCInternal(child, out);
                out.push_back(" ");
            }
        }

        break;
    }

    return;
} // toCInternal

// Turn a .oak sequence into a .cpp one
string toC(const sequence &What)
{
    vector<string> out;
    toCInternal(What, out);

    int size = 0;
    for (const auto &what : out)
    {
        size += what.size();
    }

    string outStr;
    outStr.reserve(size);
    for (const auto &what : out)
    {
        outStr += what;
    }

    return outStr;
}

// Get the return type from a Type (of a function signature)
Type getReturnType(const Type &T)
{
    static map<unsigned long long, Type> getReturnTypeCache;

    if (getReturnTypeCache.count(T.ID) != 0)
    {
        return getReturnTypeCache[T.ID];
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

                getReturnTypeCache[T.ID] = out;

                return out;
            }
        }
    }

    if (getReturnTypeCache.size() > 1000)
    {
        getReturnTypeCache.clear();
    }

    getReturnTypeCache[T.ID] = T;

    return T;
}

vector<pair<string, Type>> getArgs(Type &type)
{
    static map<unsigned long long, vector<pair<string, Type>>> cache;

    // Check cache for existing value
    if (cache.count(type.ID) != 0)
    {
        return cache[type.ID];
    }

    // Get everything between final function and maps
    // For the case of function pointers, all variable names will be the unit string
    vector<pair<string, Type>> out;
    string curName = "";
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
                out.push_back(make_pair(curName, curType));
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
            out.push_back(make_pair(curName, curType));
            curName = "";
            curType = nullType;
        }
        else if (count != 1 || type[cur].info != function)
        {
            // Append to curType
            curType.append(type[cur].info, type[cur].name);
        }

        // Increment at end
        cur++;
    }

    // Copy into cache
    if (cache.size() > 1000)
    {
        cache.clear();
    }

    cache[type.ID] = out;

    // Return
    return out;
}

// Can throw errors (IE malformed definitions)
void addEnum(const vector<string> &FromIn)
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

    parse_assert(i < From.size());

    // Scrape generics here (and mangle)
    vector<string> curGen;
    vector<vector<string>> generics;

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

    if (enumData.count(name) != 0 || structData.count(name) != 0)
    {
        cout << tags::yellow_bold << "Warning! Definition of enum '" << name << "' erases struct of the same name.\n"
             << tags::reset;
    }

    // Ensure for unit enums
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

    table["New"].push_back(__multiTableSymbol{s, t, false, curFile});
    table["Del"].push_back(__multiTableSymbol{s, t, false, curFile});

    parse_assert(From[i] == ":");
    i++;
    parse_assert(From[i] == "enum");
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
                enumData[name].options[varName] = toAdd;
                enumData[name].order.push_back(varName);
            }
        }
    }
    else if (From[4] != ";")
    {
        throw parse_error("Malformed enum definition; Expected ';' or '{'.");
    }

    enumData[name].size = 0;
    __enumLookupData &cur = enumData[name];
    string enumTypeStr = name;
    unsigned long long curSize;
    for (auto optionName : cur.order)
    {
        // Sizing
        if (cur.options[optionName][0].info == atomic)
        {
            if (structData.count(cur.options[optionName][0].name) != 0)
            {
                curSize = structData[cur.options[optionName][0].name].size;
            }
            else
            {
                curSize = 1;
            }
        }
        else
        {
            curSize = sizeof(void *);
        }

        if (curSize > cur.size)
        {
            cur.size = curSize;
        }

        if (cur.options[optionName][0].info == atomic && cur.options[optionName][0].name == "unit")
        {
            // Unit struct; Single argument constructor

            // Insert Oak version
            Type constructorType = nullType;
            constructorType.reserve(6);

            constructorType.append(function);
            constructorType.append(var_name, "self");
            constructorType.append(pointer);
            constructorType.append(atomic, enumTypeStr);
            constructorType.append(maps);
            constructorType.append(atomic, "void");

            table["wrap_" + optionName].push_back(__multiTableSymbol{sequence{}, constructorType, false, curFile});
        }
        else
        {
            // Double argument constructor

            // Insert Oak version
            Type constructorType = nullType;
            constructorType.reserve(9);

            constructorType.append(function);
            constructorType.append(var_name, "self");
            constructorType.append(pointer);
            constructorType.append(atomic, enumTypeStr);
            constructorType.append(join);
            constructorType.append(var_name, "data");
            constructorType.append(cur.options[optionName]);
            constructorType.append(maps);
            constructorType.append(atomic, "void");

            table["wrap_" + optionName].push_back(__multiTableSymbol{sequence{}, constructorType, false, curFile});
        }
    }

    cur.size += enumSize;

    return;
}

// Dump data to file
void dump(const vector<string> &Lexed, const string &Where, const string &FileName, const int &Line,
          const sequence &FileSeq, const vector<string> LexedBackup)
{
    string sep = "";
    for (int i = 0; i < 50; i++)
    {
        sep += '-';
    }
    sep += '\n';

    ofstream file(Where);
    if (!file.is_open())
    {
        throw runtime_error("Failed to open dump file '" + Where + "'; At this point, you should give up.");
    }

    auto curTime = time(NULL);

    file << sep << "Dump file for " << FileName << " generated by Acorn at " << ctime(&curTime);

    file << sep << "// Pre-everything lexed:\n";

    for (auto s : LexedBackup)
    {
        if (s.size() >= 2 && s.substr(0, 2) == "//")
        {
            file << "\n" << s << "\t|\t";
        }
        else
        {
            file << s << ' ';
        }
    }
    file << '\n';

    file << sep << "// Post-substitution lexed:\n";

    for (auto s : Lexed)
    {
        if (s.size() >= 2 && s.substr(0, 2) == "//")
        {
            file << "\n" << s << "\t|\t";
        }
        else
        {
            file << s << ' ';
        }
    }
    file << '\n';

    file << sep << "// Symbols and their types:\n";

    for (auto p : table)
    {
        file << p.first << ":\n";

        for (auto item : p.second)
        {
            file << '\t' << toStr(&item.type) << '\n';
        }
    }

    file << sep << "// Structs:\n";

    for (auto s : structData)
    {
        file << s.first << "\n";

        for (auto m : s.second.members)
        {
            file << '\t' << m.first << '\t' << toStr(&m.second) << '\n';
        }
    }

    file << sep << "// Enums:\n";

    for (auto e : enumData)
    {
        file << e.first << "\n";

        for (auto m : e.second.options)
        {
            file << '\t' << m.first << '\t' << toStr(&m.second) << '\n';
        }
    }

    file << sep << "// Generics:\n";

    printGenericDumpInfo(file);

    file << sep << "// Full anatomy:\n";

    debugPrint(FileSeq, 0, file);

    file << sep << "// All rules:\n";

    int i = 0;
    for (auto s : dialectRules)
    {
        file << i << '\t' << s << '\n' << '\t';

        for (auto t : rules[s].inputPattern)
        {
            file << t << ' ';
        }

        file << "\n\t";

        for (auto t : rules[s].outputPattern)
        {
            file << t << ' ';
        }

        file << "\n";
        i++;
    }
    for (auto s : activeRules)
    {
        file << i << '\t' << s << '\n' << '\t';

        for (auto t : rules[s].inputPattern)
        {
            file << t << ' ';
        }

        file << "\n\t";

        for (auto t : rules[s].outputPattern)
        {
            file << t << ' ';
        }

        file << "\n";
        i++;
    }

    file << sep << "// All bundles:\n";

    for (auto p : bundles)
    {
        file << p.first << "\n\t";

        for (auto r : p.second)
        {
            file << r << ' ';
        }

        file << '\n';
    }

    file << sep << "// Active rules:\n";

    for (auto s : activeRules)
    {
        file << s << '\n';
    }

    file.close();
    return;
}

////////////////////////////////////////////////////////////////

// Get all candidates which match exactly.
vector<int> getStageOneCandidates(const vector<vector<Type>> &candArgs, const vector<Type> &argTypes)
{
    vector<int> out;
    bool isMatch;

    // Check for exact match for each candidate
    for (int j = 0; j < candArgs.size(); j++)
    {
        if (candArgs[j].size() != argTypes.size())
        {
            // Cannot match if number of arguments is different
            continue;
        }

        isMatch = true;

        // Iterate over argument types
        for (int k = 0; k < candArgs[j].size(); k++)
        {
            if (!typesAreSameExact(&candArgs[j][k], &argTypes[k]))
            {
                isMatch = false;
                break;
            }
        }

        if (isMatch)
        {
            out.push_back(j);
        }
    }

    return out;
}

// Get all candidates which match with implicit casting.
vector<int> getStageTwoCandidates(const vector<vector<Type>> &candArgs, const vector<Type> &argTypes)
{
    vector<int> out;
    bool isMatch;

    // Min number of conversions
    int min = argTypes.size();
    int iOfMin = 0;
    int cur;

    // Check for exact match for each candidate
    for (int j = 0; j < candArgs.size(); j++)
    {
        if (candArgs[j].size() != argTypes.size())
        {
            // Cannot match if number of arguments is different
            continue;
        }

        isMatch = true;

        // Iterate over argument types
        for (int k = 0; k < candArgs[j].size(); k++)
        {
            cur = 0;

            // Inexact match, rather than exact in the last one
            // Special case; Internal implicit casting
            if (candArgs[j][k].size() == 1 && argTypes[k].size() == 1)
            {
                string a, b;
                a = candArgs[j][k][0].name;
                b = argTypes[k][0].name;

                // Special case 1: Implicit int casting
                if (a == "u8" || a == "i8" || a == "u16" || a == "i16" || a == "u32" || a == "i32" || a == "u64" ||
                    a == "i64" || a == "u128" || a == "i128")
                {
                    if (b == "u8" || b == "i8" || b == "u16" || b == "i16" || b == "u32" || b == "i32" || b == "u64" ||
                        b == "i64" || b == "u128" || b == "i128")
                    {
                        cur++;
                        continue;
                    }
                }

                // Special case 2: Implicit float casting
                else if (a == "f32" || a == "f64" || a == "f128")
                {
                    if (b == "f32" || b == "f64" || b == "f128")
                    {
                        cur++;
                        continue;
                    }
                }
            }

            isMatch = false;
            break;
        }

        if (isMatch)
        {
            if (cur < min)
            {
                min = cur;
                iOfMin = j;
            }

            out.push_back(j);
        }
    }

    // Make the occurrence with the fewest casts first
    if (iOfMin != 0)
    {
        for (int i = 0; i < out.size(); i++)
        {
            if (i == iOfMin)
            {
                auto temp = out[i];
                out[i] = out[iOfMin];
                out[iOfMin] = temp;
                break;
            }
        }
    }

    return out;
}

// Get all candidates which match with auto referencing and/or
// dereferencing.
vector<int> getStageThreeCandidates(const vector<vector<Type>> &candArgs, const vector<Type> &argTypes)
{
    vector<int> out;
    bool isMatch;
    int min = 999, iOfMin = 0, cur;

    // Check for exact match for each candidate
    for (int j = 0; j < candArgs.size(); j++)
    {
        if (candArgs[j].size() != argTypes.size())
        {
            // Cannot match if number of arguments is different
            continue;
        }

        isMatch = true;

        // Iterate over argument types
        cur = 0;
        for (int k = 0; k < candArgs[j].size(); k++)
        {
            // Inexact match, rather than exact in the last one
            if (!typesAreSame(&candArgs[j][k], &argTypes[k], cur))
            {
                isMatch = false;
                break;
            }
        }

        if (isMatch)
        {
            out.push_back(j);

            if (cur < min)
            {
                min = cur;
                iOfMin = j;
            }
        }
    }

    // Make the occurrence with the fewest casts first
    if (iOfMin != 0)
    {
        for (int i = 0; i < out.size(); i++)
        {
            if (i == iOfMin)
            {
                auto temp = out[i];
                out[i] = out[iOfMin];
                out[iOfMin] = temp;
                break;
            }
        }
    }

    return out;
}

// Prints the reason why each candidate was rejected
void printCandidateErrors(const vector<__multiTableSymbol> &candidates, const vector<Type> &argTypes,
                          const string &name)
{
    cout << "--------------------------------------------------\n";

    vector<vector<Type>> candArgs;
    for (auto item : candidates)
    {
        Type curType = item.type;
        while (curType != nullType && curType[0].info == pointer)
        {
            curType.pop_front();
        }

        auto args = getArgs(item.type);
        candArgs.push_back(vector<Type>());

        for (auto arg : args)
        {
            if (arg.second == nullType)
            {
                continue;
            }

            candArgs.back().push_back(arg.second);
        }
    }

    auto s1 = getStageOneCandidates(candArgs, argTypes);
    auto s2 = getStageTwoCandidates(candArgs, argTypes);
    auto s3 = getStageThreeCandidates(candArgs, argTypes);

    cout << "Passed s1: " << s1.size() << '\n'
         << "Passed s2: " << s2.size() << '\n'
         << "Passed s3: " << s3.size() << "\n\n";

    for (int i = 0; i < candidates.size(); i++)
    {
        cout << left << name << setw(30) << toStr(&candidates[i].type) << "\tFrom " << candidates[i].sourceFilePath
             << '\n';

        if (candidates[i].erased)
        {
            // Was erased
            cout << "\tWas erased.\n";
        }
        else
        {
            // Stage 1
            if (find(s1.begin(), s1.end(), i) == s1.end())
            {
                cout << "\tFailed exact match.\n";
            }
            else
            {
                cout << tags::green << "\tPassed exact match.\n" << tags::reset;
            }

            // Stage 2
            if (find(s2.begin(), s2.end(), i) == s2.end())
            {
                cout << "\tFailed casting match.\n";
            }
            else
            {
                cout << tags::green << "\tPassed casting match.\n" << tags::reset;
            }

            // Stage 3
            if (find(s3.begin(), s3.end(), i) == s3.end())
            {
                cout << "\tFailed ref/deref match.\n";
            }
            else
            {
                cout << tags::green << "\tPassed ref/deref match.\n" << tags::reset;
            }
        }

        cout << "\tArgs:\n";
        for (auto arg : candArgs[i])
        {
            cout << "\t\t" << toStr(&arg) << '\n';
        }
    }

    cout << "--------------------------------------------------\n"
         << "Provided argument types:\n";

    for (auto arg : argTypes)
    {
        cout << '\t' << toStr(&arg) << '\n';
    }

    cout << "--------------------------------------------------\n\n";

    return;
}

string cleanMacroArgument(const string &from)
{
    sm_assert(from.size() > 1 && (from.front() == '"' && from.back() == '"') ||
                  (from.front() == '\'' && from.back() == '\''),
              "Internal macro arguments must be string-enclosed.");

    string out;
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
void destroyUnits(const string &name, const Type &type, const bool &doThrow)
{
    // Safety check
    if (table.count(name) != 0 && table[name].size() != 0)
    {
        // Iterate over items
        for (int i = 0; i < table[name].size(); i++)
        {
            if (table[name][i].type == type)
            {
                // If is unit, erase
                if (table[name][i].seq.items.size() == 0 ||
                    (table[name][i].seq.items.size() >= 1 && table[name][i].seq.items[0].raw.size() > 8 &&
                     table[name][i].seq.items[0].raw.substr(0, 9) == "//AUTOGEN"))
                {
                    table.at(name).erase(table[name].begin() + i);
                    i--;
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
void debugPrint(const sequence &What, int spaces, ostream &to)
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
