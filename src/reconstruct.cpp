/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "oakc_fns.hpp"
#include "options.hpp"

// Removes illegal characters
std::string purifyStr(const std::string &What)
{
    const std::string illegalChars =
        "<>(){}[]\\'\"`~!@#$%^&*-=+|?.,;:/";
    std::string out = What;

    // Replace illegal characters
    for (int i = 0; i < out.size(); i++)
    {
        if (illegalChars.find(out[i]) != std::string::npos)
        {
            out[i] = '_';
        }
    }

    if (out == "")
    {
        std::cout << tags::yellow_bold
                  << "Warning: The trimming of file name '"
                  << What << "' resulted in a null string.\n"
                  << tags::reset;
        out = "NULL_STR";
    }

    return out;
}

std::string reconstructAndSave(const std::string &Name,
                               AcornSettings &settings)
{
    std::stringstream body;
    reconstruct(Name, settings, body);
    return save(body, Name);
}

void reconstruct(const std::string &Name,
                 AcornSettings &settings,
                 std::stringstream &body)
{
    // Step A1: Load Oak standard translational header
    body << "#include \"" << OAK_HEADER_PATH << "\"\n";

    // Step A2: Struct definitions
    for (auto name : settings.structOrder)
    {
        if (settings.enumData.count(name) != 0)
        {
            body << enumToC(name, settings) << '\n';
            continue;
        }

        body << "struct " << name << "\n{\n";

        for (auto m : settings.structData[name].order)
        {
            body << toStrC(
                        &settings.structData[name].members[m],
                        settings, m)
                 << ";\n";
        }

        body << "};\n";
    }

    // Step A4: Insert global definitions into header
    // (Translate Oak syntax into C syntax)
    for (auto entry : settings.table)
    {
        std::string name = entry.first;

        for (MultiTableSymbol s : entry.second)
        {
            try
            {
                std::string toAdd =
                    toStrCFunction(&s.type, settings, name);

                body << toAdd << ";\n";
            }
            catch (std::runtime_error &e)
            {
                std::cout << "Failure in symbol " << name
                          << " w/ type " << toStr(&s.type)
                          << " from " << s.sourceFilePath
                          << '\n';

                throw sequencing_error(e.what());
            }
        }
    }

    for (auto entry : settings.table)
    {
        std::string name = entry.first;

        for (MultiTableSymbol s : entry.second)
        {
            try
            {
                std::string toAdd =
                    toStrCFunction(&s.type, settings, name);

                if (s.seq.items.size() != 0)
                {
                    std::string definition =
                        toC(s.seq, settings);

                    if (definition != "")
                    {
                        body << toAdd << " " << definition;
                    }
                }
            }
            catch (std::runtime_error &e)
            {
                std::cout << "Failure in symbol " << name
                          << " w/ type " << toStr(&s.type)
                          << " from " << s.sourceFilePath
                          << '\n';

                throw sequencing_error(e.what());
            }
        }
    }

    return;
}

// Save reconstructed files and return name
std::string save(const std::stringstream &body,
                 const std::string &Name)
{
    std::string rootName, bodyName;

    rootName = purifyStr(Name);

    fs::create_directory(".oak_build");

    bodyName = ".oak_build/" + rootName + ".c";

    // Save body
    std::ofstream bodyFile(bodyName);
    if (!bodyFile.is_open())
    {
        throw std::runtime_error("Failed to open file `" +
                                 bodyName + "`");
    }

    bodyFile << body.str();
    bodyFile.close();

    return bodyName;
}

// This is separate due to complexity
std::string toStrCFunction(const Type *What,
                           AcornSettings &settings,
                           const std::string &Name,
                           const unsigned int &pos)
{
    parse_assert(What != nullptr);
    parse_assert(What->size() > 0);
    parse_assert((*What)[0].info == function);

    if (pos >= What->size())
    {
        return "";
    }

    // Second section, between function and maps, will be
    // arguments.
    std::string arguments = "";
    int i = pos + 1;
    for (; i < What->size(); i++)
    {
        // (a: *const map<string, int>, b: u32)
        // (const map<string, int> *a, u32 b)

        // Var names can only occur in non-fn-ptr types
        if (What->operator[](i).info == var_name)
        {
            std::string name = What->operator[](i).name;

            i++;

            if (i >= What->size())
            {
                break;
            }

            Type temp;
            while (i < What->size() &&
                   !(What->operator[](i).info == join ||
                     What->operator[](i).info == maps))
            {
                temp.append(What->operator[](i).info,
                            What->operator[](i).name);
                i++;
            }
            i--;

            if (arguments != "")
            {
                arguments += ", ";
            }

            arguments += toStrC(&temp, settings, name);
        }

        else if (What->operator[](i).info == maps)
        {
            break;
        }
    }

    if (i >= What->size() || What->operator[](i).info != maps)
    {
        std::cout << "Function '" << toStr(What)
                  << "' has no return type.\n";
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
    std::string returnType = toStrC(What, settings, "", i);

    if (returnType == "")
    {
        returnType = "void";
    }

    if (arguments.empty())
    {
        arguments = "void";
    }

    // Assemble and return: SEMICOLON IS HANDLED ELSEWHERE
    std::string out;

    if (Name != "main")
    {
        // Mangle to disambiguate identical functions
        out = returnType + " " +
              mangleSymb(Name, mangleType(*What)) + "(" +
              arguments + ")";
    }
    else
    {
        out = returnType + " main(" + arguments + ")";
    }

    return out;
}

std::string toStrCFunctionRef(const Type *What,
                              AcornSettings &settings,
                              const std::string &Name,
                              const unsigned int &pos)
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

    std::string returnType = "";
    std::string arguments = "";
    Type argType;
    int count = 0;

    int i = pos + 1;

    // Then some number of args, possibly containing maps
    do
    {
        TypeNode &cur = (*What)[i];

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

            arguments += toStrC(&argType, settings);

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

        arguments += toStrC(&argType, settings);
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
        returnType = toStrC(What, settings, "", i);
    }

    if (arguments.empty())
    {
        arguments = "void";
    }

    // Reconstruct out of partial strings
    std::string out =
        returnType + " (*" + Name + ")(" + arguments + ")";

    return out;
}

std::string toStrC(const Type *What, AcornSettings &settings,
                   const std::string &Name,
                   const unsigned int &pos)
{
    if (settings.toStrCTypeCache.count(
            make_pair(What->ID, Name)) != 0)
    {
        return settings
            .toStrCTypeCache[make_pair(What->ID, Name)];
    }
    if (settings.toStrCTypeCache.size() > 1000)
    {
        settings.toStrCTypeCache.clear();
    }

    std::string out = "";
    std::string suffix = "";

    // Safety check
    if (What == nullptr || What->size() == 0 ||
        pos >= What->size())
    {
        settings.toStrCTypeCache[make_pair(What->ID, Name)] =
            "";
        return "";
    }

    if ((*What)[pos].info == pointer && What->size() > 1 &&
        (*What)[pos + 1].info == function)
    {
        out = toStrCFunctionRef(What, settings, Name);
        settings.toStrCTypeCache[make_pair(What->ID, Name)] =
            out;
        return out;
    }

    switch ((*What)[pos].info)
    {
    case atomic:
        if (ATOMICS.count((*What)[pos].name) == 0)
        {
            out += "struct ";
        }

        out += (*What)[pos].name;
        out += toStrC(What, settings, "", pos + 1);
        break;
    case join:
        out += ",";
        out += toStrC(What, settings, "", pos + 1);
        break;
    case arr:
    case pointer:
        sm_assert(pos + 1 >= What->size() ||
                      (*What)[pos + 1].info != sarr,
                  "Cannot point to a sized array [n]. Use a "
                  "regular array [] instead.");
        out += toStrC(What, settings, "", pos + 1);
        out += "*";
        break;
    case sarr:
        // Sized array- goes after in C
        out += toStrC(What, settings, "", pos + 1);
        suffix = "[" + (*What)[pos].name + "]" + suffix;
        break;
    case var_name:
        out += toStrC(What, settings, Name, pos + 1);
        out += (*What)[pos].name;
        break;

    case function:
    case maps:
        out += toStrC(What, settings, "", pos + 1);
        break;

    default:
        throw parse_error(
            "Unforeseen info enum option for Type object.");
    }

    if (Name != "")
    {
        out += " " + Name;
    }

    if (suffix != "")
    {
        out += suffix;
    }

    settings.toStrCTypeCache[make_pair(What->ID, Name)] = out;

    return out;
}

std::string enumToC(const std::string &name,
                    AcornSettings &settings)
{
    if (settings.toStrCEnumCache.count(name) != 0)
    {
        return settings.toStrCEnumCache[name];
    }

    // Basic error checking; Should NOT constitute the entirety
    // of safety checks!!! This just ensures a lack of internal
    // errors.
    if (name == "" || settings.enumData.count(name) == 0)
    {
        throw std::runtime_error(std::string(__FILE__) + ":" +
                                 std::to_string(__LINE__) +
                                 " Error in enumeration toC.");
    }

    EnumLookupData &cur = settings.enumData[name];

    std::string out = "struct " + name + " {\nenum {\n";

    // names
    for (auto item_name : cur.order)
    {
        out += name + "_OPT_" + item_name + ",\n";
    }

    out += "\n} __info;\nunion {\n";

    // types
    for (auto name : cur.order)
    {
        out += toStrC(&cur.options[name], settings) + " " +
               name + "_data" + ";\n";
    }

    out += "\n} __data;\n};\n";

    // That's the end of the easy part.
    // Now, it must generate constructors.

    std::string enumTypeStr = name;
    for (auto optionName : cur.order)
    {
        std::string optionTypeStr =
            toStrC(&cur.options[optionName], settings);

        if (cur.options[optionName][0].info == atomic &&
            cur.options[optionName][0].name == "unit")
        {
            // Unit struct; Single argument constructor

            // Generate C version
            out += "void wrap_" + optionName + "_FN_PTR_" +
                   enumTypeStr + "_MAPS_void(struct " +
                   enumTypeStr + " *self)\n{\n";
            out += "self->__info = " + enumTypeStr + "_OPT_" +
                   optionName + ";\n}\n";
        }
        else
        {
            // Double argument constructor

            // Generate C version
            out += "void wrap_" + optionName + "_FN_PTR_" +
                   enumTypeStr + "_JOIN_" +
                   mangleType(cur.options[optionName]) +
                   "_MAPS_void(struct " + enumTypeStr +
                   " *self, ";
            out += optionTypeStr + " data)\n";
            out += "{\n";
            out += "self->__info = " + enumTypeStr + "_OPT_" +
                   optionName + ";\n";
            out += "self->__data." + optionName +
                   "_data = data;\n";
            out += "}\n";
        }
    }

    if (settings.toStrCEnumCache.size() > 1000)
    {
        settings.toStrCEnumCache.clear();
    }
    settings.toStrCEnumCache[name] = out;

    return out;
}
