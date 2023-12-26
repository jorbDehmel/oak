#include "mem.hpp"

extern string toStrC(const Type *What, const string &Name = "", const unsigned int &pos = 0);

sequence getAllocSequence(Type &type, const string &name, const string &num)
{
    // Assumes that name is a pointer to type which already exists in scope
    // declarations are printed verbatim during reconstruction; ergo...

    sequence out;
    out.info = code_line;
    out.type = nullType;

    // C++
    // name = new type[len];
    // C
    // name = (type *)malloc(len * sizeof(type));

    out.items.push_back(sequence{nullType, vector<sequence>(), atom,
                                 name + " = (" + toStrC(&type) + " *)malloc(sizeof(" + name + ") * " + num + ")"});

    return out;
}

sequence getFreeSequence(const string &name, const bool &isArr)
{
    sequence out;
    out.info = code_line;
    out.type = nullType;
    out.items.clear();

    out.items.push_back(sequence{nullType, vector<sequence>(), atom, "free(" + name + ")"});

    return out;
}
