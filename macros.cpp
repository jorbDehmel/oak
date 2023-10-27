/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "macros.hpp"

// The pre-inserted ones are used by the compiler- Not literal macros
set<string> compiled = {
    "include!",
    "link!",
    "package!",
    "alloc!",
    "free!",
    "free_arr!",
    "new_rule!",
    "use_rule!",
    "rem_rule!",
    "bundle_rule!",
    "erase!",
    "c_print!",
    "c_panic!",
    "type!",
    "size!"};
map<string, string> macros;
map<string, string> macroSourceFiles;

long long getAgeOfFile(const string &filepath)
{
    static map<string, long long> ageCache;

    if (ageCache.count(filepath) != 0)
    {
        return ageCache[filepath];
    }

    smartSystem("mkdir -p .oak_build");
    if (system(("stat -Lc %Y " + filepath + " > .oak_build/age_temp.txt 2> /dev/null").c_str()) != 0)
    {
        return -1;
    }

    ifstream file(".oak_build/age_temp.txt");
    if (!file.is_open())
    {
        return -1;
    }

    long long out = -1;
    file >> out;

    file.close();

    if (ageCache.size() > 200)
    {
        ageCache.clear();
    }
    ageCache[filepath] = out;

    return out;
}

// Returns true if the source file is newer than the destination one
// OR if either file is nonexistant
bool isSourceNewer(const string &source, const string &dest)
{
    long long sourceAge = getAgeOfFile(source);
    long long destAge = getAgeOfFile(dest);

    if (sourceAge == -1 || destAge == -1)
    {
        return true;
    }

    return (sourceAge > destAge);
}

void compileMacro(const string &Name, bool debug)
{
    if (macros.count(Name) == 0)
    {
        throw runtime_error("Cannot compile macro `" + Name + "`; No source code present");
    }
    else if (Name.back() != '!')
    {
        throw runtime_error("'" + Name + "' is an illegal macro name; This is an error of the compiler");
    }
    else if (compiled.count(Name) != 0)
    {
        // Already compiled: This is fine
        return;
    }

    string rootName = purifyStr(Name.substr(0, Name.size() - 1));

    // Write file to be compiled
    string srcPath = COMPILED_PATH + rootName + ".oak";
    string binPath = COMPILED_PATH + rootName + ".out";

    // Check ages, makefile-style
    if (!isSourceNewer(macroSourceFiles[Name], srcPath))
    {
        return;
    }

    throw_assert(system("mkdir -p " COMPILED_PATH) == 0);

    ofstream macroFile(srcPath);
    if (!macroFile.is_open())
    {
        throw runtime_error("Could not open file `" + srcPath + "`");
    }

    macroFile << macros[Name] << '\n';

    macroFile.close();

    // Call compiler
    string command = COMPILER_COMMAND + (debug ? string(" -d") : string("")) + " -Mo " + binPath + " " + srcPath;

    if (debug)
    {
        cout << "Compiling via command '" << command << "'\n";

        cout << tags::yellow_bold
             << "\n-----------------------------\n"
             << "Entering sub-file '" << srcPath << "'\n"
             << "-----------------------------\n"
             << "\\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/\n\n"
             << tags::reset
             << flush;
    }

    try
    {
        throw_assert(system(command.c_str()) == 0);
    }
    catch (runtime_error &e)
    {
        if (debug)
        {
            cout << tags::yellow_bold
                 << "\n/\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\\n"
                 << "-----------------------------\n"
                 << "Exiting sub-file '" << srcPath << "'\n"
                 << "-----------------------------\n\n"
                 << tags::reset
                 << flush;
        }

        throw runtime_error(string("Macro failure: ") + e.what());
    }

    if (debug)
    {
        cout << tags::yellow_bold
             << "/\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\\n"
             << "-----------------------------\n"
             << "Exiting sub-file '" << srcPath << "'\n"
             << "-----------------------------\n\n"
             << tags::reset
             << flush;
    }

    compiled.insert(Name);

    return;
}

string callMacro(const string &Name, const vector<string> &Args, bool debug)
{
    if (compiled.count(Name) == 0)
    {
        compileMacro(Name, debug);
    }

    string outputName = COMPILED_PATH + string("__oak_macro_out") + ".txt";

    string command = COMPILED_PATH + purifyStr(Name.substr(0, Name.size() - 1)) + ".out ";

    // args here
    for (string s : Args)
    {
        if (s.size() == 0)
        {
            continue;
        }

        if (s.front() != '"' || s.back() != '"')
        {
            command += '"';
        }

        int pos = 0;
        for (char c : s)
        {
            if (c == '"' && pos != 0 && pos + 1 != s.size())
            {
                command += string("\\") + c;
            }
            else
            {
                command += c;
            }

            pos++;
        }

        if (s.front() != '"' || s.back() != '"')
        {
            command += '"';
        }

        command += " ";
    }

    command += " > " + outputName;

    if (debug)
    {
        cout << "Macro call `" << command << "`\n";
    }

    try
    {
        if (system(command.c_str()) != 0)
        {
            throw runtime_error(string("Self-reported macro failure with command '" + command + "'"));
        }
    }
    catch (...)
    {
        throw runtime_error(string("Macro system failure with command '" + command + "'"));
    }

    // Load from output file
    string out = "";
    ifstream file(outputName);

    if (!file.is_open())
    {
        throw runtime_error(string("Failed to open macro output file ") + outputName);
    }

    string line;
    while (getline(file, line))
    {
        out += line + '\n';
    }

    file.close();

    if (debug)
    {
        cout << "Macro returned\n```\n"
             << out << "\n```\n";
    }

    return out;
}
