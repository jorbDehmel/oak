/*
Jordan Dehmel, 2023 - present
*/

#include "mangler.hpp"

std::string mangleStruct(const std::string &name, const std::vector<std::vector<std::string>> &generics)
{
    if (generics.size() == 0)
    {
        return name;
    }

    std::vector<std::string> outputParts;
    outputParts.push_back(name);

    if (generics.size() != 0)
    {
        outputParts.push_back("GEN");

        // Generics here
        int i = 0;
        for (const auto &raw : generics)
        {
            std::string s = mangle(raw);
            outputParts.push_back(s);

            if (i + 1 < generics.size() && s != "PTR" && s != "GEN" && s != "ENDGEN" && s != "JOIN" && s != "")
            {
                outputParts.push_back("JOIN");
            }

            i++;
        }

        outputParts.push_back("ENDGEN");
    }

    std::string out;
    for (int i = 0; i < outputParts.size(); i++)
    {
        out += outputParts[i];

        if (i + 1 < outputParts.size())
        {
            out += "_";
        }
    }

    return out;
}

std::string mangleEnum(const std::string &name, const std::vector<std::vector<std::string>> &generics)
{
    return mangleStruct(name, generics);
}

std::string mangleType(const Type &type)
{
    std::vector<std::string> outputParts;

    for (int i = 0; i < type.size(); i++)
    {
        switch (type[i].info)
        {
        case sarr:
            outputParts.push_back("SARR");
            outputParts.push_back(type[i].name);
            break;
        case arr:
            outputParts.push_back("ARR");
            break;
        case pointer:
            outputParts.push_back("PTR");
            break;
        case join:
            outputParts.push_back("JOIN");
            break;
        case function:
            outputParts.push_back("FN");
            break;
        case maps:
            outputParts.push_back("MAPS");
            break;
        case var_name:
            break;
        default:
            outputParts.push_back(type[i].name);
        }
    }

    std::string out;
    for (int i = 0; i < outputParts.size(); i++)
    {
        out += outputParts[i];

        if (i + 1 < outputParts.size())
        {
            out += "_";
        }
    }

    return out;
}

std::string mangleSymb(const std::string &name, Type &type)
{
    std::string typeStr = mangleType(type);
    return mangleSymb(name, typeStr);
}

std::string mangleSymb(const std::string &name, const std::string &typeStr)
{
    if (typeStr == "")
    {
        return name;
    }
    else
    {
        return name + "_" + typeStr;
    }
}

std::string mangle(const std::vector<std::string> &what)
{
    std::vector<std::string> outputParts;

    for (auto s : what)
    {
        if (s == "^")
        {
            outputParts.push_back("PTR");
        }
        else if (s == "[]")
        {
            outputParts.push_back("ARR");
        }
        else if (s == "[")
        {
            // Sized ARR- takes one arg after
            outputParts.push_back("SARR");
        }
        else if (s == "]")
        {
            ;
        }
        else if (s == ",")
        {
            outputParts.push_back("JOIN");
        }
        else if (s == "(")
        {
            outputParts.push_back("FN");
        }
        else if (s == ")")
        {
            ;
        }
        else if (s == "->")
        {
            outputParts.push_back("MAPS");
        }
        else if (s == ":")
        {
            outputParts.push_back("TYPE");
        }
        else if (s == "<")
        {
            outputParts.push_back("GEN");
        }
        else if (s == ">")
        {
            outputParts.push_back("ENDGEN");
        }
        else if (s != "")
        {
            outputParts.push_back(s);
        }
    }

    std::string out;
    for (int i = 0; i < outputParts.size(); i++)
    {
        out += outputParts[i];

        if (i + 1 < outputParts.size())
        {
            out += "_";
        }
    }

    return out;
}
