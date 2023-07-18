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
            if (!s.second.templatedTypes.empty())
            {
                // Templated! Must complete full definition here

                // Insert template definition line
                header << "template <";

                int i = 0;
                for (auto t : s.second.templatedTypes)
                {
                    header << "class " << t;

                    if (i + 1 != s.second.templatedTypes.size())
                    {
                        header << ", ";
                    }

                    i++;
                }

                header << ">\n";

                header << "struct " << s.first;

                if (s.second.type.name == "live")
                {
                    header << " : public __live_base";
                }
                else if (s.second.type.name != "NULL" && s.second.type.name != "struct")
                {
                    header << " : public " << toStrC(&s.second.type);
                }

                header << "\n{\n";

                for (auto m : s.second.members)
                {
                    header << "\t" << toStrC(&m.second) << " " << m.first << ";\n";
                }

                header << "};\n\n";
            }
            else
            {
                // Non-templated struct
                header << "struct " << s.first;

                if (s.second.type.name != "NULL" && s.second.type.name != "struct")
                {
                    header << " : public " << toStrC(&s.second.type);
                }

                header << "\n{\n";

                for (auto m : s.second.members)
                {
                    header << "\t" << toStrC(&m.second) << " " << m.first << ";\n";
                }

                header << "};\n\n";
            }
        }
    }

    // Step A4: Insert global definitions into header
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
                if (isTemplated(&s.ts.type))
                {
                    vector<string> templates = getTemplate(&s.ts.type);

                    // Header stuff
                    header << "template <";
                    for (int i = 0; i < templates.size(); i++)
                    {
                        header << "class " << templates[i];
                        if (i + 1 < templates.size())
                        {
                            header << ", ";
                        }
                    }
                    header << ">\n";

                    // Body stuff
                    if (s.seq.items.size() != 0)
                    {
                        body << "template <";
                        for (int i = 0; i < templates.size(); i++)
                        {
                            body << "class " << templates[i];
                            if (i + 1 < templates.size())
                            {
                                body << ", ";
                            }
                        }
                        body << ">\n";
                    }
                }

                if (s.ts.type.info == function)
                {
                    string toAdd = toStrCFunction(&s.ts.type, name);

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
                    string toAdd = toStrC(&s.ts.type);

                    header << "extern " << toAdd << " " << name << ";\n";
                    body << toAdd << " " << name << ";\n";
                }

                header << '\n';
            }
        }
    }

    // End header enclosure
    header << "\n#endif\n";

    // Part B1: Insert all function definitions

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
    // function -> GENERICS -> function -> ARGS -> maps -> RETURN_TYPE
    // function -> templ -> GENERICS -> templ_end
    //          -> function -> ARGS -> maps -> RETURN_TYPE

    // function -> ARGS -> maps -> RETURN_TYPE
    // RETURN_TYPE NAME(ARGS);

    parse_assert(What != nullptr && What->info == function);
    Type *current = What->next;

    // Generics section; optional
    if (current != nullptr && current->info == templ)
    {
        while (current != nullptr && current->info != templ_end)
        {
            current = current->next;
        }

        if (current != nullptr)
        {
            current = current->next;
        }
    }

    // Second section, between function and maps, will be arguments.
    string arguments = "";
    for (; current != nullptr && current->info != maps; current = current->next)
    {
        // (a: *const map<string, int>, b: u32)
        // (const map<string, int> *a, u32 b)

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

            arguments += toStrC(&temp) + " " + name;
        }
    }

    if (current == nullptr || current->info != maps)
    {
        cout << "Function '" << arguments << "' has no return type.\n";
        parse_assert(current != nullptr);
    }

    current = current->next;

    // Next, after maps, we will have the return type.
    string returnType = toStrC(current);

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
    return toStrCFunction(What, string("(*") + Name + ")");
}

string toStrC(Type *What)
{
    string out = "";

    // Safety check
    if (What == nullptr)
    {
        return out;
    }

    switch (What->info)
    {
    case generic:
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
    case templ:
        out += "<";
        out += toStrC(What->next);
        break;
    case templ_end:
        out += "> ";
        out += toStrC(What->next);
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

    return out;
}
