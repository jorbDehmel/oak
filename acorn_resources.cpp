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
bool doLink = true;
bool pretty = false;
bool alwaysDump = false;
bool manual = false;
bool ignoreSyntaxErrors = false;
bool timeAnalysis = false;

set<string> visitedFiles;
set<string> cppSources;
set<string> objects;
set<string> cflags;

map<string, string> preprocDefines;
vector<unsigned long long> phaseTimes;

string debugTreePrefix = "";

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

void doFile(const string &From)
{
    while (phaseTimes.size() < NUM_PHASES)
    {
        phaseTimes.push_back(0);
    }
    int curPhase = 0;

    auto start = chrono::high_resolution_clock::now();
    auto end = start;

    // Clear active rules
    activeRules.clear();

    unsigned long long oldLineNum = curLine;
    string oldFile = curFile;

    curLine = 1;
    curFile = From;

    vector<string> lexed, lexedCopy;

    preprocDefines["__PREV_FILE__"] = (oldFile == "" ? "\"NULL\"" : ("\"" + oldFile + "\""));
    preprocDefines["__FILE__"] = '"' + From + '"';
    preprocDefines["__COMP_TIME__"] = to_string(time(NULL));

    // System defines
    /*
    Options:
    -WINDOWS
    -LINUX
    -MAC
    -UNKNOWN
    */
#if (defined(_WIN32) || defined(_WIN64))
    preprocDefines["__SYS__"] = "WINDOWS";
#elif (defined(LINUX) || defined(__linux__))
    preprocDefines["__SYS__"] = "LINUX";
#elif (defined(__APPLE__))
    preprocDefines["__SYS__"] = "MAC";
#else
    preprocDefines["__SYS__"] = "UNKNOWN";
#endif

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

        if (visitedFiles.count(realName) == 0)
        {
            visitedFiles.insert(realName);

            if (debug)
            {
                cout << debugTreePrefix << "Loading file '" << From << "'\n";
                debugTreePrefix.append("|");
            }

            // A - 2: Load file
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

            // A: Syntax check
            start = chrono::high_resolution_clock::now();

            if (!ignoreSyntaxErrors)
            {
                ensureSyntax(text, true);
            }

            end = chrono::high_resolution_clock::now();
            phaseTimes[curPhase] += chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            curPhase++;

            // B: __CONTENTS__ macro
            start = chrono::high_resolution_clock::now();

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

            end = chrono::high_resolution_clock::now();
            phaseTimes[curPhase] += chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            curPhase++;

            // C: Lex
            start = chrono::high_resolution_clock::now();

            lexed = lex(text);
            lexedCopy = lexed;

            end = chrono::high_resolution_clock::now();
            phaseTimes[curPhase] += chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            curPhase++;

            // D: Scan for macro definitions and handle them
            // This erases them from lexed
            start = chrono::high_resolution_clock::now();

            for (int i = 1; i + 1 < lexed.size(); i++)
            {
                if (lexed[i] != "!" && lexed[i].back() == '!' && lexed[i - 1] == "let")
                {
                    if (lexed[i + 1] != "=")
                    {
                        string name, contents;
                        name = lexed[i];

                        // Safety checks
                        if (preprocDefines.count(name) != 0)
                        {
                            throw sequencing_error("Macro  '" + name + "' cannot overwrite definition of same name.");
                        }
                        else if (macros.count(name) != 0)
                        {
                            throw sequencing_error("Macro '" + name + "' cannot be overridden.");
                        }

                        contents = "let main(argc: i32, argv: ^^i8) -> i32 ";

                        i--;
                        lexed.erase(lexed.begin() + i); // Let
                        lexed.erase(lexed.begin() + i); // Name

                        while (lexed.size() >= i && lexed[i] != "{" && lexed[i] != ";")
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

                                if (lexed[i].size() < 2 || lexed[i].substr(0, 2) != "//")
                                {
                                    contents += " " + lexed[i];
                                }
                                else
                                {
                                    contents += "\n";
                                }

                                lexed.erase(lexed.begin() + i);
                            }
                        }

                        macros[name] = contents;
                    }
                    else
                    {
                        // Preproc definition; Not a full macro

                        // Safety checks
                        if (preprocDefines.count(lexed[i]) != 0)
                        {
                            throw sequencing_error("Preprocessor definition '" + lexed[i] + "' cannot be overridden.");
                        }
                        else if (macros.count(lexed[i]) != 0)
                        {
                            throw sequencing_error("Preprocessor definition '" + lexed[i] + "' cannot overwrite macro of same name.");
                        }

                        string name = lexed[i];

                        // Erase as needed
                        i--;
                        lexed.erase(lexed.begin() + i); // Let
                        lexed.erase(lexed.begin() + i); // Name!
                        lexed.erase(lexed.begin() + i); // =

                        // Scrape until next semicolon
                        string contents = "";
                        while (lexed[i] != ";")
                        {
                            contents.append(lexed[i]);
                            lexed.erase(lexed.begin() + i);
                        }

                        lexed.erase(lexed.begin() + i); // ;

                        // Insert
                        preprocDefines[name] = contents;
                    }
                }
            }

            end = chrono::high_resolution_clock::now();
            phaseTimes[curPhase] += chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            curPhase++;

            // E: Preproc definitions
            start = chrono::high_resolution_clock::now();

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

            end = chrono::high_resolution_clock::now();
            phaseTimes[curPhase] += chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            curPhase++;

            // F: Scan for compiler macros; Do these first
            // This erases them from lexed
            start = chrono::high_resolution_clock::now();
            unsigned long long int recurseTime = 0;

            for (int i = 0; i < lexed.size(); i++)
            {
                if (lexed[i] != "!" && lexed[i].back() == '!')
                {
                    // File handling / translation unit macros
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
                            auto recStart = chrono::high_resolution_clock::now();
                            doFile(OAK_DIR_PATH + a);
                            auto recEnd = chrono::high_resolution_clock::now();
                            recurseTime += chrono::duration_cast<chrono::nanoseconds>(recEnd - recStart).count();
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

                            if (!args[j].empty() && args[j][0] != '-')
                            {
                                args[j] = OAK_DIR_PATH + args[j];
                            }
                        }

                        for (string a : args)
                        {
                            if (debug)
                            {
                                cout << debugTreePrefix << "Inserting object " << a << '\n';
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
                            auto recStart = chrono::high_resolution_clock::now();
                            files = getPackageFiles(a);
                            auto recEnd = chrono::high_resolution_clock::now();
                            recurseTime += chrono::duration_cast<chrono::nanoseconds>(recEnd - recStart).count();

                            for (string f : files)
                            {
                                if (debug)
                                {
                                    cout << debugTreePrefix << "Loading package file '" << f << "'...\n";
                                }

                                auto recStart = chrono::high_resolution_clock::now();
                                doFile(f);
                                auto recEnd = chrono::high_resolution_clock::now();
                                recurseTime += chrono::duration_cast<chrono::nanoseconds>(recEnd - recStart).count();
                            }
                        }
                        i--;
                    }
                    else
                    {
                        // Non-compiler macro definition
                        // Nested stuff may be allowed within
                        if (i > 0 && lexed[i - 1] == "let")
                        {
                            while (lexed.size() >= i && lexed[i] != "{" && lexed[i] != ";")
                            {
                                i++;
                            }

                            if (lexed[i] == ";")
                            {
                                i++;
                            }
                            else
                            {
                                int count = 1;
                                i++;

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

                                    i++;
                                }
                            }
                        }
                    }
                }
            }

            end = chrono::high_resolution_clock::now();
            phaseTimes[curPhase] += chrono::duration_cast<chrono::nanoseconds>(end - start).count() - recurseTime;
            curPhase++;

            // G: Rules
            start = chrono::high_resolution_clock::now();

            doRules(lexed);

            end = chrono::high_resolution_clock::now();
            phaseTimes[curPhase] += chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            curPhase++;

            // H: Scan for macro calls and handle them
            start = chrono::high_resolution_clock::now();

            for (int i = 0; i < lexed.size(); i++)
            {
                // We can assume that no macro definitions remain
                if (lexed[i] != "!" && lexed[i].back() == '!')
                {
                    // Special cases: Memory macros
                    if (lexed[i] == "alloc!" || lexed[i] == "free!" || lexed[i] == "free_arr!")
                    {
                        continue;
                    }

                    // Another special case: Erasure macro
                    else if (lexed[i] == "erase!")
                    {
                        continue;
                    }

                    // More special cases
                    else if (lexed[i] == "c_print!")
                    {
                        continue;
                    }
                    else if (lexed[i] == "c_panic!")
                    {
                        continue;
                    }

                    // More special cases: Rule macros
                    else if (lexed[i] == "new_rule!" || lexed[i] == "use_rule!" || lexed[i] == "rem_rule!" || lexed[i] == "bundle_rule!")
                    {
                        continue;
                    }

                    string name = lexed[i];

                    // Erases all trace of the call in the process
                    vector<string> args = getMacroArgs(lexed, i);

                    string output = callMacro(name, args, debug);
                    vector<string> lexedOutput = lex(output);

                    // Reset preproc defs, as they tend to break w/ macros
                    preprocDefines["__PREV_FILE__"] = (oldFile == "" ? "\"NULL\"" : ("\"" + oldFile + "\""));
                    preprocDefines["__FILE__"] = '"' + From + '"';

                    // Remove lines
                    for (int ind = 0; ind < lexedOutput.size(); ind++)
                    {
                        if (lexedOutput[ind].size() > 1 && lexedOutput[ind].substr(0, 2) == "//")
                        {
                            if (lexedOutput[ind].size() > 11 && lexedOutput[ind].substr(0, 11) == "//__LINE__=")
                            {
                                preprocDefines["__LINE__"] = lexedOutput[ind].substr(11);
                            }

                            lexedOutput.erase(lexedOutput.begin() + ind);
                        }

                        // Preproc defines subs
                        else if (preprocDefines.count(lexedOutput[ind]) != 0)
                        {
                            vector<string> lexedDef = lex(preprocDefines[lexedOutput[ind]]);
                            lexedOutput.erase(lexedOutput.begin() + ind);

                            for (int j = lexedDef.size() - 1; j >= 0; j--)
                            {
                                lexedOutput.insert(lexedOutput.begin() + ind, lexedDef[j]);
                            }

                            ind--;
                        }
                    }

                    // Insert the new code
                    for (int j = lexedOutput.size() - 1; j >= 0; j--)
                    {
                        lexed.insert(lexed.begin() + i, lexedOutput[j]);
                    }

                    // Since we do not change i, this new code will be scanned next.
                }
            }

            end = chrono::high_resolution_clock::now();
            phaseTimes[curPhase] += chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            curPhase++;

            // I: Operator substitution (within parenthesis and between commas)
            start = chrono::high_resolution_clock::now();

            parenSub(lexed);

            end = chrono::high_resolution_clock::now();
            phaseTimes[curPhase] += chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            curPhase++;

            // J: Sequencing
            start = chrono::high_resolution_clock::now();

            sequence fileSeq = createSequence(lexed);

            end = chrono::high_resolution_clock::now();
            phaseTimes[curPhase] += chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            curPhase++;

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
                    cout << debugTreePrefix << "Saving dump file '" << name << "'\n";
                }

                dump(lexed, name, From, curLine, fileSeq, lexedCopy);
            }

            if (debug)
            {
                debugTreePrefix.pop_back();
                cout << debugTreePrefix << "Finished file '" << From << "'\n";
            }
        }
        else if (debug)
        {
            cout << debugTreePrefix << "Skipping repeated file '" << From << "'\n";
        }
    }
    catch (rule_error &e)
    {
        cout << tags::red_bold
             << "Caught rule error '" << e.what() << "'\n"
             << tags::reset;

        string name = "oak_dump_" + purifyStr(From) + ".log";
        cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, curLine, sequence(), lexedCopy);

        throw runtime_error("Failure in file '" + From + "': " + e.what());
    }
    catch (sequencing_error &e)
    {
        cout << tags::red_bold
             << "Caught sequencing error '" << e.what() << "'\n"
             << tags::reset;

        string name = "oak_dump_" + purifyStr(From) + ".log";
        cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, curLine, sequence(), lexedCopy);

        throw runtime_error("Failure in file '" + From + "': " + e.what());
    }
    catch (parse_error &e)
    {
        cout << tags::red_bold
             << "Caught parse error '" << e.what() << "'\n"
             << tags::reset;

        string name = "oak_dump_" + purifyStr(From) + ".log";
        cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, curLine, sequence(), lexedCopy);

        throw runtime_error("Failure in file '" + From + "': " + e.what());
    }
    catch (package_error &e)
    {
        cout << tags::red_bold
             << "Caught package error '" << e.what() << "'\n"
             << tags::reset;

        string name = "oak_dump_" + purifyStr(From) + ".log";
        cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, curLine, sequence(), lexedCopy);

        throw runtime_error("Failure in file '" + From + "': " + e.what());
    }
    catch (generic_error &e)
    {
        cout << tags::red_bold
             << "Caught generic error '" << e.what() << "'\n"
             << tags::reset;

        string name = "oak_dump_" + purifyStr(From) + ".log";
        cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, curLine, sequence(), lexedCopy);

        throw runtime_error("Failure in file '" + From + "': " + e.what());
    }
    catch (runtime_error &e)
    {
        cout << tags::red_bold
             << "Caught runtime error '" << e.what() << "'\n"
             << tags::reset;

        string name = "oak_dump_" + purifyStr(From) + ".log";
        cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, curLine, sequence(), lexedCopy);

        throw runtime_error("Failure in file '" + From + "': " + e.what());
    }
    catch (...)
    {
        string name = "oak_dump_" + purifyStr(From) + ".log";
        cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, curLine, sequence(), lexedCopy);

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

void makePackage(const string &RawName)
{
    string name = purifyStr(RawName);
    for (int i = 0; i < name.size(); i++)
    {
        name[i] = tolower(name[i]);
    }

    // mkdir NAME
    smartSystem("mkdir -p " + name);

    // touch NAME/oak_package_info.txt
    ofstream info(name + "/oak_package_info.txt");
    pm_assert(info.is_open(), "Could not open oak_package_info.txt in newly created package folder.");

    auto now = time(NULL);
    string time = ctime(&now);
    time.pop_back();

    /*
    VERSION = "0.0.1"
    LICENSE = "GPLv3"
    SOURCE = "github.com/jorbDehmel"
    AUTHOR = "Jordan Dehmel"
    EMAIL = "jdehmel@outlook.com"
    ABOUT = "Oak STD Package"
    SYS_DEPS = "git clang"
    */

    info << "NAME = '" << name << "'\n"
         << "DATE = '" << time << "'\n"
         << "INCLUDE = '" << name << ".oak'\n"
         << "VERSION = ''\n"
         << "LICENSE = ''\n"
         << "SOURCE = ''\n"
         << "AUTHOR = ''\n"
         << "EMAIL = ''\n"
         << "ABOUT = ''\n"
         << "SYS_DEPS = ''\n";

    info.close();

    // touch NAME/NAME.oak
    ofstream include(name + "/" + name + ".oak");
    pm_assert(include.is_open(), "Could not open main include file in newly created package folder.");
    include.close();

    ofstream ignore(name + "/.gitignore");
    pm_assert(ignore.is_open(), "Could not open git ignore file in newly created package folder.");
    ignore.close();

    // git init
    throw_assert(system(("git init " + name + " > /dev/null").c_str()) == 0);

    return;
}

void printSyntaxError(const string &what, const vector<char> &curLineVec)
{
    cout << tags::yellow_bold
         << '\n'
         << "In line '";

    for (const auto &c : curLineVec)
    {
        cout << c;
    }

    cout << "'\n"
         << tags::reset;

    cout << '\n'
         << tags::red_bold
         << "Syntax error at " << curFile << ':' << curLine << '\n'
         << what << '\n'
         << "(Use -x to make syntax errors nonfatal)"
         << tags::reset
         << "\n\n";

    return;
}

void ensureSyntax(const string &text, const bool &fatal)
{
    vector<char> curLineVec;
    curLineVec.reserve(64);
    curLine = 1;
    int commentDepth = 0;
    int errorCount = 0;

    // space for none, or ' or "
    char stringMarker = ' ';

    int ind = 0;
    for (const auto &c : text)
    {
        if (c == '\n')
        {
            // Multi-line comments
            if (curLineVec.size() >= 2 &&
                ((curLineVec[0] == '/' && curLineVec[1] == '*') ||
                 (curLineVec[0] == '*' && curLineVec[1] == '/')))
            {
                for (int i = 2; i < curLineVec.size(); i++)
                {
                    if (curLineVec[i] != ' ' && curLineVec[i] != '\t')
                    {
                        printSyntaxError("Multi-line comment deliminators (/* and */) must occupy their own line", curLineVec);
                        errorCount++;
                    }
                }

                if (curLineVec[0] == '/')
                {
                    commentDepth++;
                }
                else
                {
                    commentDepth--;
                }
            }

            // Single-line comments
            else if (curLineVec.size() >= 3 && curLineVec[0] == '/' && curLineVec[1] == '/')
            {
                if (curLineVec[2] != ' ' && curLineVec[2] != '/')
                {
                    printSyntaxError("Comments must begin with '// ' (slash-slash-SPACE) or ///", curLineVec);
                    errorCount++;
                }
            }

            // All code-line-based checks must occur herein
            else if (commentDepth == 0)
            {
                // Peel off any trailing whitespace
                // (leading whitespace should already be cleaned)
                while (!curLineVec.empty() && (curLineVec.back() == ' ' || curLineVec.back() == '\t'))
                {
                    curLineVec.pop_back();
                }

                for (const auto &c : curLineVec)
                {
                    if (c == '{' && curLineVec.size() != 1)
                    {
                        printSyntaxError("Open curly brackets must occupy their own line", curLineVec);
                        errorCount++;
                    }
                    else if (c == '}' && curLineVec.size() != 1)
                    {
                        printSyntaxError("Closing curly brackets must occupy their own line", curLineVec);
                        errorCount++;
                    }
                }
            }

            // Reset current line
            curLineVec.clear();
            curLine++;
        }
        else
        {
            if (curLineVec.size() >= 64)
            {
                printSyntaxError("No line shall may exceed 64 characters (excluding leading whitespace)", curLineVec);
                errorCount++;
            }

            if (curLineVec.size() > 0 || (c != ' ' && c != '\t'))
            {
                if (c == '"')
                {
                    if (ind == 0 || text[ind - 1] != '\\')
                    {
                        if (stringMarker == ' ')
                        {
                            stringMarker = '"';
                        }
                        else if (stringMarker == '"')
                        {
                            stringMarker = ' ';
                        }
                    }
                }
                else if (c == '\'')
                {
                    if (ind == 0 || text[ind - 1] != '\\')
                    {
                        if (stringMarker == ' ')
                        {
                            stringMarker = '\'';
                        }
                        else if (stringMarker == '\'')
                        {
                            stringMarker = ' ';
                        }
                    }
                }

                if (stringMarker == ' ')
                {
                    curLineVec.push_back(c);
                }
            }
        }

        ind++;
    }

    if (fatal && errorCount > 0)
    {
        throw runtime_error(to_string(errorCount) + " syntax errors.");
    }

    return;
}
