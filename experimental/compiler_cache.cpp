#include "compiler_cache.hpp"

/*
// File outline:

UNIX_TIMESTAMP_LL
number_visited_files
    file1 file2 file3 etc
num_structs
    struct_name
    is_erased
    num_members
        member1 member1_type
num_enums
    enum_name
    num_members
        member1 member1_type
num_generics
    generic_name
    generic_type_vec
    """generic_guts"""
    """generic_needs"""
num_symbols
    symbol_name
    is_erased
    symbol_signature
    """symbol_c_code"""
num_macros
    macro_name
    """macro_guts"""
num_cpp_sources
    cpp_source1
    etc
num_packages
    package1
    etc
num_dialect_rules
    rule1inp
    rule1out
    etc
dialect_lock
num_rules
    rule1name
    rule1inp
    rule1out
    etc
space separated compilation flags
*/

// Loads a ```-enclosed code chunk into a single string from
// the given input stream
string loadCodeChunk(ifstream &stream)
{
    // First, must loak ```
    string token, text;

    stream >> token;
    assert(token == "```");

    // Now, load guts
    do
    {
        stream >> token;

        if (token != "```")
        {
            text.append(token);
            text.push_back(' ');
        }
    } while (token != "```");

    // Finally, load ```

    return text;
}

// Save the entire state of the compiler to a file
void saveCompilerCache(const string &where)
{
    // Open file
    ofstream file(where);
    if (!file.is_open())
    {
        throw cache_error("Failed to open compiler cache file " + where);
    }

    // Save metadata and visited files
    file << (long long)time(NULL) << ' ' << visitedFiles.size() << '\n';

    for (const string &filepath : visitedFiles)
    {
        file << filepath << ' ';
    }

    // Save structs
    file << structData.size() << '\n';

    for (auto &s : structData)
    {
        file << s.first << "\n "
             << (s.second.erased ? "true" : "false") << "\n "
             << s.second.order.size() << "\n";

        for (const auto &item : s.second.order)
        {
            file << item << ' ' << s.second.members[item] << '\n';
        }
    }

    // Save enums
    file << enumData.size() << '\n';

    for (auto &s : enumData)
    {
        file << s.first << "\n"
             << (s.second.erased ? "true" : "false") << "\n"
             << s.second.order.size() << "\n";

        for (const auto &item : s.second.order)
        {
            file << item << ' ' << s.second.options[item] << '\n';
        }
    }

    // Save generics
    file << generics.size() << '\n';

    for (auto &s : generics)
    {
        for (auto &instance : s.second)
        {
            // name
            file << s.first << '\n';

            // generics
            file << instance.genericNames << '\n';

            // type vec
            file << instance.typeVec << '\n';

            // guts
            file << instance.symbols << '\n';

            // needs block
            file << instance.needsBlock << '\n';

            // instances
            file << instance.instances << '\n';
        }
    }

    // Save symbols
    file << table.size() << '\n';

    for (auto &s : table)
    {
        file << s.second.size() << ' ';
        for (auto &instance : s.second)
        {
            file << s.first << ' ' << (instance.erased ? "true" : "false") << "\n"
                 << instance.type << "\n``` "
                 << toC(instance.seq) << " ```\n";
        }
    }

    // Save macros
    file << macros.size() << '\n';
    for (const auto &s : macros)
    {
        file << s.first << "\n``` "
             << s.second << " ```\n";
    }

    // Save cpp sources
    file << cppSources << '\n';

    // Save packages
    file << packages.size() << ' ';
    for (auto &s : packages)
    {
        file << s.first << ' ';
    }
    file << '\n';

    // Save dialect rules
    file << dialectRules << '\n';

    // Dialect lock
    file << (dialectLock ? "true" : "false") << '\n';

    // Save regular rules
    file << rules.size() << '\n';
    for (auto rule : rules)
    {
        file << rule.first << ' ' << rule.second.inputPattern << ' ' << rule.second.outputPattern << '\n';
    }

    // Save compilation flags
    file << cflags << '\n';

    // Save object files
    file << objects << '\n';

    // Close file
    file.close();
    return;
}

// Load the entire state of the compiler from a file
void loadCompilerCache(const string &where)
{
    // Open file
    ifstream file(where);
    if (!file.is_open())
    {
        throw cache_error("Failed to open compiler cache file " + where);
    }

    // Read all data
    long long cacheTime = 0;
    size_t size;
    string temp;

    // Save metadata and visited files
    file >> cacheTime >> size;

    for (int i = 0; i < size; i++)
    {
        file >> temp;
        visitedFiles.insert(temp);
    }

    // Save structs
    file >> size;
    __structLookupData structTemp;
    string name, subname;
    size_t subsize;
    for (int i = 0; i < size; i++)
    {
        file >> name >> temp >> subsize;
        structTemp.erased = (temp == "true");

        structTemp.order.clear();
        structTemp.members.clear();

        for (int j = 0; j < subsize; j++)
        {
            file >> subname >> structTemp.members[subname];
            structTemp.order.push_back(subname);
        }

        structData[name] = structTemp;
    }

    // Save enums
    file >> size;

    __enumLookupData enumTemp;

    for (int i = 0; i < size; i++)
    {
        file >> name >> temp >> subsize;
        enumTemp.erased = (temp == "true");

        enumTemp.order.clear();
        enumTemp.options.clear();

        for (int j = 0; j < subsize; j++)
        {
            file >> subname >> enumTemp.options[subname];
            enumTemp.order.push_back(subname);
        }
    }

    // Save generics
    file >> size;

    for (int i = 0; i < size; i++)
    {
        // name
        file >> name;
        generics[name].push_back(genericInfo{});

        // generics
        file >> generics[name].back().genericNames;

        // type vec
        file >> generics[name].back().typeVec;

        // guts
        file >> generics[name].back().symbols;

        // needs block
        file >> generics[name].back().needsBlock;

        // instances
        file >> generics[name].back().instances;
    }

    // Save symbols
    file >> size;
    for (int i = 0; i < size; i++)
    {
        file >> subsize;
        for (int j = 0; j < subsize; j++)
        {
            file >> name >> temp;
            table[name].push_back(__multiTableSymbol{});

            table[name].back().erased = (temp == "true");
            file >> table[name].back().type;

            table[name].back().seq.info = code_line;
            table[name].back().seq.raw = loadCodeChunk(file);
        }
    }

    // Save macros
    file >> size;
    for (int i = 0; i < size; i++)
    {
        file >> name;
        temp = loadCodeChunk(file);
        macros[name] = temp;
    }

    // Save cpp sources
    file >> cppSources;

    // Save packages
    file >> size;
    for (int i = 0; i < size; i++)
    {
        file >> temp;
        packages[temp] = loadPackageInfo(temp);
    }

    // Save dialect rules
    file >> dialectRules;

    // Dialect lock
    file >> temp;
    dialectLock = (temp == "true");

    // Save regular rules
    rule ruleTemp;
    file >> size;
    for (int i = 0; i < size; i++)
    {
        file >> name >> ruleTemp.inputPattern >> ruleTemp.outputPattern;
        rules[name] = ruleTemp;
    }

    // Save compilation flags
    file >> cflags;

    // Save object files
    file >> objects;

    // Close file
    file.close();
    return;
}

ostream &operator<<(ostream &strm, Type &type)
{
    strm << type.size() << ' ';

    for (int i = 0; i < type.size(); i++)
    {
        typeNode &to_write = type[i];

        switch (to_write.info)
        {
        case pointer:
            strm << "PTR";
            break;
        case atomic:
            strm << to_write.name;
            break;
        case join:
            strm << "JOIN";
            break;
        case function:
            strm << "FN";
            break;
        case maps:
            strm << "MAPS";
            break;
        default:
            break;
        }

        if (i + 1 < type.size())
        {
            strm << ' ';
        }
    }

    return strm;
}

istream &operator>>(istream &strm, Type &type)
{
    size_t size = 0;
    type.internal.clear();

    strm >> size;
    type.internal.reserve(size);

    string tempStr;
    typeNode tempNode;

    for (int i = 0; i < size; i++)
    {
        strm >> tempStr;
        tempNode.name = "";

        if (tempStr == "PTR")
        {
            tempNode.info = pointer;
        }

        else if (tempStr == "JOIN")
        {
            tempNode.info = join;
        }

        else if (tempStr == "FN")
        {
            tempNode.info = function;
        }

        else if (tempStr == "MAPS")
        {
            tempNode.info = maps;
        }

        else
        {
            tempNode.info = atomic;
            tempNode.name = tempStr;
        }

        type.internal.push_back(tempNode);
    }

    return strm;
}
