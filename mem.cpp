#include "mem.hpp"

sequence getAllocSequence(Type &type, const string &name, const int &num)
{
    // Assumes that name is a pointer to type which already exists in scope
    // declarations are printed verbatim during reconstruction; ergo...

    sequence out;
    out.info = code_line;
    out.type = nullType;
    out.items.clear();

    vector<string> toAdd = {name, "=", "new", toStrC(&type), "[", to_string(num), "]"};
    for (auto s : toAdd)
    {
        out.items.push_back(sequence{nullType, vector<sequence>(), atom, s});
    }

    return out;
}

sequence getFreeSequence(const string &name, const bool &isArr)
{
    sequence out;
    out.info = code_line;
    out.type = nullType;
    out.items.clear();

    vector<string> toAdd = {"delete"};
    if (isArr)
    {
        toAdd.push_back("[");
        toAdd.push_back("]");
    }
    toAdd.push_back(name);

    for (auto s : toAdd)
    {
        out.items.push_back(sequence{nullType, vector<sequence>(), atom, s});
    }

    return out;
}
