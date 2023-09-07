#include "mangler.hpp"

/*
// Generic instantiation can become a new pass before sequencing

let node<t>: struct
{
    data: t,
    node: ^node<t>,
}

let get_data<t>(self: ^node<t>) -> t
{
    self.data
}

let main() -> i32
{
    // Implicitly calls instantiator
    let head: node<i32>;

    get_data<i32>(head);
}

// Becomes:

// node<t> moved exclusively to template table
// get_data<t> moved exclusively to template table

let main() -> i32
{
    let head: node<i32>;
    get_data<i32>(head);
}

// Calls instantiator!

// Retrieved node<t>, made node<i32>
let node_GEN_i32_ENDGEN: struct
{
    data: i32,
    node: ^node_GEN_i32_ENDGEN,
}

// Retrieved get_data<t>, made get_data<i32>
let get_data_GEN_i32_ENDGEN(self: ^node_GEN_i32_ENDGEN) -> i32
{
    self.data
}

let main() -> i32
{
    let head: node_GEN_i32_ENDGEN;
    get_data_GEN_i32_ENDGEN(head);
}

// Now resolvable in sequencing!

*/

string mangleStruct(const string &name, const vector<string> &generics)
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
        for (const auto &s : generics)
        {
            if (s != "<" && s != ">" && s != ",")
            {
                outputParts.push_back(s);
            }
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

string mangleEnum(const string &name, const vector<string> &generics)
{
    return mangleStruct(name, generics);
}

string mangleType(Type &type)
{
    vector<string> outputParts;

    for (Type *cur = &type; cur != nullptr; cur = cur->next)
    {
        switch (cur->info)
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
            outputParts.push_back(cur->name);
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
        else
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
