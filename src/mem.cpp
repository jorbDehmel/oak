/*
Jordan Dehmel
2023 - present
jdehmel@outlook.com
*/

#include "mem.hpp"

extern std::string toStrC(const Type *What, const std::string &Name = "", const unsigned int &pos = 0);

ASTNode getAllocSequence(Type &type, const std::string &name, const std::string &num)
{
    // Assumes that name is a pointer to type which already exists in scope
    //

    ASTNode out;
    out.info = code_line;
    out.type = nullType;

    // name = (type *)malloc(len * sizeof(type));
    out.items.push_back(
        ASTNode{nullType, std::vector<ASTNode>(), atom,
                name + " = (" + toStrC(&type) + " *)malloc(sizeof(" + toStrC(&type) + ") * " + num + ")"});

    return out;
}

ASTNode getFreeSequence(const std::string &name, const bool &isArr)
{
    ASTNode out;
    out.info = code_line;
    out.type = nullType;
    out.items.clear();

    out.items.push_back(ASTNode{nullType, std::vector<ASTNode>(), atom, "free(" + name + ")"});

    return out;
}
