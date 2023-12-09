/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "acorn_resources.hpp"
#include "rules.hpp"
#include <bits/chrono.h>
#include <chrono>
#include <ratio>

// Settings
bool debug = false;
bool compile = true;
bool doLink = true;
bool alwaysDump = false;
bool manual = false;
bool ignoreSyntaxErrors = false;
bool timeAnalysis = false;
bool isMacroCall = false;

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
    const vector<string> filesToCheck = {"/usr/bin/acorn", "/usr/include/oak"};
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
    // chrono::high_resolution_clock::time_point global_start, global_end;
    // unsigned long long elapsedms = 0;
    // global_start = chrono::high_resolution_clock::now();

    if (debug)
    {
        while (phaseTimes.size() < 9)
        {
            phaseTimes.push_back(0);
        }
    }

    system("mkdir -p .oak_build");

    int curPhase = 0;

    const static set<string> compilerMacros = {"include!", "package!", "link!", "flag!"};

    chrono::_V2::system_clock::time_point start, end;

    // Clear active rules
    activeRules.clear();

    unsigned long long oldLineNum = curLine;
    string oldFile = curFile;

    curLine = 1;
    curFile = From;

    vector<string> lexed, lexedCopy;

    preprocDefines["prev_file!"] = (oldFile == "" ? "\"NULL\"" : ("\"" + oldFile + "\""));
    preprocDefines["file!"] = '"' + From + '"';
    preprocDefines["comp_time!"] = to_string(time(NULL));

    // System defines
    /*
    Options:
    -WINDOWS
    -LINUX
    -MAC
    -UNKNOWN
    */
    if (preprocDefines.count("sys!") == 0)
    {
#if (defined(_WIN32) || defined(_WIN64))
        preprocDefines["sys!"] = "WINDOWS";
#elif (defined(LINUX) || defined(__linux__))
        preprocDefines["sys!"] = "LINUX";
#elif (defined(__APPLE__))
        preprocDefines["sys!"] = "MAC";
#else
        preprocDefines["sys!"] = "UNKNOWN";
#endif
    }

    try
    {
        if (From.size() < 4 || From.substr(From.size() - 4) != ".oak")
        {
            cout << tags::yellow_bold << "Warning! File '" << From << "' is not a .oak file.\n" << tags::reset;
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
                cout << tags::yellow_bold << "Warning! File '" << From << "' has illegal name.\n"
                     << "Oak files use underscore formatting (ie /path/to/file_to_use.oak).\n"
                     << tags::reset;
                break;
            }
        }

        if (visitedFiles.count(realName) != 0)
        {
            if (debug)
            {
                cout << debugTreePrefix << "Skipping repeated file '" << From << "'\n";
            }
            return;
        }
        visitedFiles.insert(realName);

        if (debug)
        {
            cout << debugTreePrefix << "Loading file '" << From << "'\n";
            debugTreePrefix.append("|");
        }

        // A: Load file
        ifstream file(From, ios::in | ios::ate);
        if (!file.is_open())
        {
            throw runtime_error("Could not open source file '" + From + "'");
        }

        long long size = file.tellg();

        file.clear();
        file.seekg(0, ios::beg);

        size -= file.tellg();

        string text, line;

        text.reserve(size);
        line.reserve(64);

        while (getline(file, line))
        {
            text.append(line);
            text.push_back('\n');
        }

        file.close();

        // B: Syntax check

        if (debug)
        {
            cout << debugTreePrefix << "Syntax check\n";
            start = chrono::high_resolution_clock::now();
        }

        if (!ignoreSyntaxErrors || isMacroCall)
        {
            ensureSyntax(text, true);
        }

        if (debug)
        {
            end = chrono::high_resolution_clock::now();
            // Log at 0
            phaseTimes[curPhase] += chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            curPhase++;
        }

        // C: Lex
        if (debug)
        {
            cout << debugTreePrefix << "Lexing\n";
            start = chrono::high_resolution_clock::now();
        }

        lexed = lex(text);
        lexedCopy = lexed;

        if (debug)
        {
            end = chrono::high_resolution_clock::now();
            // Log at 1
            phaseTimes[curPhase] += chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            curPhase++;
        }

        // D: Scan for macro definitions and handle them
        // This erases them from lexed
        if (debug)
        {
            cout << debugTreePrefix << "Macro definitions\n";
            start = chrono::high_resolution_clock::now();
        }

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
                    macroSourceFiles[name] = curFile;
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
                        throw sequencing_error("Preprocessor definition '" + lexed[i] +
                                               "' cannot overwrite macro of same name.");
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

        if (debug)
        {
            end = chrono::high_resolution_clock::now();
            phaseTimes[curPhase] += chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            curPhase++;
        }

        bool compilerMacrosLeft;
        int compilerMacroPos = curPhase;
        do
        {
            // E: Scan for compiler macros; Do these first
            // This erases them from lexed
            if (debug)
            {
                curPhase = compilerMacroPos;
                cout << debugTreePrefix << "Compiler macros\n";
                start = chrono::high_resolution_clock::now();
            }

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

                        // global_end = chrono::high_resolution_clock::now();
                        // elapsedms += chrono::duration_cast<chrono::milliseconds>(global_end - global_start).count();

                        for (string a : args)
                        {
                            doFile(OAK_DIR_PATH + a);
                        }

                        // global_start = chrono::high_resolution_clock::now();

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
                    else if (lexed[i] == "flag!")
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
                                cout << debugTreePrefix << "Appending flag " << a << '\n';
                            }

                            cflags.insert(a);
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

                        // Backup dialect rules; These do NOT
                        vector<string> backupDialectRules = dialectRules;
                        dialectRules.clear();

                        for (string a : args)
                        {
                            files = getPackageFiles(a);

                            for (string f : files)
                            {
                                if (debug)
                                {
                                    cout << debugTreePrefix << "Loading package file '" << f << "'...\n";
                                }

                                // Backup dialect rules; These do NOT
                                vector<string> backupDialectRules = dialectRules;
                                dialectRules.clear();

                                // global_end = chrono::high_resolution_clock::now();
                                // elapsedms +=
                                //     chrono::duration_cast<chrono::milliseconds>(global_end - global_start).count();

                                doFile(f);

                                // global_start = chrono::high_resolution_clock::now();

                                dialectRules = backupDialectRules;
                            }
                        }
                        i--;

                        dialectRules = backupDialectRules;
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

            if (debug)
            {
                end = chrono::high_resolution_clock::now();
                phaseTimes[curPhase] += chrono::duration_cast<chrono::nanoseconds>(end - start).count();
                curPhase = compilerMacroPos + 1;
            }

            // E: Rules
            if (debug)
            {
                cout << debugTreePrefix << "Rules\n";
                start = chrono::high_resolution_clock::now();
            }

            doRules(lexed);

            if (debug)
            {
                end = chrono::high_resolution_clock::now();
                phaseTimes[curPhase] += chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            }

            // Update compilerMacrosLeft
            compilerMacrosLeft = false;
            for (const auto &item : lexed)
            {
                if (compilerMacros.count(item) != 0)
                {
                    compilerMacrosLeft = true;
                    break;
                }
            }
        } while (compilerMacrosLeft);
        curPhase = compilerMacroPos + 2;

        // G: Scan for macro calls and handle them
        if (debug)
        {
            cout << debugTreePrefix << "Macro calls\n";
            start = chrono::high_resolution_clock::now();
        }

        for (int i = 0; i < lexed.size(); i++)
        {
            // We can assume that no macro definitions remain
            if (lexed[i] != "!" && lexed[i].back() == '!' && i + 1 < lexed.size() && lexed[i + 1] == "(")
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
                else if (lexed[i] == "c_sys!")
                {
                    continue;
                }

                // More special cases: Rule macros
                else if (lexed[i] == "new_rule!" || lexed[i] == "use_rule!" || lexed[i] == "rem_rule!" ||
                         lexed[i] == "bundle_rule!")
                {
                    continue;
                }

                // Extra bonus special cases: Typing and sizing
                else if (lexed[i] == "type!" || lexed[i] == "size!")
                {
                    continue;
                }

                // Raw C insertion
                else if (lexed[i] == "raw_c!")
                {
                    continue;
                }

                // Super secret special cases: Pointer manipulation
                else if (lexed[i] == "ptrcpy!" || lexed[i] == "ptrarr!")
                {
                    continue;
                }

                string name = lexed[i];

                // Erases all trace of the call in the process
                vector<string> args = getMacroArgs(lexed, i);

                string output = callMacro(name, args, debug);
                vector<string> lexedOutput = lex(output);

                // Reset preproc defs, as they tend to break w/ macros
                preprocDefines["prev_file!"] = (oldFile == "" ? "\"NULL\"" : ("\"" + oldFile + "\""));
                preprocDefines["file!"] = '"' + From + '"';

                // Remove lines
                for (int ind = 0; ind < lexedOutput.size(); ind++)
                {
                    if (lexedOutput[ind].size() > 1 && lexedOutput[ind].substr(0, 2) == "//")
                    {
                        if (lexedOutput[ind].size() > 11 && lexedOutput[ind].substr(0, 11) == "//__LINE__=")
                        {
                            preprocDefines["line!"] = lexedOutput[ind].substr(11);
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

        if (debug)
        {
            end = chrono::high_resolution_clock::now();
            phaseTimes[curPhase] += chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            curPhase++;
        }

        // H: Preproc definitions
        if (debug)
        {
            cout << debugTreePrefix << "Preproc definitions\n";
            start = chrono::high_resolution_clock::now();
        }

        preprocDefines["line!"] = "1";
        for (int i = 0; i < lexed.size(); i++)
        {
            if (lexed[i].size() >= 2 && lexed[i].substr(0, 2) == "//")
            {
                // Line update special symbol
                string newLineNum = lexed[i].substr(11);
                preprocDefines["line!"] = newLineNum;
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

        if (debug)
        {
            end = chrono::high_resolution_clock::now();
            phaseTimes[curPhase] += chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            curPhase++;
        }

        // I: Operator substitution (within parenthesis and between commas)
        if (debug)
        {
            cout << debugTreePrefix << "Operator substitution\n";
            start = chrono::high_resolution_clock::now();
        }

        parenSub(lexed);

        if (debug)
        {
            end = chrono::high_resolution_clock::now();
            phaseTimes[curPhase] += chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            curPhase++;
        }

        // J: Sequencing
        if (debug)
        {
            cout << debugTreePrefix << "Sequencing (AST & translation)\n";
            start = chrono::high_resolution_clock::now();
        }

        sequence fileSeq = createSequence(lexed);

        if (debug)
        {
            end = chrono::high_resolution_clock::now();
            phaseTimes[curPhase] += chrono::duration_cast<chrono::nanoseconds>(end - start).count();
            curPhase++;
        }

        if (fileSeq.type != nullType)
        {
            cout << tags::yellow_bold << "Warning! File '" << From << "' has hanging type '" << toStr(&fileSeq.type)
                 << "'\n"
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
    catch (rule_error &e)
    {
        cout << tags::red_bold << "Caught rule error '" << e.what() << "'\n" << tags::reset;

        string name = "oak_dump_" + purifyStr(From) + ".log";
        cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, curLine, sequence(), lexedCopy);

        throw runtime_error("Failure in file '" + From + "': " + e.what());
    }
    catch (sequencing_error &e)
    {
        cout << tags::red_bold << "Caught sequencing error '" << e.what() << "'\n" << tags::reset;

        string name = "oak_dump_" + purifyStr(From) + ".log";
        cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, curLine, sequence(), lexedCopy);

        throw runtime_error("Failure in file '" + From + "': " + e.what());
    }
    catch (parse_error &e)
    {
        cout << tags::red_bold << "Caught parse error '" << e.what() << "'\n" << tags::reset;

        string name = "oak_dump_" + purifyStr(From) + ".log";
        cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, curLine, sequence(), lexedCopy);

        throw runtime_error("Failure in file '" + From + "': " + e.what());
    }
    catch (package_error &e)
    {
        cout << tags::red_bold << "Caught package error '" << e.what() << "'\n" << tags::reset;

        string name = "oak_dump_" + purifyStr(From) + ".log";
        cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, curLine, sequence(), lexedCopy);

        throw runtime_error("Failure in file '" + From + "': " + e.what());
    }
    catch (generic_error &e)
    {
        cout << tags::red_bold << "Caught generic error '" << e.what() << "'\n" << tags::reset;

        string name = "oak_dump_" + purifyStr(From) + ".log";
        cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, curLine, sequence(), lexedCopy);

        throw runtime_error("Failure in file '" + From + "': " + e.what());
    }
    catch (runtime_error &e)
    {
        cout << tags::red_bold << "Caught runtime error '" << e.what() << "'\n" << tags::reset;

        string name = "oak_dump_" + purifyStr(From) + ".log";
        cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, curLine, sequence(), lexedCopy);

        throw runtime_error("Failure in file '" + From + "': " + e.what());
    }
    catch (exception &e)
    {
        cout << tags::red_bold << "Caught exception '" << e.what() << "'\n" << tags::reset;

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

    // global_end = chrono::high_resolution_clock::now();
    // elapsedms += chrono::duration_cast<chrono::milliseconds>(global_end - global_start).count();
    // cout << DB_INFO << curFile << "\t" << elapsedms << " ms\n";

    curLine = oldLineNum;
    curFile = oldFile;

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
    cout << tags::yellow_bold << '\n' << "In line '";

    for (const auto &c : curLineVec)
    {
        cout << c;
    }

    cout << "'\n" << tags::reset;

    cout << '\n'
         << tags::red_bold << "Syntax error at " << curFile << ':' << curLine << '\n'
         << what << '\n'
         << "(Use -x to make syntax errors nonfatal)" << tags::reset << "\n\n";

    return;
}

void ensureSyntax(const string &text, const bool &fatal)
{
    vector<char> curLineVec;
    curLineVec.reserve(64);
    curLine = 1;

    int parenthesisDepth = 0;
    int bracketDepth = 0;
    int commentDepth = 0;
    int errorCount = 0;
    char stringMarker = ' ';

    char globalStringChoice = ' ';

    // Iterate over lines
    for (int i = 0; i < text.size(); i++)
    {
        if (text[i] == '\n')
        {
            // Single-line comment
            if (curLineVec.size() > 2 && curLineVec[0] == '/' && curLineVec[0] == '/')
            {
                if (curLineVec[2] != ' ' && curLineVec[2] != '/')
                {
                    printSyntaxError("Comments must begin with either '// ' or '///'", curLineVec);
                    errorCount++;
                }
            }

            // Multi-line comment opening
            else if (curLineVec.size() > 1 && curLineVec[0] == '/' && curLineVec[1] == '*')
            {
                if (curLineVec.size() > 2)
                {
                    printSyntaxError("Symbol '/*' must occupy its own line", curLineVec);
                    errorCount++;
                }

                commentDepth++;
            }

            // Multi-line comment closing
            else if (curLineVec.size() > 1 && curLineVec[0] == '*' && curLineVec[1] == '/')
            {
                if (curLineVec.size() > 2)
                {
                    printSyntaxError("Symbol '*/' must occupy its own line", curLineVec);
                    errorCount++;
                }

                commentDepth--;
            }

            // Non-comment code line
            else if (commentDepth == 0)
            {
                // Iterate over line here more more detailed checks
                stringMarker = ' ';
                for (int j = 0; j < curLineVec.size(); j++)
                {
                    const char c = curLineVec[j];
                    // cout << DB_INFO << c << '\t' << j << '\t' << curLineVec[j] << '\n';

                    if (c == '\\')
                    {
                        j++;
                        continue;
                    }

                    if (c == '"')
                    {
                        if (stringMarker == '"')
                        {
                            stringMarker = ' ';
                        }
                        else if (stringMarker == ' ')
                        {
                            stringMarker = '"';
                        }

                        if (globalStringChoice == ' ')
                        {
                            globalStringChoice = '"';
                        }
                        else if (globalStringChoice == '\'')
                        {
                            printSyntaxError("Precedent has been set for single-quotes, but double-quotes were used.",
                                             curLineVec);
                            errorCount++;
                        }
                    }
                    else if (c == '\'')
                    {
                        if (stringMarker == '\'')
                        {
                            stringMarker = ' ';
                        }
                        else if (stringMarker == ' ')
                        {
                            stringMarker = '\'';
                        }

                        if (globalStringChoice == ' ')
                        {
                            globalStringChoice = '\'';
                        }
                        else if (globalStringChoice == '"' && stringMarker == ' ')
                        {
                            printSyntaxError("Precedent has been set for double-quotes, but single-quotes were used.",
                                             curLineVec);
                            errorCount++;
                        }
                    }
                    else if (c == ' ')
                    {
                        // Do actual checks here
                        if (c == '{')
                        {
                            bracketDepth++;

                            if (curLineVec.size() != 0)
                            {
                                printSyntaxError("Symbol '{' must occupy its own line", curLineVec);
                                errorCount++;
                            }
                        }
                        else if (c == '}')
                        {
                            bracketDepth--;

                            if (bracketDepth < 0)
                            {
                                printSyntaxError("Unmatched close curly bracket", curLineVec);
                                errorCount++;
                            }

                            if (curLineVec.size() != 0)
                            {
                                printSyntaxError("Symbol '}' must occupy its own line", curLineVec);
                                errorCount++;
                            }
                        }
                        else if (c == '(')
                        {
                            parenthesisDepth++;
                        }
                        else if (c == ')')
                        {
                            parenthesisDepth--;

                            if (parenthesisDepth < 0)
                            {
                                printSyntaxError("Unmatched end parenthesis", curLineVec);
                                errorCount++;
                            }
                        }
                    }
                }

                if (stringMarker != ' ')
                {
                    printSyntaxError("Unclosed string", curLineVec);
                    errorCount++;
                }
            }

            // Reset line
            curLineVec.clear();
        }
        else
        {
            // Append to curLineVec
            if (!(curLineVec.size() == 0 && (text[i] == ' ' || text[i] == '\t')))
            {
                curLineVec.push_back(text[i]);
            }

            if (curLineVec.size() == 65 && !(curLineVec.front() == '\'' || curLineVec.front() == '"'))
            {
                printSyntaxError("Lines should not exceed 64 characters", curLineVec);
                errorCount++;
            }
        }
    }

    if (text.size() == 0 || text.back() != '\n')
    {
        printSyntaxError("File must end with newline", curLineVec);
        errorCount++;
    }

    if (bracketDepth > 0)
    {
        printSyntaxError("Unmatched open curly bracket", curLineVec);
        errorCount++;
    }

    if (parenthesisDepth > 0)
    {
        printSyntaxError("Unmatched open parenthesis", curLineVec);
        errorCount++;
    }

    if (fatal && errorCount > 0)
    {
        throw runtime_error(to_string(errorCount) + " syntax errors.");
    }

    return;
}
