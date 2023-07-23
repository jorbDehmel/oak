/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "acorn_resources.hpp"
using namespace std;

// Settings
bool debug = false;
bool compile = true;
bool link = true;
bool pretty = false;
bool alwaysDump = false;

set<string> visitedFiles;
set<string> cppSources;
set<string> objects;

map<string, string> preprocDefines;

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

    if (lexed[i].size() > 2 && lexed[i].substr(0, 2) == "//")
    {
        lexed.erase(lexed.begin() + i);
    }

    parse_assert(lexed[i] == "(");
    lexed.erase(lexed.begin() + i);

    if (lexed[i].size() > 2 && lexed[i].substr(0, 2) == "//")
    {
        lexed.erase(lexed.begin() + i);
    }

    string cur = "";
    while (!lexed.empty() && lexed[i] != ";")
    {
        if (lexed[i] == "," || lexed[i] == ")")
        {
            out.push_back(cur);
            cur = "";
        }
        else if (lexed[i].size() < 3 || lexed[i].substr(0, 2) != "//")
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
    unsigned long long oldLineNum = curLine;
    string oldFile = curFile;

    curLine = 1;
    curFile = From;

    vector<string> lexed;

    preprocDefines["__PREV_FILE__"] = (oldFile == "" ? "\"NULL\"" : ("\"" + oldFile + "\""));
    preprocDefines["__FILE__"] = '"' + From + '"';
    preprocDefines["__COMP_TIME__"] = to_string(time(NULL));

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

            // This one goes out to quine writers (myself included)
            string cleanedText;
            for (char c : text)
            {
                if (c == '"' || c == '\'' || c == '\\')
                {
                    cleanedText += "\\";
                }

                if (c == '\n')
                {
                    cleanedText += "\\n";
                }
                else
                {
                    cleanedText += c;
                }
            }
            preprocDefines["__CONTENTS__"] = "\"" + cleanedText + "\"";

            // B: Lex
            lexed = lex(text);

            // Pre-C: Preproc definitions
            preprocDefines["__LINE__"] = "1";
            for (int i = 0; i < lexed.size(); i++)
            {
                if (lexed[i].size() >= 2 && lexed[i].substr(0, 2) == "//")
                {
                    // Line update special symbol
                    string newLineNum = lexed[i].substr(11);
                    preprocDefines["__LINE__"] = newLineNum;
                }
                else if (preprocDefines.count(lexed[i]) != 0)
                {
                    vector<string> lexedDef = lex(preprocDefines[lexed[i]]);
                    lexed.erase(lexed.begin() + i);

                    for (int j = lexedDef.size() - 1; j >= 0; j--)
                    {
                        lexed.insert(lexed.begin() + i, lexedDef[j]);
                    }

                    i--;
                }
            }

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

                    contents = "let main(argc: i16, argv: ^str) -> i16 ";

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
                        // Special cases; Memory macros
                        if (lexed[i] == "alloc!" || lexed[i] == "free!" || lexed[i] == "free_arr!")
                        {
                            continue;
                        }

                        string name = lexed[i];
                        vector<string> args = getMacroArgs(lexed, i);
                        callMacro(name, args, debug);
                    }
                }
            } while (foundMacros);

            // F: Operator substitution (within parenthesis and between commas)
            parenSub(lexed);

            // G: Load file stuff
            sequence fileSeq = createSequence(lexed);

            if (fileSeq.type != nullType)
            {
                cout << tags::yellow_bold
                     << "Warning! File '" << From << "' has hanging type '"
                     << toStr(&fileSeq.type) << "'\n"
                     << tags::reset;
            }

            if (alwaysDump)
            {
                string name = "oak_dump_" + purifyStr(From) + ".log";

                if (debug)
                {
                    cout << "Saving dump file '" << name << "'\n";
                }

                dump(lexed, name, From, curLine, fileSeq);
            }
        }
        else if (debug)
        {
            cout << "Skipping repeated file '" << From << "'\n";
        }
    }
    catch (runtime_error &e)
    {
        cout << tags::red_bold
             << "Caught error '" << e.what() << "'\n"
             << tags::reset;

        string name = "oak_dump_" + purifyStr(From) + ".log";
        cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, curLine, sequence());

        throw runtime_error("Failure in file '" + From + "'");
    }
    catch (...)
    {
        string name = "oak_dump_" + purifyStr(From) + ".log";
        cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, curLine, sequence());

        throw runtime_error("Unknown failure in file '" + From + "'");
    }

    curLine = oldLineNum;
    curFile = oldFile;

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
        if (curLine == ";")
        {
            continue;
        }

        if (curLine == "}")
        {
            indentation--;
        }

        string tabbing;
        for (int i = 0; i < indentation; i++)
        {
            tabbing += '\t';
        }
        out << tabbing;

        while (curLine.find(" ;") != string::npos)
        {
            curLine.replace(curLine.find(" ; "), 3, ";\n" + tabbing);
        }

        out << curLine << '\n';

        if (curLine.size() != 0 && (curLine[0] == '#' || curLine[0] == '}'))
        {
            out << '\n';
        }

        if (curLine == "{")
        {
            indentation++;
        }
    }

    out.close();

    return;
}
