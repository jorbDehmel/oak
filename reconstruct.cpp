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
           << "#define " << name << "\n";

    // Step A1: Load Oak standard translational header
    {
        ifstream translationHeader(OAK_HEADER_PATH);
        if (!translationHeader.is_open())
        {
            cout << "\nError loading standard Oak translational header;\n"
                 << "This is most likely an issue with your Oak install.\n\n";

            throw runtime_error("Failed to open file `" OAK_HEADER_PATH "`");
        }

        string line;
        while (getline(translationHeader, line))
        {
            header << line << '\n';
        }

        translationHeader.close();
    }

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
        for (auto s : structData)
        {
            if (enumData.count(s.first) != 0)
            {
                continue;
            }

            header << "struct " << s.first << "\n{\n";

            for (auto m : s.second.order)
            {
                header << '\t' << toStrC(&s.second.members[m]) << ' ' << m << ";\n";
            }

            header << "};\n\n";
        }
    }

    // Step A4: Enumeration definitions
    {
        header << "// Enumeration definitions\n";
        for (auto e : enumData)
        {
            header << enumToC(e.first) << '\n';
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
                if (s.type.info == function)
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
string toStrCFunction(Type *What, const string &Name)
{
    // function -> ARGS -> maps -> RETURN_TYPE

    parse_assert(What != nullptr && What->info == function);
    Type *current = What->next;

    // Second section, between function and maps, will be arguments.
    string arguments = "";
    for (; current != nullptr; current = current->next)
    {
        // (a: *const map<string, int>, b: u32)
        // (const map<string, int> *a, u32 b)

        // Var names can only occur in non-fn-ptr types
        if (current->info == var_name)
        {
            string name = current->name;
            current = current->next;

            Type temp;

            Type *prev = &temp;
            while (current != nullptr && !(current->info == join || current->info == maps))
            {
                temp.append(current->info, current->name);

                prev = current;
                current = current->next;
            }
            current = prev;

            if (arguments != "")
            {
                arguments += ", ";
            }

            arguments += toStrC(&temp, name);
        }

        else if (current->info == maps)
        {
            break;
        }
    }

    if (current == nullptr || current->info != maps)
    {
        cout << "Function '" << arguments << "' has no return type.\n";
        parse_assert(current != nullptr);
    }

    current = current->next;

    // Jump past any remaining maps (idk why this is necessary)
    Type *toUse = current;
    for (auto temp = current; temp != nullptr; temp = temp->next)
    {
        if (temp->info == maps)
        {
            toUse = temp;
        }
    }

    // Next, after maps, we will have the return type.
    string returnType = toStrC(toUse);

    if (returnType == "")
    {
        returnType = "void";
    }

    // Assemble and return: SEMICOLON IS HANDLED ELSEWHERE
    string out;
    out = returnType + " " + Name + "(" + arguments + ")";

    return out;
}

string toStrCFunctionRef(Type *What, const string &Name)
{
    // pointer -> function -> ARGS -> maps -> RETURN_TYPE
    // RETURN_TYPE (*Name)(ARGS);

    parse_assert(Name != "");
    parse_assert(What != nullptr);
    parse_assert(What->info == pointer);
    parse_assert(What->next != nullptr);
    parse_assert(What->next->info == function);

    string returnType = "";
    string arguments = "";
    Type *cur = What->next;
    Type argType;
    int count = 0;

    // Then some number of args, possibly containing maps
    do
    {
        if (cur->info == function)
        {
            count++;
        }
        else if (cur->info == maps)
        {
            count--;
        }
        else if (cur->info == var_name)
        {
            cur = cur->next;
            continue;
        }
        else if (cur->info == join)
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
            argType.append(cur->info, cur->name);
        }

        cur = cur->next;
    } while (cur != nullptr && count != 0);

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
    if (cur == nullptr)
    {
        returnType = "void";
    }
    else
    {
        returnType = toStrC(cur);
    }

    // Reconstruct out of partial strings
    string out = returnType + " (*" + Name + ")(" + arguments + ")";

    return out;
}

string toStrC(Type *What, const string &Name)
{
    string out = "";

    // Safety check
    if (What == nullptr)
    {
        return out;
    }

    // cout << __FILE__ << ":" << __LINE__ << " calling '" << toStr(What) << "'\n";

    if (What->info == pointer &&
        What->next != nullptr &&
        What->next->info == function)
    {
        return toStrCFunctionRef(What, Name);
    }

    switch (What->info)
    {
    case atomic:
        out += What->name;
        out += toStrC(What->next);
        break;
    case join:
        out += ",";
        out += toStrC(What->next);
        break;
    case modifier:
        out += What->name + " ";
        out += toStrC(What->next);
        break;
    case pointer:
        out += toStrC(What->next);
        out += "*";
        break;
    case var_name:
        // Argument in function: Will be FOLLOWED by its type in Oak
        // hello: *map<string, int>;
        // map<string, int> *hello;
        out += toStrC(What->next);
        out += What->name;
        break;

    case function:
    case maps:
        out += toStrC(What->next);
        break;

    default:
        throw parse_error("Unforeseen info enum option for Type object.");
    }

    if (Name != "")
    {
        out += " " + Name;
    }

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
         << "// Full definitions:\n";

    file << "\n// Symbols\n";

    for (auto p : table)
    {
        bool hasPrinted = false;

        for (auto item : p.second)
        {
            if (item.seq.items.size() != 0)
            {
                if (!hasPrinted)
                {
                    file << p.first << ":\n";
                    hasPrinted = true;
                }

                file << '\t' << toStr(&item.type) << '\n'
                     << "\t\t" << toC(item.seq) << '\n';
            }
        }
    }

    file << "\n// Generic functions:\n";

    for (auto p : templTable)
    {
        file << p.first << ":\n";

        for (auto item : p.second)
        {
            file << "\t(";
            for (auto gen : item.generics)
            {
                file << gen << " ";
            }
            file << ") ";

            for (auto s : item.returnType)
            {
                file << s << ' ';
            }
            file << '\n';

            if (item.guts.size() != 0)
            {
                file << "\t\t";

                for (auto s : item.guts)
                {
                    file << s << ' ';
                }
                file << '\n';
            }
        }
    }

    file << "\n// Generic structs:\n";

    for (auto p : templStructData)
    {
        file << p.first << "<";
        for (auto g : p.second.generics)
        {
            file << g << ", ";
        }
        file << ">:\n\t";

        for (auto item : p.second.guts)
        {
            file << item << ' ';
        }
    }

    file << "\n// Generic enums:\n";

    for (auto p : templEnumData)
    {
        file << p.first << "<";
        for (auto g : p.second.generics)
        {
            file << g << ", ";
        }
        file << ">:\n\t";

        for (auto item : p.second.guts)
        {
            file << item << ' ';
        }
    }

    file << sep
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

string enumToC(const string &name)
{
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

        if (cur.options[optionName].info == atomic && cur.options[optionName].name == "unit")
        {
            // Unit struct; Single argument constructor

            // Generate C version
            out += "void init_" + optionName + "(" + enumTypeStr + " *self)\n{\n";
            out += "self->__info = " + enumTypeStr + "::" + optionName + ";\n}\n";

            // Insert Oak version
            Type constructorType = nullType;
            constructorType.append(function);
            constructorType.append(var_name, "self");
            constructorType.append(pointer);
            constructorType.append(atomic, enumTypeStr);
            constructorType.append(maps);
            constructorType.append(atomic, "void");

            table["init_" + optionName].push_back(__multiTableSymbol{sequence{}, constructorType, false});
        }
        else
        {
            // Double argument constructor

            // Generate C version
            out += "void init_" + optionName + "(" + enumTypeStr + " *self, ";
            out += optionTypeStr + " data)\n";
            out += "{\n";
            out += "self->__info = " + enumTypeStr + "::" + optionName + ";\n";
            out += "self->__data." + optionName + "_data = data;\n";
            out += "}\n";

            // Insert Oak version
            Type constructorType = nullType;
            constructorType.append(function);
            constructorType.append(var_name, "self");
            constructorType.append(pointer);
            constructorType.append(atomic, enumTypeStr);
            constructorType.append(join);
            constructorType.append(var_name, "data");
            constructorType.append(atomic, optionTypeStr);
            constructorType.append(maps);
            constructorType.append(atomic, "void");

            table["init_" + optionName].push_back(__multiTableSymbol{sequence{}, constructorType, false});
        }
    }

    return out;
}
