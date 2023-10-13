#include "mangler.hpp"

string mangleStruct(const string &name, const vector<vector<string>> &generics)
{
    if (generics.size() == 0)
    {
        return name;
    }

    vector<string> outputParts;
    outputParts.push_back(name);

    if (generics.size() != 0)
    {
        outputParts.push_back("GEN");

        // Generics here
        int i = 0;
        for (const auto &raw : generics)
        {
            string s = mangle(raw);
            outputParts.push_back(s);

            if (i + 1 < generics.size() && s != "PTR" && s != "GEN" && s != "ENDGEN" && s != "JOIN" && s != "")
            {
                outputParts.push_back("JOIN");
            }

            i++;
        }

        outputParts.push_back("ENDGEN");
    }

    string out;
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

string mangleEnum(const string &name, const vector<vector<string>> &generics)
{
    return mangleStruct(name, generics);
}

string mangleType(Type &type)
{
    vector<string> outputParts;

    for (int i = 0; i < type.size(); i++)
    {
        switch (type[i].info)
        {
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
            outputParts.push_back("TYPE");
            break;
        default:
            outputParts.push_back(type[i].name);
        }
    }

    string out;
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

string mangleSymb(const string &name, Type &type)
{
    string typeStr = mangleType(type);
    return mangleSymb(name, typeStr);
}

string mangleSymb(const string &name, const string &typeStr)
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

string mangle(const vector<string> &what)
{
    vector<string> outputParts;

    for (auto s : what)
    {
        if (s == "^")
        {
            outputParts.push_back("PTR");
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

    string out;
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
