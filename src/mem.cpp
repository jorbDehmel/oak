/*
Jordan Dehmel
2023 - present
jdehmel@outlook.com
*/

#include "mem.hpp"

extern std::string toStrC(const Type *What, const std::string &Name = "", const unsigned int &pos = 0);

sequence getAllocSequence(Type &type, const std::string &name, const std::string &num)
{
    // Assumes that name is a pointer to type which already exists in scope
    //

    sequence out;
    out.info = code_line;
    out.type = nullType;

    // name = (type *)malloc(len * sizeof(type));
    out.items.push_back(
        sequence{nullType, std::vector<sequence>(), atom,
                 name + " = (" + toStrC(&type) + " *)malloc(sizeof(" + toStrC(&type) + ") * " + num + ")"});

    return out;
}

sequence getFreeSequence(const std::string &name, const bool &isArr)
{
    sequence out;
    out.info = code_line;
    out.type = nullType;
    out.items.clear();

    out.items.push_back(sequence{nullType, std::vector<sequence>(), atom, "free(" + name + ")"});

    return out;
}
