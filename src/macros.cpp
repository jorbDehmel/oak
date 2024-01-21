/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "macros.hpp"
#include <stdexcept>

// The pre-inserted ones are used by the compiler- Not literal macros
std::set<std::string> compiled = {"include!",  "link!",     "package!",  "alloc!",    "free!",
                                  "free_arr!", "new_rule!", "use_rule!", "rem_rule!", "bundle_rule!",
                                  "erase!",    "c_print!",  "c_warn!",   "c_panic!",  "type!",
                                  "size!",     "ptrcpy!",   "ptrarr!",   "raw_c!"};
std::map<std::string, std::string> macros;
std::map<std::string, std::string> macroSourceFiles;

long long getAgeOfFile(const std::string &filepath)
{
    static std::map<std::string, long long> ageCache;

    if (ageCache.count(filepath) != 0)
    {
        return ageCache[filepath];
    }

    smartSystem("mkdir -p .oak_build");
    if (system(("stat -Lc %Y " + filepath + " > .oak_build/age_temp.txt 2> /dev/null").c_str()) != 0)
    {
        return -1;
    }

    std::ifstream file(".oak_build/age_temp.txt");
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
bool isSourceNewer(const std::string &source, const std::string &dest)
{
    long long sourceAge = getAgeOfFile(source);
    long long destAge = getAgeOfFile(dest);

    if (sourceAge == -1 || destAge == -1)
    {
        return true;
    }

    return (sourceAge > destAge);
}

void compileMacro(const std::string &Name, bool debug)
{
    if (macros.count(Name) == 0)
    {
        throw std::runtime_error("Cannot compile macro `" + Name + "`; No source code present");
    }
    else if (Name.back() != '!')
    {
        throw std::runtime_error("'" + Name + "' is an illegal macro name; This is an error of the compiler");
    }
    else if (compiled.count(Name) != 0)
    {
        // Already compiled: This is fine
        return;
    }

    std::string rootName = purifyStr(Name.substr(0, Name.size() - 1));

    // Write file to be compiled
    std::string srcPath = COMPILED_PATH + rootName + ".oak";
    std::string binPath = COMPILED_PATH + rootName + ".out";

    // Check ages, makefile-style
    if (!isSourceNewer(macroSourceFiles[Name], binPath))
    {
        return;
    }

    if (system(("mkdir -p " + COMPILED_PATH).c_str()) != 0)
    {
        throw std::runtime_error("Failed to create temp folder " + COMPILED_PATH);
    }

    std::ofstream macroFile(srcPath);
    if (!macroFile.is_open())
    {
        throw std::runtime_error("Could not open file `" + srcPath + "`");
    }

    macroFile << macros[Name] << '\n';

    macroFile.close();

    // Call compiler
    std::string command =
        COMPILER_COMMAND + (debug ? std::string(" -d") : std::string("")) + " -Mo " + binPath + " " + srcPath;

    if (debug)
    {
        std::cout << "Compiling via command '" << command << "'\n";

        std::cout << tags::yellow_bold << "\n-----------------------------\n"
                  << "Entering sub-file '" << srcPath << "'\n"
                  << "-----------------------------\n"
                  << "\\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/\n\n"
                  << tags::reset << std::flush;
    }

    try
    {
        if (system(command.c_str()) != 0)
        {
            throw std::runtime_error("System call '" + command + "' failed.");
        }
    }
    catch (std::runtime_error &e)
    {
        if (debug)
        {
            std::cout << tags::yellow_bold << "\n/\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\\n"
                      << "-----------------------------\n"
                      << "Exiting sub-file '" << srcPath << "'\n"
                      << "-----------------------------\n\n"
                      << tags::reset << std::flush;
        }

        throw std::runtime_error(std::string("Macro failure: ") + e.what());
    }

    if (debug)
    {
        std::cout << tags::yellow_bold << "/\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\\n"
                  << "-----------------------------\n"
                  << "Exiting sub-file '" << srcPath << "'\n"
                  << "-----------------------------\n\n"
                  << tags::reset << std::flush;
    }

    compiled.insert(Name);

    return;
}

std::string callMacro(const std::string &Name, const std::vector<std::string> &Args, bool debug)
{
    if (compiled.count(Name) == 0)
    {
        compileMacro(Name, debug);
    }

    std::string outputName = COMPILED_PATH + std::string("__oak_macro_out") + ".txt";

    std::string command = COMPILED_PATH + purifyStr(Name.substr(0, Name.size() - 1)) + ".out ";

    // args here
    for (std::string s : Args)
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
                command += std::string("\\") + c;
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
        std::cout << "Macro call `" << command << "`\n";
    }

    try
    {
        if (system(command.c_str()) != 0)
        {
            throw std::runtime_error(std::string("Self-reported macro failure with command '" + command + "'"));
        }
    }
    catch (...)
    {
        throw std::runtime_error(std::string("Macro system failure with command '" + command + "'"));
    }

    // Load from output file
    std::string out = "";
    std::ifstream file(outputName);

    if (!file.is_open())
    {
        throw std::runtime_error(std::string("Failed to open macro output file ") + outputName);
    }

    std::string line;
    while (getline(file, line))
    {
        out += line + '\n';
    }

    file.close();

    if (debug)
    {
        std::cout << "Macro returned\n```\n" << out << "\n```\n";
    }

    return out;
}
