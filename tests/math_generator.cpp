// Script for generating the math definition files easier

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

struct fn_data
{
    string name, c_op;
    bool mut, force_ret_type_left;
};

inline void newfn(vector<fn_data> &into, const string &name, const string &c_op, const bool &mut = false, const bool &force_l = false)
{
    fn_data out;
    out.name = name;
    out.c_op = c_op;
    out.mut = mut;
    out.force_ret_type_left = force_l;
    into.push_back(out);

    return;
}

// Write all possible permutations in C++
// Including
void write_combos_c(const vector<string> &types, const vector<fn_data> &fns, ofstream &file)
{
    for (const auto &fn : fns)
    {
        for (int i = 0; i < types.size(); i++)
        {
            for (int j = i; j < types.size(); j++)
            {
                string ret;

                try
                {
                    if (stoi(types[i].substr(1)) > stoi(types[j].substr(1)))
                    {
                        ret = types[i];
                    }
                    else
                    {
                        ret = types[j];
                    }
                }
                catch (...)
                {
                    ret = types[i];
                }

                if (fn.mut || fn.force_ret_type_left)
                {
                    ret = types[i];
                }

                file << ret << " " << fn.name << "(" << types[i] << " " << (fn.mut ? "*" : "")
                     << "A, " << types[j] << " B)\n{\n"
                     << "\treturn (" << (fn.mut ? "*" : "") << "A " << fn.c_op << " B);\n"
                     << "}\n\n";
            }
        }
    }

    return;
}

// Write all possible permutations in Oak
void write_combos_oak(const vector<string> &types, const vector<fn_data> &fns, ofstream &file)
{
    for (const auto &fn : fns)
    {
        for (int i = 0; i < types.size(); i++)
        {
            for (int j = i; j < types.size(); j++)
            {
                string ret;

                try
                {
                    if (stoi(types[i].substr(1)) > stoi(types[j].substr(1)))
                    {
                        ret = types[i];
                    }
                    else
                    {
                        ret = types[j];
                    }
                }
                catch (...)
                {
                    ret = types[i];
                }

                file << "let " << fn.name << "(A: " << (fn.mut ? "^" : "") << types[i] << ", "
                     << "B: " << types[j] << ") -> " << ret << ";\n";
            }
        }
    }

    return;
}

void write_conversions_c(const vector<string> &types, ofstream &file)
{
    for (int i = 0; i < types.size(); i++)
    {
        for (int j = 0; j < types.size(); j++)
        {
            if (i == j)
            {
                continue;
            }

            string from, to;

            from = types[j];
            to = types[i];

            file << to
                 << " to_"
                 << to << "("
                 << from
                 << " what)\n"
                 << "{\n"
                 << "\treturn ("
                 << to
                 << ")(what);\n"
                 << "}\n\n";
        }
    }

    return;
}

void write_conversions_oak(const vector<string> &types, ofstream &file)
{
    for (int i = 0; i < types.size(); i++)
    {
        for (int j = i + 1; j < types.size(); j++)
        {
            string from, to;

            from = types[j];
            to = types[i];

            file << "let to_" << to << "(what: " << from << ") -> " << to << ";\n";
        }
    }

    return;
}

void write_to(const vector<string> &types, const vector<fn_data> &fns, const string &name)
{
    ofstream cpp(name + ".cpp");
    ofstream oak(name + ".oak");

    if (!cpp.is_open() || !oak.is_open())
    {
        throw runtime_error("Failed to open file(s)");
    }

    string header = "/*\n"
                    "This file was generated via script.\n"
                    "The time of generation was: ";

    auto now = time(NULL);
    header += ctime(&now);

    header += "\n"
              "Jordan Dehmel, 2023\n"
              "*/\n\n";

    cpp << header;
    oak << header;

    cpp << "#include \"std_oak_header.hpp\"\n\n";

    write_combos_c(types, fns, cpp);
    write_combos_oak(types, fns, oak);

    cpp << "// Conversions\n\n";

    oak << "\n// Conversions\n\n";

    write_conversions_c(types, cpp);
    write_conversions_oak(types, oak);

    cpp.close();
    oak.close();

    return;
}

#define NAME "extra_math"

int main()
{
    vector<string> types = {
        "u8",
        "i8",
        "u16",
        "i16",
        "u32",
        "i32",
        "u64",
        "i64",
        "u128",
        "i128",
        "f32",
        "f64",
        "f128",
    };

    vector<fn_data> fns;

    newfn(fns, "Add", "+", false);
    newfn(fns, "Sub", "+", false);
    newfn(fns, "Mult", "+", false);
    newfn(fns, "Div", "+", false);
    newfn(fns, "Mod", "+", false);

    newfn(fns, "AddEq", "+", false);
    newfn(fns, "SubEq", "+", false);
    newfn(fns, "MultEq", "+", false);
    newfn(fns, "DivEq", "+", false);
    newfn(fns, "ModEq", "+", false);

    newfn(fns, "Eq", "==", false);
    newfn(fns, "Neq", "!=", false);
    newfn(fns, "Less", "<", false);
    newfn(fns, "Great", ">", false);
    newfn(fns, "Leq", "<=", false);
    newfn(fns, "Greq", ">=", false);

    newfn(fns, "Copy", "=", true);

    write_to(types, fns, NAME);

    return 0;
}
