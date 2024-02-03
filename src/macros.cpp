/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "macros.hpp"
#include "options.hpp"
#include <filesystem>
#include <stdexcept>
namespace fs = std::filesystem;

// Returns true if and only if the given macro already exists.
bool hasMacro(const std::string &name, AcornSettings &settings) noexcept
{
    return settings.macros.count(name) != 0;
}

// Adds the given macro definition into the internal macro lookup table.
void addMacro(const std::string &name, const std::string &contents, AcornSettings &settings)
{
    settings.macros[name] = contents;
    settings.macroSourceFiles[name] = settings.curFile;
}

long long getFileLastModification(const std::string &filepath)
{
    static std::map<std::string, long long> ageCache;

    if (ageCache.count(filepath) != 0)
    {
        return ageCache[filepath];
    }

    // Get age (theoretically system independent)
    long long out = -1;

    if (fs::exists(filepath))
    {
        out = fs::last_write_time(filepath).time_since_epoch().count();
    }

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
    long long sourceAge = getFileLastModification(source);
    long long destAge = getFileLastModification(dest);

    if (sourceAge == -1 || destAge == -1)
    {
        return true;
    }

    return (sourceAge > destAge);
}

void compileMacro(const std::string &Name, AcornSettings &settings)
{
    if (settings.macros.count(Name) == 0)
    {
        throw std::runtime_error("Cannot compile macro `" + Name + "`; No source code present");
    }
    else if (Name.back() != '!')
    {
        throw std::runtime_error("'" + Name + "' is an illegal macro name; This is an error of the compiler");
    }
    else if (settings.compiled.count(Name) != 0)
    {
        // Already compiled: This is fine
        return;
    }

    std::string rootName = purifyStr(Name.substr(0, Name.size() - 1));

    // Write file to be compiled
    std::string srcPath = COMPILED_PATH + rootName + ".oak";
    std::string binPath = COMPILED_PATH + rootName + ".out";

    // Check ages, makefile-style
    if (!isSourceNewer(settings.macroSourceFiles[Name], binPath))
    {
        return;
    }

    fs::create_directory(COMPILED_PATH);

    std::ofstream macroFile(srcPath);
    if (!macroFile.is_open())
    {
        throw std::runtime_error("Could not open file `" + srcPath + "`");
    }

    macroFile << settings.macros[Name] << '\n';

    macroFile.close();

    // Call compiler
    std::string command =
        COMPILER_COMMAND + (settings.debug ? std::string(" -d") : std::string("")) + " -Mo " + binPath + " " + srcPath;

    if (settings.debug)
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
        if (settings.debug)
        {
            std::cout << tags::yellow_bold << "\n/\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\\n"
                      << "-----------------------------\n"
                      << "Exiting sub-file '" << srcPath << "'\n"
                      << "-----------------------------\n\n"
                      << tags::reset << std::flush;
        }

        throw std::runtime_error(std::string("Macro failure: ") + e.what());
    }

    if (settings.debug)
    {
        std::cout << tags::yellow_bold << "/\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\\n"
                  << "-----------------------------\n"
                  << "Exiting sub-file '" << srcPath << "'\n"
                  << "-----------------------------\n\n"
                  << tags::reset << std::flush;
    }

    settings.compiled.insert(Name);

    return;
}

std::string callMacro(const std::string &Name, const std::vector<std::string> &Args, AcornSettings &settings)
{
    if (settings.compiled.count(Name) == 0)
    {
        compileMacro(Name, settings);
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

    if (settings.debug)
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

    if (settings.debug)
    {
        std::cout << "Macro returned\n```\n" << out << "\n```\n";
    }

    return out;
}
