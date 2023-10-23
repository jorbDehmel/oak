/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "reconstruct.hpp"

// Removes illegal characters
string purifyStr(const string &What)
{
    const string illegalChars = "<>(){}[]\\'\"`~!@#$%^&*-=+|?,;:";
    string out = What;

    // Trim filepath
    while (out.find("/") != string::npos)
    {
        out.erase(0, out.find("/") + 1);
    }

    // Trim file extension(s)
    while (out.find(".") != string::npos)
    {
        out.erase(out.find("."));
    }

    // Replace other illegal characters
    for (int i = 0; i < out.size(); i++)
    {
        if (illegalChars.find(out[i]) != string::npos)
        {
            out[i] = '_';
        }
    }

    return (out == "" ? "NULL_STR" : out);
}

pair<string, string> reconstructAndSave(const string &Name)
{
    stringstream header, body;
    reconstruct(Name, header, body);
    return save(header, body, Name);
}

void reconstruct(const string &Name,
                 stringstream &header,
                 stringstream &body)
{
    // Purify name
    string rootName;
    if (Name.substr(Name.size() - 4) == ".oak")
    {
        rootName = Name.substr(0, Name.size() - 4);
    }
    else
    {
        rootName = Name;
    }

    string name = purifyStr(rootName) + "_HPP";

    for (int i = 0; i < name.size(); i++)
    {
        name[i] = toupper(name[i]);
    }

    // Begin body
    string cleanedName = purifyStr(rootName);
    body << "#include \"" << cleanedName << ".hpp\"\n";

    // Begin header enclosure
    header << "#ifndef " << name << "\n"
           << "#define " << name << "\n\n";

    // Step A1: Load Oak standard translational header
    header << "#include \"" << OAK_HEADER_PATH << "\"\n\n";

    // Step A2: Dependencies
    {
        header << "// Included files\n";
        for (auto d : deps)
        {
            header << "#include \"" << d << "\"\n";
        }
        header << '\n';
    }

    // Step A3: Struct definitions
    {
        header << "// Struct definitions\n";
        for (auto name : structOrder)
        {

            if (enumData.count(name) != 0)
            {
                continue;
            }

            header << "struct " << name << "\n{\n";

            for (auto m : structData[name].order)
            {
                header << '\t' << toStrC(&structData[name].members[m]) << ' ' << m << ";\n";
            }

            header << "};\n\n";
        }
    }

    // Step A4: Enumeration definitions
    {
        header << "// Enumeration definitions\n";
        for (auto name : enumOrder)
        {
            header << enumToC(name) << '\n';
        }
    }

    // Step A5: Insert global definitions into header
    // (Translate Oak syntax into C++ syntax)
    {
        header << "// Global function definitions\n";

        for (auto entry : table)
        {
            string name = entry.first;

            // Trim stuff for method definitions
            if (name.find(".") != string::npos)
            {
                name = name.substr(name.find(".") + 1);
            }

            for (__multiTableSymbol s : entry.second)
            {
                header << "// Source file: " << s.sourceFilePath << '\n';

                if (s.type[0].info == function)
                {
                    string toAdd = toStrCFunction(&s.type, name);

                    header << toAdd << ";\n";

                    if (s.seq.items.size() != 0)
                    {
                        string definition = toC(s.seq);

                        body << toAdd << "\n"
                             << (definition == "" ? ";" : definition);
                    }
                }
                else
                {
                    string toAdd = toStrC(&s.type);

                    header << "extern " << toAdd << " " << name << ";\n";
                    body << toAdd << " " << name << ";\n";
                }

                header << '\n';
            }
        }
    }

    // End header enclosure
    header << "\n#endif\n";

    return;
}

// Save reconstructed files and return compilation command
pair<string, string> save(const stringstream &header, const stringstream &body, const string &Name)
{
    string rootName, headerName, bodyName;

    if (Name.substr(Name.size() - 4) == ".oak")
    {
        rootName = Name.substr(0, Name.size() - 4);
    }
    else
    {
        rootName = Name;
    }

    rootName = purifyStr(rootName);

    smartSystem("mkdir -p .oak_build");

    headerName = ".oak_build/" + rootName + ".hpp";
    bodyName = ".oak_build/" + rootName + ".cpp";

    // Save header
    {
        ofstream headerFile(headerName);
        if (!headerFile.is_open())
        {
            throw runtime_error("Failed to open file `" + headerName + "`");
        }

        headerFile << header.str();

        headerFile.close();
    }

    // Save body
    {
        ofstream bodyFile(bodyName);
        if (!bodyFile.is_open())
        {
            throw runtime_error("Failed to open file `" + bodyName + "`");
        }

        bodyFile << body.str();

        bodyFile.close();
    }

    return make_pair(headerName, bodyName);
}

// This is separate due to complexity
string toStrCFunction(Type *What, const string &Name, const unsigned int &pos)
{
    parse_assert(What != nullptr && What->size() > 0 && (*What)[0].info == function);

    if (pos >= What->size())
    {
        return "";
    }

    // Second section, between function and maps, will be arguments.
    string arguments = "";
    int i = pos + 1;
    for (; i < What->size(); i++)
    {
        // (a: *const map<string, int>, b: u32)
        // (const map<string, int> *a, u32 b)

        // Var names can only occur in non-fn-ptr types
        if (What->operator[](i).info == var_name)
        {
            string name = What->operator[](i).name;

            i++;

            if (i >= What->size())
            {
                break;
            }

            Type temp;
            while (i < What->size() && !(What->operator[](i).info == join || What->operator[](i).info == maps))
            {
                temp.append(What->operator[](i).info, What->operator[](i).name);
                i++;
            }
            i--;

            if (arguments != "")
            {
                arguments += ", ";
            }

            arguments += toStrC(&temp, name);
        }

        else if (What->operator[](i).info == maps)
        {
            break;
        }
    }

    if (i >= What->size() || What->operator[](i).info != maps)
    {
        cout << "Function '" << toStr(What) << "' has no return type.\n";
        parse_assert(i < What->size());
    }

    i++;

    // Jump past any remaining maps (idk why this is necessary)
    for (int j = What->size() - 1; j > i; j--)
    {
        if ((*What)[j].info == maps)
        {
            i = j;
            break;
        }
    }

    // Next, after maps, we will have the return type.
    string returnType = toStrC(What, "", i);

    if (returnType == "")
    {
        returnType = "void";
    }

    // Assemble and return: SEMICOLON IS HANDLED ELSEWHERE
    string out;
    out = returnType + " " + Name + "(" + arguments + ")";

    return out;
}

string toStrCFunctionRef(Type *What, const string &Name, const unsigned int &pos)
{
    // pointer -> function -> ARGS -> maps -> RETURN_TYPE
    // RETURN_TYPE (*Name)(ARGS);

    parse_assert(Name != "");
    parse_assert(What != nullptr);
    parse_assert((*What)[0].info == pointer);
    parse_assert(What->size() > 1);
    parse_assert((*What)[1].info == function);

    if (pos >= What->size())
    {
        return "";
    }

    string returnType = "";
    string arguments = "";
    Type argType;
    int count = 0;

    int i = pos + 1;

    // Then some number of args, possibly containing maps
    do
    {
        typeNode &cur = (*What)[i];

        if (cur.info == function)
        {
            count++;
        }
        else if (cur.info == maps)
        {
            count--;
        }
        else if (cur.info == var_name)
        {
            i++;
            continue;
        }
        else if (cur.info == join)
        {
            // Append to arguments string
            if (arguments != "")
            {
                arguments += ", ";
            }

            arguments += toStrC(&argType);

            // Erase current argType
            argType = nullType;
        }
        else
        {
            // Type append to argType
            argType.append(cur.info, cur.name);
        }

        i++;
    } while (i < What->size() && count != 0);

    // Final append
    if (argType != nullType)
    {
        if (arguments != "")
        {
            arguments += ", ";
        }

        arguments += toStrC(&argType);
    }

    // Then the maps corresponding to the opening function
    // (this is skipped past in the loop)

    // Then the return type is the remainder
    if (i >= What->size())
    {
        returnType = "void";
    }
    else
    {
        returnType = toStrC(What, "", i);
    }

    // Reconstruct out of partial strings
    string out = returnType + " (*" + Name + ")(" + arguments + ")";

    return out;
}

map<unsigned long long, string> toStrCTypeCache;
string toStrC(Type *What, const string &Name, const unsigned int &pos)
{
    if (toStrCTypeCache.count(What->ID) != 0)
    {
        return toStrCTypeCache[What->ID];
    }

    string out = "";

    // Safety check
    if (What == nullptr || What->size() == 0 || pos >= What->size())
    {
        if (toStrCTypeCache.size() > 1000)
        {
            toStrCTypeCache.clear();
        }

        toStrCTypeCache[What->ID] = "";
        return "";
    }

    if ((*What)[pos].info == pointer && What->size() > 1 && (*What)[pos + 1].info == function)
    {
        if (toStrCTypeCache.size() > 1000)
        {
            toStrCTypeCache.clear();
        }

        out = toStrCFunctionRef(What, Name);
        toStrCTypeCache[What->ID] = out;
        return out;
    }

    switch ((*What)[pos].info)
    {
    case atomic:
        out += (*What)[pos].name;
        out += toStrC(What, "", pos + 1);
        break;
    case join:
        out += ",";
        out += toStrC(What, "", pos + 1);
        break;
    case pointer:
        out += toStrC(What, "", pos + 1);
        out += "*";
        break;
    case var_name:
        // Argument in function: Will be FOLLOWED by its type in Oak
        // hello: *map<string, int>;
        // map<string, int> *hello;
        out += toStrC(What, "", pos + 1);
        out += (*What)[pos].name;
        break;

    case function:
    case maps:
        out += toStrC(What, "", pos + 1);
        break;

    default:
        throw parse_error("Unforeseen info enum option for Type object.");
    }

    if (Name != "")
    {
        out += " " + Name;
    }

    if (toStrCTypeCache.size() > 1000)
    {
        toStrCTypeCache.clear();
    }
    toStrCTypeCache[What->ID] = out;

    return out;
}

// Dump data to file
void dump(const vector<string> &Lexed, const string &Where, const string &FileName,
          const int &Line, const sequence &FileSeq, const vector<string> LexedBackup)
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

    file << sep
         << "Dump file for " << FileName << " generated by Acorn at " << ctime(&curTime);

    file << sep
         << "// Pre-everything lexed:\n";

    for (auto s : LexedBackup)
    {
        if (s.size() >= 2 && s.substr(0, 2) == "//")
        {
            file << "\n"
                 << s << "\t|\t";
        }
        else
        {
            file << s << ' ';
        }
    }
    file << '\n';

    file << sep
         << "// Post-substitution lexed:\n";

    for (auto s : Lexed)
    {
        if (s.size() >= 2 && s.substr(0, 2) == "//")
        {
            file << "\n"
                 << s << "\t|\t";
        }
        else
        {
            file << s << ' ';
        }
    }
    file << '\n';

    file << sep
         << "// Symbols and their types:\n";

    for (auto p : table)
    {
        file << p.first << ":\n";

        for (auto item : p.second)
        {
            file << '\t' << toStr(&item.type) << '\n';
        }
    }

    file << sep
         << "// Structs:\n";

    for (auto s : structData)
    {
        file << s.first << "\n";

        for (auto m : s.second.members)
        {
            file << '\t' << m.first << '\t' << toStr(&m.second) << '\n';
        }
    }

    file << sep
         << "// Enums:\n";

    for (auto e : enumData)
    {
        file << e.first << "\n";

        for (auto m : e.second.options)
        {
            file << '\t' << m.first << '\t' << toStr(&m.second) << '\n';
        }
    }

    file << sep
         << "// Generics:\n";

    printGenericDumpInfo(file);

    file
        << sep
        << "// Full anatomy:\n";

    debugPrint(FileSeq, 0, file);

    file << sep
         << "// All rules:\n";

    for (auto s : rules)
    {
        file << s.first << '\n'
             << '\t';

        for (auto t : s.second.inputPattern)
        {
            file << t << ' ';
        }

        file << "\n\t";

        for (auto t : s.second.outputPattern)
        {
            file << t << ' ';
        }

        file << "\n";
    }

    file << sep
         << "// All bundles:\n";

    for (auto p : bundles)
    {
        file << p.first << "\n\t";

        for (auto r : p.second)
        {
            file << r << ' ';
        }

        file << '\n';
    }

    file << sep
         << "// Active rules:\n";

    for (auto s : activeRules)
    {
        file << s << '\n';
    }

    file.close();
    return;
}

map<string, string> toStrCEnumCache;
string enumToC(const string &name)
{
    if (toStrCEnumCache.count(name) != 0)
    {
        return toStrCEnumCache[name];
    }

    // Basic error checking; Should NOT constitute the entirety
    // of safety checks!!! This just ensures a lack of internal
    // errors.
    if (name == "" || enumData.count(name) == 0)
    {
        throw runtime_error(string(__FILE__) + ":" + to_string(__LINE__) + " Error in enumeration toC.");
    }

    __enumLookupData &cur = enumData[name];

    string out = "struct " + name + " {\nenum {\n";

    // names
    for (auto name : cur.order)
    {
        out += name + ",\n";
    }

    out += "\n} __info;\nunion {\n";

    // types
    for (auto name : cur.order)
    {
        out += toStrC(&cur.options[name]) + " " + name + "_data" + ";\n";
    }

    out += "\n} __data;\n};\n";

    // That's the end of the easy part.
    // Now, it must generate constructors.

    string enumTypeStr = name;
    for (auto optionName : cur.order)
    {
        string optionTypeStr = toStrC(&cur.options[optionName]);

        if (cur.options[optionName][0].info == atomic && cur.options[optionName][0].name == "unit")
        {
            // Unit struct; Single argument constructor

            // Generate C version
            out += "void wrap_" + optionName + "(" + enumTypeStr + " *self)\n{\n";
            out += "self->__info = " + enumTypeStr + "::" + optionName + ";\n}\n";
        }
        else
        {
            // Double argument constructor

            // Generate C version
            out += "void wrap_" + optionName + "(" + enumTypeStr + " *self, ";
            out += optionTypeStr + " data)\n";
            out += "{\n";
            out += "self->__info = " + enumTypeStr + "::" + optionName + ";\n";
            out += "self->__data." + optionName + "_data = data;\n";
            out += "}\n";
        }
    }

    if (toStrCEnumCache.size() > 1000)
    {
        toStrCEnumCache.clear();
    }
    toStrCEnumCache[name] = out;

    return out;
}
