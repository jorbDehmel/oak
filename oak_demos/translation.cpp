#include <iostream>
#include <string>
#include <fstream>
#include <map>
using namespace std;

/*
Oak translator

let i: u32;
u32 i;

let u32.hello() -> u32;
u32 __hello(u32 &__self);

*/

int main(const int argc, const char *argv[])
{
    if (argc != 3)
    {
        throw runtime_error("Invalid parameters");
    }

    ifstream inp(argv[1]);
    string line;
    if (!inp.is_open())
    {
        throw runtime_error("Invalid input file");
    }

    map<string, string> symbolTypes;
    while (getline(inp, line))
    {
        while (line[0] == '\t' || line[0] == ' ')
        {
            line.erase(0, 1);
        }

        if (line.size() > 2 && line.substr(0, 2) == "//")
        {
            // Single comment
            continue;
        }
        else if (false)
        {
        }
        else if (line.size() > 4 && line.substr(0, 4) == "let ")
        {
            string name = line.substr(4);
            string type;

            for (int i = 0; i < name.size(); i++)
            {
                if (name[i] == ':')
                {
                    type = name.substr(i + 1);
                    name.erase(i);
                    break;
                }
                else if (name[i] == '(')
                {
                    type = name.substr(i);
                    name.erase(i);
                    break;
                }
            }

            for (int i = 0; i < type.size(); i++)
            {
                if (type[i] == '=')
                {
                    type.erase(i);
                    break;
                }
            }

            while (name[0] == '\t' || name[0] == ' ')
            {
                name.erase(0, 1);
            }

            while (name.back() == '\t' || name.back() == ' ')
            {
                name.pop_back();
            }

            while (!type.empty() && (type[0] == '\t' || type[0] == ' '))
            {
                type.erase(0, 1);
            }

            while (!type.empty() && (type.back() == '\t' || type.back() == ' ' || type.back() == ';'))
            {
                type.pop_back();
            }

            symbolTypes[name] = type;
            cout << "Symbol '" << name << "' has type '" << type << "'\n";
        }
    }

    inp.close();

    ofstream out(argv[2]);
    if (!out.is_open())
    {
        throw runtime_error("Failed to open output file");
    }

    //////////////////// Load header ////////////////////
    ifstream head("std_oak_header.cpp");
    if (!head.is_open())
    {
        throw runtime_error("Filed to open std oak head file");
    }

    while (getline(head, line))
    {
        out << line << '\n';
    }

    head.close();
    /////////////////////////////////////////////////////

    out.close();

    return 0;
}
