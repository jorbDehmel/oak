/*
This file contains miscellaneous function definitions which used
to occupy separate files. These have been consolidated for the
sake of linearizing the `include` flow.

Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "oakc_fns.hpp"
#include "oakc_structs.hpp"
#include "options.hpp"
#include "tags.hpp"
#include <stdexcept>
#include <string>
#include <unistd.h>

// Returns true if and only if (it + offset) == compareTo.
// Runs in O(offset), unfortunately, so only use for small
// offsets. If it is too close to the beginning or end of
// `inside`, will return false,
bool itCmp(const std::list<Token> &inside,
           const std::list<Token>::iterator &it,
           const int &offset, const Token &compareTo) noexcept
{
    auto temp = it;

    if (offset > 0)
    {
        for (int i = 0; i < offset; i++)
        {
            temp++;

            if (temp == inside.end())
            {
                return false;
            }
        }
    }
    else
    {
        for (int i = 0; i < -offset; i++)
        {
            if (temp == inside.begin())
            {
                return false;
            }

            temp--;
        }
    }

    return *temp == compareTo;
}

bool itCmp(const std::list<Token> &inside,
           const std::list<Token>::iterator &it,
           const int &offset,
           const std::string &compareTo) noexcept
{
    return itCmp(inside, it, offset, Token(compareTo));
}

bool itCmp(const std::list<Token> &inside,
           const std::list<Token>::const_iterator &it,
           const int &offset,
           const std::string &compareTo) noexcept
{
    auto temp = it;

    if (offset > 0)
    {
        for (int i = 0; i < offset; i++)
        {
            temp++;

            if (temp == inside.end())
            {
                return false;
            }
        }
    }
    else
    {
        for (int i = 0; i < -offset; i++)
        {
            if (temp == inside.begin())
            {
                return false;
            }

            temp--;
        }
    }

    return *temp == compareTo;
}

// Returns true if the given iterator is in range.
bool itIsInRange(const std::list<Token> &inside,
                 const std::list<Token>::iterator &it,
                 const int &offset) noexcept
{
    auto temp = it;

    if (temp == inside.end())
    {
        return false;
    }

    if (offset > 0)
    {
        for (int i = 0; i < offset; i++)
        {
            temp++;

            if (temp == inside.end())
            {
                return false;
            }
        }
    }
    else
    {
        for (int i = 0; i < -offset; i++)
        {
            if (temp == inside.begin())
            {
                return false;
            }

            temp--;
        }
    }

    return true;
}

// Returns the token at the given offset, or a null token if
// it is out of range.
Token itGet(const std::list<Token> &inside,
            const std::list<Token>::iterator &it,
            const int &offset) noexcept
{
    auto temp = it;

    if (offset > 0)
    {
        for (int i = 0; i < offset; i++)
        {
            temp++;

            if (temp == inside.end())
            {
                return Token("");
            }
        }
    }
    else
    {
        for (int i = 0; i < -offset; i++)
        {
            if (temp == inside.begin())
            {
                return Token("");
            }

            temp--;
        }
    }

    return *temp;
}

std::string execute(const std::string &command)
{
    std::string output = "";
    char buffer[128];

    // Start process and open output as the file `pipe`
    FILE *pipe = popen(command.c_str(), "r");
    if (pipe == NULL)
    {
        throw sequencing_error("Failed to run command '" +
                               command + "'");
    }

    // For as long as there is data to read, read it.
    try
    {
        // Read chunk
        while (fgets(buffer, sizeof(buffer), pipe) != NULL)
        {
            // Append
            output += buffer;
            memset(buffer, 0, sizeof(buffer));
        }
    }
    catch (...)
    {
        pclose(pipe);
        throw sequencing_error(
            "Command '" + command +
            "' failed during output reading.");
    }

    // Close the pipe and get return value
    int return_value = pclose(pipe);

    // If return value is not 0, throw error
    if (return_value != 0)
    {
        throw sequencing_error(
            "Command '" + command + "' failed with exit code " +
            std::to_string(return_value) + ".");
    }

    // Otherwise, return retrieved output
    return output;
}

void generate(const std::list<std::string> &Files,
              const std::string &Output)
{
    if (Files.size() == 0)
    {
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();

    std::ofstream out(Output);
    if (!out.is_open())
    {
        throw std::runtime_error("Failed to open file '" +
                                 Output + "'");
    }

    // File header
    out << HEADER_START << Output << "\n\n"
        << "Generated by Acorn from source files:\n";

    for (auto fileName : Files)
    {
        out << " - " << fileName << '\n';
    }

    auto curTime = time(NULL);
    out << ctime(&curTime) << '\n';

    // Iterate over files
    for (auto fileName : Files)
    {
        // Insert header
        out << H_LINE << '\n'
            << FILE_START << fileName << "\n\n";

        std::map<std::string, std::string> data;
        std::vector<std::string> lines;
        std::string line;

        // Load file
        std::ifstream file(fileName);
        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file '" +
                                     fileName + "'");
        }

        while (getline(file, line))
        {
            lines.push_back(line);
        }

        file.close();

        // Scan and build data
        std::string mostRecentComment = "";
        for (int i = 0; i < lines.size(); i++)
        {
            if (lines[i].size() >= 2 &&
                lines[i].substr(0, 2) == "//")
            {
                // Single line comment
                mostRecentComment += lines[i].substr(3) + "\n";
            }
            else if (lines[i].size() >= 2 &&
                     lines[i].substr(0, 2) == "/*")
            {
                // Multi-line comment
                while (lines[i].size() < 2 ||
                       lines[i].substr(0, 2) != "*/")
                {
                    mostRecentComment += lines[i] + "\n";
                    i++;
                }

                // Erase the begin comment
                mostRecentComment.erase(0, 2);

                // Erase the end comment
                mostRecentComment.pop_back();
                mostRecentComment.pop_back();
            }
            else if (lines[i] == "{")
            {
                // Code scope; Skip to end

                int count = 0;
                do
                {
                    if (lines[i] == "{")
                    {
                        count++;
                    }
                    else if (lines[i] == "}")
                    {
                        count--;
                    }

                    i++;
                } while (i < lines.size() && count != 0);

                // Reset commenting
                mostRecentComment = "";
            }
            else
            {
                if (lines[i].find("!") == std::string::npos)
                {
                    // Add to data
                    data[lines[i]] = mostRecentComment;
                }

                // Reset commenting
                mostRecentComment = "";
            }
        }

        // Reconstruct into output document
        for (std::pair<std::string, std::string> cur : data)
        {
            if (cur.first.empty())
            {
                continue;
            }

            out << FN_START << cur.first << "\n\n";

            if (cur.second == "")
            {
                out << "(No documentation was provided)\n\n";
            }
            else
            {
                out << cur.second << "\n\n";
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    unsigned long long elapsedNS =
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            end - start)
            .count();

    out << H_LINE << "\n\n"
        << "Generation took " << elapsedNS << " ns, about "
        << ((double)elapsedNS / Files.size())
        << " ns / file.\n";

    out.close();

    return;
}

// Returns true if and only if the given macro already exists.
bool hasMacro(const std::string &name,
              AcornSettings &settings) noexcept
{
    return settings.macros.count(name) != 0;
}

// Adds the given macro definition into the internal macro
// lookup table.
void addMacro(const std::string &name,
              const std::string &contents,
              AcornSettings &settings)
{
    settings.macros[name] = contents;
    settings.macroSourceFiles[name] = settings.curFile;
}

long long getFileLastModification(const std::string &filepath,
                                  AcornSettings &settings)
{
    if (settings.ageCache.count(filepath) != 0)
    {
        return settings.ageCache[filepath];
    }

    long long out = -1;

    if (fs::exists(filepath))
    {
        out = fs::last_write_time(filepath)
                  .time_since_epoch()
                  .count();
    }

    if (settings.ageCache.size() > 200)
    {
        settings.ageCache.clear();
    }
    settings.ageCache[filepath] = out;

    return out;
}

// Returns true if the source file is newer than the destination
// one OR if either file is nonexistant
bool isSourceNewer(const std::string &source,
                   const std::string &dest,
                   AcornSettings &settings)
{
    long long sourceAge =
        getFileLastModification(source, settings);
    long long destAge = getFileLastModification(dest, settings);

    if (sourceAge == -1 || destAge == -1)
    {
        return true;
    }

    return (sourceAge > destAge);
}

void compileMacro(const std::string &Name,
                  AcornSettings &settings)
{
    if (settings.macros.count(Name) == 0)
    {
        throw std::runtime_error("Cannot compile macro `" +
                                 Name +
                                 "`; No source code present");
    }
    else if (Name.back() != '!')
    {
        throw std::runtime_error(
            "'" + Name +
            "' is an illegal macro name; This is an error of "
            "the compiler");
    }
    else if (settings.compiled.count(Name) != 0)
    {
        // Already compiled: This is fine
        return;
    }

    std::string rootName =
        purifyStr(Name.substr(0, Name.size() - 1));

    // Write file to be compiled
    std::string srcPath = COMPILED_PATH + rootName + ".oak";
    std::string binPath = COMPILED_PATH + rootName + ".out";

    // Check ages, makefile-style
    if (!isSourceNewer(settings.macroSourceFiles[Name], binPath,
                       settings))
    {
        return;
    }

    fs::create_directory(COMPILED_PATH);

    std::ofstream macroFile(srcPath);
    if (!macroFile.is_open())
    {
        throw std::runtime_error("Could not open file `" +
                                 srcPath + "`");
    }

    macroFile << settings.macros[Name] << '\n';

    macroFile.close();

    // Call compiler
    std::string command = COMPILER_COMMAND +
                          (settings.debug ? std::string(" -d")
                                          : std::string("")) +
                          " -Mo " + binPath + " " + srcPath;

    if (settings.debug)
    {
        std::cout << "Compiling via command '" << command
                  << "'\n";

        std::cout
            << tags::yellow_bold
            << "\n-----------------------------\n"
            << "Entering sub-file '" << srcPath << "'\n"
            << "-----------------------------\n"
            << "\\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/ \\/\n\n"
            << tags::reset << std::flush;
    }

    try
    {
        std::string result = execute(command);

        if (result != "")
        {
            std::cout << result << "\n";
        }
    }
    catch (std::runtime_error &e)
    {
        if (settings.debug)
        {
            std::cout
                << tags::yellow_bold
                << "\n/\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\\n"
                << "-----------------------------\n"
                << "Exiting sub-file '" << srcPath << "'\n"
                << "-----------------------------\n\n"
                << tags::reset << std::flush;
        }

        throw std::runtime_error(
            std::string("Macro failure: ") + e.what());
    }

    if (settings.debug)
    {
        std::cout << tags::yellow_bold
                  << "/\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\ /\\\n"
                  << "-----------------------------\n"
                  << "Exiting sub-file '" << srcPath << "'\n"
                  << "-----------------------------\n\n"
                  << tags::reset << std::flush;
    }

    settings.compiled.insert(Name);

    return;
}

std::string callMacro(const std::string &Name,
                      const std::list<std::string> &Args,
                      AcornSettings &settings)
{
    if (settings.compiled.count(Name) == 0)
    {
        compileMacro(Name, settings);
    }

    std::string outputName =
        COMPILED_PATH + std::string("__oak_macro_out") + ".txt";

    std::string command =
        COMPILED_PATH +
        purifyStr(Name.substr(0, Name.size() - 1)) + ".out ";

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

    if (settings.debug)
    {
        std::cout << "Macro call `" << command << "`\n";
    }

    std::string out = execute(command);

    if (settings.debug)
    {
        std::cout << "Macro returned\n```\n"
                  << out << "\n```\n";
    }

    return out;
}

std::string mangleStruct(
    const std::string &name,
    const std::list<std::list<std::string>> &generics)
{
    if (generics.size() == 0)
    {
        return name;
    }

    std::list<std::string> outputParts;
    outputParts.push_back(name);

    if (generics.size() != 0)
    {
        outputParts.push_back("GEN");

        // Generics here
        int i = 0;
        for (const auto &raw : generics)
        {
            std::string s = mangle(raw);
            outputParts.push_back(s);

            if (i + 1 < generics.size() && s != "PTR" &&
                s != "GEN" && s != "ENDGEN" && s != "JOIN" &&
                s != "")
            {
                outputParts.push_back("JOIN");
            }

            i++;
        }

        outputParts.push_back("ENDGEN");
    }

    std::string out;
    int i = 0;
    for (auto it = outputParts.begin(); it != outputParts.end();
         it++)
    {
        out += *it;

        if (i + 1 < outputParts.size())
        {
            out += "_";
        }

        i++;
    }

    return out;
}

std::string mangleEnum(
    const std::string &name,
    const std::list<std::list<std::string>> &generics)
{
    return mangleStruct(name, generics);
}

std::string mangleType(const Type &type)
{
    std::list<std::string> outputParts;

    for (int i = 0; i < type.size(); i++)
    {
        switch (type[i].info)
        {
        case sarr:
            outputParts.push_back("SARR");
            outputParts.push_back(type[i].name);
            break;
        case arr:
            outputParts.push_back("ARR");
            break;
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
            break;
        default:
            outputParts.push_back(type[i].name);
        }
    }

    std::string out;
    int i = 0;
    for (auto it = outputParts.begin(); it != outputParts.end();
         it++)
    {
        out += *it;

        if (i + 1 < outputParts.size())
        {
            out += "_";
        }

        i++;
    }

    return out;
}

std::string mangleSymb(const std::string &name, Type &type)
{
    std::string typeStr = mangleType(type);
    return mangleSymb(name, typeStr);
}

std::string mangleSymb(const std::string &name,
                       const std::string &typeStr)
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

std::string mangle(const std::list<std::string> &what)
{
    std::list<std::string> outputParts;

    for (auto s : what)
    {
        if (s == "^")
        {
            outputParts.push_back("PTR");
        }
        else if (s == "[]")
        {
            outputParts.push_back("ARR");
        }
        else if (s == "[")
        {
            // Sized ARR- takes one arg after
            outputParts.push_back("SARR");
        }
        else if (s == "]")
        {
            ;
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
        else if (s != "")
        {
            outputParts.push_back(s);
        }
    }

    std::string out;
    int i = 0;
    for (auto it = outputParts.begin(); it != outputParts.end();
         it++)
    {
        out += *it;

        if (i + 1 < outputParts.size())
        {
            out += "_";
        }

        i++;
    }

    return out;
}

ASTNode getAllocSequence(Type &type, const std::string &name,
                         AcornSettings &settings,
                         const std::string &num)
{
    // Assumes that name is a pointer to type which already
    // exists in scope
    //

    ASTNode out;
    out.info = code_line;
    out.type = nullType;

    // name = (type *)malloc(len * sizeof(type));
    out.items.push_back(ASTNode{
        nullType, std::list<ASTNode>(), atom,
        name + " = (" + toStrC(&type, settings) +
            " *)malloc(sizeof(" + toStrC(&type, settings) +
            ") * " + num + ")"});

    return out;
}

ASTNode getFreeSequence(const std::string &name,
                        AcornSettings &settings)
{
    ASTNode out;
    out.info = code_line;
    out.type = nullType;
    out.items.clear();

    out.items.push_back(ASTNode{nullType, std::list<ASTNode>(),
                                atom, "free(" + name + ")"});

    return out;
}

// Get size of file or path in kilobytes (recursive)
unsigned long long int getSize(const std::string &FilePath)
{
    if (!fs::exists(FilePath))
    {
        return 0;
    }
    else if (fs::is_directory(FilePath))
    {
        unsigned long long int out = 0;
        for (auto &item : fs::directory_iterator(FilePath))
        {
            out += getSize(item.path().string());
        }
        return out;
    }
    else
    {
        return fs::file_size(FilePath) / 1'000;
    }
}

// Karl Marx gave the people eleven zeppelins, yo
std::string humanReadable(const unsigned long long int &Size)
{
    if (Size < 1'000)
    {
        // in kb
        return std::to_string(Size) + "K";
    }
    else if (Size < 1'000'000)
    {
        // in mb
        return std::to_string(Size / 1'000.0) + "M";
    }
    else if (Size < 1'000'000'000)
    {
        // in gb
        return std::to_string(Size / 1'000'000.0) + "G";
    }
    else if (Size < 1'000'000'000'000)
    {
        // in tb
        return std::to_string(Size / 1'000'000'000.0) + "T";
    }
    else if (Size < 1'000'000'000'000'000)
    {
        // in pb
        return std::to_string(Size / 1'000'000'000'000.0) + "P";
    }
    else
    {
        // in eb
        return std::to_string(Size / 1'000'000'000'000'000.0) +
               "E";
    }
}

/*
Erases any non-function symbols which were not present
in the original table. However, skips all functions.
If not contradicted by the above rules, bases off of
the current table (not backup).
*/
std::list<std::pair<std::string, std::string>>
restoreSymbolTable(MultiSymbolTable &backup,
                   MultiSymbolTable &realTable)
{
    std::list<std::pair<std::string, std::string>> out;

    MultiSymbolTable newTable;
    for (auto p : realTable)
    {
        for (auto s : p.second)
        {
            // Functions are always added- the logic for
            // this is handled elsewhere.
            if (s.type[0].info == function)
            {
                // Add to new table for sure
                newTable[p.first];
                newTable[p.first].push_back(s);
            }

            // Variables are more complicated
            else
            {
                // Check for presence in backup
                bool present = false;
                for (auto cand : backup[p.first])
                {
                    if (cand.type == s.type)
                    {
                        present = true;
                        break;
                    }
                }

                // If was present in backup, add for sure
                if (present)
                {
                    // Add to table for sure
                    newTable[p.first];
                    newTable[p.first].push_back(s);
                }

                // Otherwise, do not add (do destructor literal
                // check)
                else
                {
                    // Variable falling out of scope
                    // Do not call Del if is atomic literal
                    if (!(s.type[0].info == atomic &&
                          (s.type[0].name == "u8" ||
                           s.type[0].name == "i8" ||
                           s.type[0].name == "u16" ||
                           s.type[0].name == "i16" ||
                           s.type[0].name == "u32" ||
                           s.type[0].name == "i32" ||
                           s.type[0].name == "u64" ||
                           s.type[0].name == "i64" ||
                           s.type[0].name == "u128" ||
                           s.type[0].name == "i128" ||
                           s.type[0].name == "f32" ||
                           s.type[0].name == "f64" ||
                           s.type[0].name == "f128" ||
                           s.type[0].name == "bool" ||
                           s.type[0].name == "str")) &&
                        s.type[0].info != function &&
                        s.type[0].info != pointer &&
                        s.type[0].info != arr &&
                        s.type[0].info != sarr && p.first != "")
                    {
                        // Del_FN_PTR_typename_MAPS_void
                        out.push_back(std::make_pair(
                            p.first, "Del_FN_PTR_" +
                                         s.type[0].name +
                                         "_MAPS_void(&" +
                                         p.first + ");"));
                    }
                }
            }
        }
    }

    realTable = newTable;

    return out;
}
