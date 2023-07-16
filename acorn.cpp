/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>

#include "auto-oak.hpp"
#include "lexer.hpp"
#include "macros.hpp"
#include "reconstruct.hpp"
#include "symbol-table.hpp"
#include "type-builder.hpp"
#include "sequence.hpp"
#include "packages.hpp"

#include "tags.hpp"
#include "sizer.hpp"

using namespace std;

#define VERSION "0.0.0"
#define LICENSE "GPLv3"
#define INFO "jdehmel@outlook.com"

#define DASHED_LINE "------------------------------------------\n"
string helpText = "Acorn - Oak Standard Translator\n" DASHED_LINE
                  "Translates .oak files to .cpp.\n"
                  "Option | Verbose     | Purpose\n" DASHED_LINE
                  " -h    | --help      | Show this\n"
                  " -v    | --version   | Show version\n"
                  " -d    | --debug     | Toggle debug mode\n"
                  " -o    | --output    | Set the output file\n"
                  " -n    | --no_save   | Produce nothing\n"
                  " -t    | --translate | Produce C++ files\n"
                  " -c    | --compile   | Produce object files\n"
                  " -l    | --link      | Produce executables\n"
                  " -e    | --clean     | Work from scratch\n"
                  " -q    | --quit      | Quit immediately\n"
                  " -p    | --pretty    | Prettify C++ files\n"
                  " -i    | --install   | Install a package\n"
                  " -r    | --reinstall | Reinstall a package\n"
                  " -s    | --size      | Show Oak disk usage\n";

bool debug = false;

// Translate -> compile -> link
bool compile = true;
bool link = true;

bool pretty;

set<string> visitedFiles;

set<string> cppSources;
set<string> objects;

// Prints the cumulative disk usage of Oak (in human-readable)
void getDiskUsage()
{
    const vector<string> filesToCheck = {
        "/usr/bin/acorn",
        "/usr/include/oak"};
    unsigned long long int totalKB = 0;

    for (string s : filesToCheck)
    {
        unsigned long long int currentSize = getSize(s);
        totalKB += currentSize;

        cout << s << ": " << humanReadable(currentSize) << '\n';
    }

    cout << "\nTotal: " << humanReadable(totalKB) << '\n';
    return;
}

// I is the point in Lexed at which a macro name was found
// CONSUMPTIVE!
vector<string> getMacroArgs(vector<string> &lexed, const int &i)
{
    vector<string> out;

    lexed.erase(lexed.begin() + i);
    parse_assert(lexed[i] == "(");
    lexed.erase(lexed.begin() + i);

    string cur = "";
    while (!lexed.empty() && lexed[i] != ";")
    {
        if (lexed[i] == "," || lexed[i] == ")")
        {
            out.push_back(cur);
            cur = "";
        }
        else
        {
            cur += lexed[i];
        }

        lexed.erase(lexed.begin() + i);
    }

    if (!lexed.empty())
    {
        lexed.erase(lexed.begin() + i);
    }

    return out;
}

void doFile(const string &From)
{
    try
    {
        if (From.size() < 4 || From.substr(From.size() - 4) != ".oak")
        {
            cout << tags::yellow_bold
                 << "Warning! File '" << From << "' is not a .oak file.\n"
                 << tags::reset;
        }

        string realName;
        if (From.find("/") == string::npos)
        {
            realName = From;
        }
        else
        {
            realName = From.substr(From.find("/") + 1);
        }

        for (char c : realName)
        {
            if ('A' <= c && c <= 'Z')
            {
                cout << tags::yellow_bold
                     << "Warning! File '" << From << "' has illegal name.\n"
                     << "Oak files use underscore formatting (ie /path/to/file_to_use.oak).\n"
                     << tags::reset;
                break;
            }
        }

        if (visitedFiles.count(From) == 0)
        {
            visitedFiles.insert(From);

            if (debug)
            {
                cout << "Loading file '" << From << "'\n";
            }

            if (From.size() > 4 && From.substr(From.size() - 4) == ".cpp")
            {
                cppSources.insert(From);
                autoOak(From);
                return;
            }

            // A: Load file
            ifstream file(From);
            if (!file.is_open())
            {
                throw runtime_error("Could not open source file '" + From + "'");
            }

            string text, line;
            while (getline(file, line))
            {
                text += line + '\n';
            }

            // B: Lex
            vector<string> lexed = lex(text);

            // C: Scan for compiler macros; Do these first
            // This erases them from lexed
            for (int i = 0; i < lexed.size(); i++)
            {
                if (lexed[i] != "!" && lexed[i].back() == '!')
                {
                    if (lexed[i] == "include!")
                    {
                        vector<string> args = getMacroArgs(lexed, i);

                        // Clean arguments
                        for (int j = 0; j < args.size(); j++)
                        {
                            while (!args[j].empty() && (args[j].front() == '"' || args[j].front() == '\''))
                            {
                                args[j].erase(0, 1);
                            }
                            while (!args[j].empty() && (args[j].back() == '"' || args[j].back() == '\''))
                            {
                                args[j].pop_back();
                            }
                        }

                        for (string a : args)
                        {
                            doFile(a);
                        }
                        i--;
                    }
                    else if (lexed[i] == "link!")
                    {
                        vector<string> args = getMacroArgs(lexed, i);

                        // Clean arguments
                        for (int j = 0; j < args.size(); j++)
                        {
                            while (!args[j].empty() && (args[j].front() == '"' || args[j].front() == '\''))
                            {
                                args[j].erase(0, 1);
                            }
                            while (!args[j].empty() && (args[j].back() == '"' || args[j].back() == '\''))
                            {
                                args[j].pop_back();
                            }
                        }

                        for (string a : args)
                        {
                            if (debug)
                            {
                                cout << "Inserting object " << a << '\n';
                            }

                            objects.insert(a);
                        }
                        i--;
                    }
                    else if (lexed[i] == "package!")
                    {
                        vector<string> args = getMacroArgs(lexed, i);
                        vector<string> files;

                        // Clean arguments
                        for (int j = 0; j < args.size(); j++)
                        {
                            while (!args[j].empty() && (args[j].front() == '"' || args[j].front() == '\''))
                            {
                                args[j].erase(0, 1);
                            }
                            while (!args[j].empty() && (args[j].back() == '"' || args[j].back() == '\''))
                            {
                                args[j].pop_back();
                            }
                        }

                        for (string a : args)
                        {
                            files = getPackageFiles(a);
                            for (string f : files)
                            {
                                if (debug)
                                {
                                    cout << "Loading package file '" << f << "'...\n";
                                }

                                doFile(f);
                            }
                        }
                        i--;
                    }
                }
            }

            // D: Scan for macro definitions and handle them
            // This erases them from lexed
            for (int i = 1; i < lexed.size(); i++)
            {
                if (lexed[i] != "!" && lexed[i].back() == '!' && lexed[i - 1] == "let")
                {
                    string name, contents;
                    name = lexed[i];

                    contents = "let main(argc: i16, argv: *str) -> i16 ";

                    lexed.erase(lexed.begin() + i);       // Name
                    lexed.erase(lexed.begin() + (i - 1)); // Let

                    // let name!(argc: u16, argv: *str) { ... }

                    while (lexed[i] != "{" && lexed[i] != ";")
                    {
                        lexed.erase(lexed.begin() + i);
                    }

                    if (lexed[i] == ";")
                    {
                        contents += "{ 0 }";
                    }
                    else
                    {
                        int count = 1;
                        lexed.erase(lexed.begin() + i);
                        contents += "\n{";

                        while (count != 0)
                        {
                            if (lexed[i] == "{")
                            {
                                count++;
                            }
                            else if (lexed[i] == "}")
                            {
                                count--;
                            }

                            contents += " " + lexed[i];
                            lexed.erase(lexed.begin() + i);
                        }
                    }

                    macros[name] = contents;
                }
            }

            // E: Scan for macro calls and handle them
            bool foundMacros = false;
            do
            {
                for (int i = 0; i < lexed.size(); i++)
                {
                    // We can assume that no macro definitions remain
                    if (lexed[i] != "!" && lexed[i].back() == '!')
                    {
                        string name = lexed[i];
                        vector<string> args = getMacroArgs(lexed, i);
                        callMacro(name, args, debug);
                    }
                }
            } while (foundMacros);

            // F: Load file stuff
            sequence fileSeq = createSequence(lexed);
            if (fileSeq.type != nullType)
            {
                cout << tags::yellow_bold
                     << "Warning! File '" << From << "' has hanging type '"
                     << toStr(&fileSeq.type) << "'\n"
                     << tags::reset;
            }
        }
        else if (debug)
        {
            cout << "Skipping repeated file '" << From << "'\n";
        }

        if (debug)
        {
            cout << "Finished file '" << From << "'\n";
        }
    }
    catch (runtime_error &e)
    {
        throw runtime_error("Failure in file '" + From + "':\n" + e.what());
    }

    return;
}

void prettify(const string &Filename)
{
    ifstream inp(Filename);
    if (!inp.is_open())
    {
        cout << tags::yellow_bold
             << "Failed to prettify file '" << Filename << "'\n"
             << tags::reset;
        return;
    }

    string line;
    vector<string> lines;
    while (getline(inp, line))
    {
        lines.push_back(line);
    }

    inp.close();

    ofstream out(Filename);
    if (!out.is_open())
    {
        cout << tags::yellow_bold
             << "Failed to prettify file '" << Filename << "'\n"
             << tags::reset;
        return;
    }

    int indentation = 0;
    for (string curLine : lines)
    {
        for (int i = 0; i < indentation; i++)
        {
            out << '\t';
        }

        out << curLine << '\n';

        if (curLine == "{")
        {
            indentation++;
        }
        else if (curLine == "}")
        {
            indentation--;
        }
    }

    out.close();

    return;
}

int main(const int argc, const char *argv[])
{
    // Necessary in other places; I just need to not forget to run this
    setUpInverseOperatorAliases();

    auto start = chrono::high_resolution_clock::now(), end = start;
    unsigned long long int oakElapsed = 0;

    vector<string> files;
    string out = "a.out";
    bool noSave = false;
    bool eraseTemp = false;

    try
    {
        // Argument parsing
        vector<string> filesToAdd;
        for (int i = 1; i < argc; i++)
        {
            string cur = argv[i];
            if (cur == "")
            {
                continue;
            }

            if (cur[0] == '-')
            {
                if (cur[1] == '-')
                {
                    // Verbose options
                    if (cur == "--help")
                    {
                        cout << helpText << '\n'
                             << "Version: " << VERSION << '\n'
                             << "License: " << LICENSE << '\n'
                             << INFO << '\n';
                    }
                    else if (cur == "--version")
                    {
                        cout << "Version: " << VERSION << '\n'
                             << "License: " << LICENSE << '\n'
                             << INFO << '\n';
                    }
                    else if (cur == "--debug")
                    {
                        debug = !debug;
                        cout << "Set debug to " << (debug ? "true" : "false") << '\n';
                    }
                    else if (cur == "--no_save")
                    {
                        noSave = !noSave;

                        if (noSave)
                        {
                            compile = link = false;
                        }
                    }
                    else if (cur == "--translate")
                    {
                        noSave = compile = link = false;
                    }
                    else if (cur == "--compile")
                    {
                        compile = true;
                        noSave = link = false;
                    }
                    else if (cur == "--link")
                    {
                        noSave = false;
                        compile = link = true;
                    }
                    else if (cur == "--clean")
                    {
                        system("rm -rf .oak_build");
                        eraseTemp = !eraseTemp;
                    }
                    else if (cur == "--quit")
                    {
                        return 0;
                    }
                    else if (cur == "--output")
                    {
                        if (i + 1 >= argc)
                        {
                            throw runtime_error("--output must be followed by a filename");
                        }

                        out = argv[i + 1];
                        i++;
                    }
                    else if (cur == "--pretty")
                    {
                        pretty = !pretty;
                    }
                    else if (cur == "--install")
                    {
                        if (i + 1 >= argc)
                        {
                            throw runtime_error("--install must be followed by a package name");
                        }

                        downloadPackage(argv[1 + 1]);

                        i++;
                    }
                    else if (cur == "--reinstall")
                    {
                        if (i + 1 >= argc)
                        {
                            throw runtime_error("--reinstall must be followed by a package name");
                        }

                        downloadPackage(argv[1 + 1], true);

                        i++;
                    }
                    else if (cur == "--size")
                    {
                        getDiskUsage();
                    }
                    else
                    {
                        throw runtime_error(string("Invalid verbose option '") + cur + "'");
                    }
                }
                else
                {
                    // Compact options

                    // Iterate through single-letter options
                    string remainder = cur.substr(1);
                    for (char c : remainder)
                    {
                        switch (c)
                        {
                        case 'h':
                            cout << helpText << '\n';
                        case 'v':
                            cout << "Version: " << VERSION << '\n'
                                 << "License: " << LICENSE << '\n'
                                 << INFO << '\n';
                            break;
                        case 'e':
                            system("rm -rf .oak_build");
                            break;
                        case 'd':
                            debug = !debug;
                            cout << "Set debug to " << (debug ? "true" : "false") << '\n';
                            break;
                        case 'o':
                            if (i + 1 >= argc)
                            {
                                throw runtime_error("-o must be followed by a filename");
                            }

                            out = argv[i + 1];
                            i++;
                            break;
                        case 't':
                            noSave = compile = link = false;
                        case 'c':
                            compile = true;
                            noSave = link = false;
                            break;
                        case 'l':
                            noSave = false;
                            compile = link = true;
                            break;
                        case 'q':
                            return 0;
                            break;
                        case 'p':
                            pretty = !pretty;
                            break;
                        case 'n':
                            noSave = !noSave;

                            if (noSave)
                            {
                                compile = link = false;
                            }
                            break;
                        case 'i':
                            if (i + 1 >= argc)
                            {
                                throw runtime_error("-i must be followed by a package name");
                            }

                            downloadPackage(argv[i + 1]);

                            i++;
                            break;
                        case 'r':
                            if (i + 1 >= argc)
                            {
                                throw runtime_error("-r must be followed by a package name");
                            }

                            downloadPackage(argv[i + 1], true);

                            i++;
                            break;
                        case 's':
                            getDiskUsage();
                            break;

                        default:
                            throw runtime_error(string("Invalid option '") + c + "'");
                            break;
                        }
                    }
                }
            }
            else
            {
                files.push_back(cur);
            }
        }

        if (files.empty())
        {
            throw runtime_error("No input files");
        }

        // Actual calls
        if (debug)
        {
            cout << tags::green_bold
                 << "\nPhase 1: File analysis\n"
                 << tags::reset;
        }

        int i = 0;
        for (auto f : files)
        {
            if (debug)
            {
                cout << 100.0 * (i / (double)files.size()) << "% done with initial files.\n";
            }

            doFile(f);
            i++;
        }

        // Reconstruct and save
        if (debug)
        {
            cout << tags::green_bold
                 << "\nLoaded all " << files.size() << " initial files.\n"
                 << visitedFiles.size() - files.size() << " more files were included,\n"
                 << "For " << visitedFiles.size() << " total files.\n"
                 << tags::reset;

            cout << tags::green_bold
                 << "\nPhase 2: Reconstruction.\n"
                 << tags::reset;
        }

        if (table.count("main") == 0)
        {
            cout << tags::yellow_bold
                 << "Warning! No main function detected!\n"
                 << tags::reset;
        }

        smartSystem("mkdir -p .oak_build");

        /*
        if (out.find(".oak_build/") == string::npos)
        {
            out = ".oak_build/" + out;
        }
        */

        pair<string, string> names = reconstructAndSave(out);
        cppSources.insert(names.second);

        end = chrono::high_resolution_clock::now();
        oakElapsed = chrono::duration_cast<chrono::nanoseconds>(end - start).count();

        if (debug)
        {
            cout << "Output header: '" << names.first << "'\n"
                 << "Output body:   '" << names.second << "'\n";
        }

        if (noSave)
        {
            int result = system((string("rm ") + names.first + " " + names.second).c_str());

            if (result != 0)
            {
                cout << tags::yellow_bold
                     << "Warning: Could not erase generated files.\n"
                     << tags::reset;
            }
            else if (debug)
            {
                cout << "Erased output files.\n";
            }
        }
        else
        {
            if (compile)
            {
                smartSystem("mkdir -p .oak_build");

                if (debug)
                {
                    cout << tags::green_bold
                         << "\nPhase 3: Compilation.\n"
                         << "(via Clang)\n"
                         << tags::reset;
                }

                for (string source : cppSources)
                {
                    string command = "clang++ -pedantic -Wall -c " + source + " -o " + source + ".o";

                    if (debug)
                    {
                        cout << "System call `" << command << "`\n";
                    }

                    smartSystem(command);
                    objects.insert(source + ".o");
                }

                if (link)
                {
                    if (debug)
                    {
                        cout << tags::green_bold
                             << "\nPhase 4: Linking.\n"
                             << "(via Clang)\n"
                             << tags::reset;
                    }

                    string command = "clang++ -pedantic -Wall -o " + out + " ";
                    for (string object : objects)
                    {
                        command += object + " ";
                    }

                    if (debug)
                    {
                        cout << "System call `" << command << "`\n";
                    }

                    smartSystem(command);
                }
            }
        }

        // Prettification of C++ files
        if (pretty)
        {
            prettify(names.second);
        }

        if (eraseTemp)
        {
            cout << "rm -rf .oak_build\n";
            int result = system("rm -rf .oak_build");
            if (result != 0)
            {
                cout << tags::yellow_bold
                     << "Warning! Failed to erase './.oak_build/'.\n"
                     << "If left unfixed, this could cause problems.\n"
                     << tags::reset;
            }
        }
    }
    catch (parse_error &e)
    {
        if (oakElapsed != 0)
        {
            cout << "Ellapsed Oak ns: " << oakElapsed;
        }

        cout << tags::red_bold
             << "\nA parsing error occurred with message:\n"
             << e.what()
             << "\n"
             << tags::reset;

        return 1;
    }
    catch (runtime_error &e)
    {
        if (oakElapsed != 0)
        {
            cout << "Ellapsed Oak ns: " << oakElapsed;
        }

        cout << tags::red_bold
             << "\nA runtime error occurred with message:\n"
             << e.what()
             << "\n"
             << tags::reset;

        return 2;
    }
    catch (...)
    {
        if (oakElapsed != 0)
        {
            cout << "Ellapsed Oak ns: " << oakElapsed;
        }

        cout << tags::red_bold
             << "\nAn unknown error ocurred.\n"
             << tags::reset;

        return 3;
    }

    if (debug)
    {
        auto size = getSize(".oak_build");

        if (size != 0)
        {
            cout << tags::green_bold
                 << "\nThe build process took up "
                 << humanReadable(size)
                 << " of local storage.\n"
                 << "(This is stored in './.oak_build/', which is now safe to delete.)\n"
                 << tags::reset;
        }
    }

    if (debug)
    {
        end = chrono::high_resolution_clock::now();
        unsigned long long totalElapsed = chrono::duration_cast<chrono::nanoseconds>(end - start).count();

        float percentAcornTime = oakElapsed / (float)totalElapsed;
        percentAcornTime *= 100;

        cout << tags::green_bold
             << "\nProcess finished with no errors.\n\n"
             << tags::violet_bold
             << "Acorn-attributable time:\n"
             << "Nanoseconds:  " << oakElapsed << '\n'
             << "Microseconds: " << oakElapsed / 1'000.0 << '\n'
             << "Milliseconds: " << oakElapsed / 1'000'000.0 << '\n'
             << "Seconds:      " << oakElapsed / 1'000'000'000.0 << "\n\n"
             << "Total time:\n"
             << "Nanoseconds:  " << totalElapsed << '\n'
             << "Microseconds: " << totalElapsed / 1'000.0 << '\n'
             << "Milliseconds: " << totalElapsed / 1'000'000.0 << '\n'
             << "Seconds:      " << totalElapsed / 1'000'000'000.0 << "\n\n"
             << "Percent of time which was Acorn-attributable: "
             << percentAcornTime
             << "%\n\n"
             << tags::reset;
    }

    cout << "Output file: " << out << '\n';

    return 0;
}
