/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "oakc_fns.hpp"

static std::string debugTreePrefix = "";

const static std::set<std::string> oakKeywords = {"if",    "else",  "let",    "case", "default",
                                                  "match", "while", "return", "pre",  "post"};
const static std::set<std::string> cKeywords = {"alignas",
                                                "alignof",
                                                "auto",
                                                "break",
                                                "case",
                                                "const",
                                                "constexpr",
                                                "continue",
                                                "default",
                                                "do",
                                                "else",
                                                "extern",
                                                "for",
                                                "goto",
                                                "if",
                                                "inline",
                                                "nullptr",
                                                "register",
                                                "restrict",
                                                "return",
                                                "signed",
                                                "sizeof",
                                                "static",
                                                "static_assert",
                                                "switch",
                                                "thread_local",
                                                "typedef",
                                                "typeof",
                                                "typeof_unqual",
                                                "union",
                                                "unsigned",
                                                "volatile",
                                                "while",
                                                "_Alignas",
                                                "_Alignof",
                                                "_Atomic",
                                                "_BitInt",
                                                "_Bool",
                                                "_Complex",
                                                "_Decimal128",
                                                "_Decimal32",
                                                "_Decimal64",
                                                "_Generic",
                                                "_Imaginary",
                                                "_Noreturn",
                                                "_Static_assert",
                                                "_Thread_local",
                                                "_Pragma",
                                                "asm",
                                                "fortran",
                                                "int",
                                                "char",
                                                "float",
                                                "double",
                                                "long"};

// Prints the cumulative disk usage of Oak (in human-readable)
void getDiskUsage()
{
    const static std::vector<std::string> filesToCheck = {"/usr/bin/acorn", "/usr/include/oak"};
    unsigned long long int totalKB = 0;

    for (std::string s : filesToCheck)
    {
        unsigned long long int currentSize = getSize(s);
        totalKB += currentSize;

        std::cout << s << ": " << humanReadable(currentSize) << '\n';
    }

    std::cout << "\nTotal: " << humanReadable(totalKB) << '\n';
    return;
}

void doFile(const std::string &From, AcornSettings &settings)
{
    if (settings.debug)
    {
        while (settings.phaseTimes.size() < 9)
        {
            settings.phaseTimes.push_back(0);
        }
    }

    fs::create_directory(".oak_build");

    int curPhase = 0;

    static std::chrono::high_resolution_clock::time_point fileStartTimePoint;
    const static std::set<std::string> compilerMacros = {"include!", "package!", "link!", "flag!"};

    std::chrono::high_resolution_clock::time_point start, end;

    // Clear active rules
    settings.activeRules.clear();

    unsigned long long oldLineNum = settings.curLine;
    std::string oldFile = settings.curFile;

    settings.curLine = 1;
    settings.curFile = From;

    std::vector<Token> lexed, lexedCopy;

    try
    {
        if (From.size() < 4 || From.substr(From.size() - 4) != ".oak")
        {
            std::cout << tags::yellow_bold << "Warning: File '" << From << "' is not a .oak file.\n" << tags::reset;
        }

        std::string realName = From;

        while (realName.find("/") != std::string::npos)
        {
            realName = realName.substr(From.find("/") + 1);
        }

        for (char c : realName)
        {
            if ('A' <= c && c <= 'Z')
            {
                std::cout << tags::yellow_bold << "Warning: File '" << From << "' has illegal name.\n"
                          << "Oak files use underscore formatting (ie /path/to/file_to_use.oak).\n"
                          << tags::reset;
                break;
            }
        }

        if (settings.visitedFiles.count(From) != 0)
        {
            if (settings.debug)
            {
                std::cout << debugTreePrefix << "Skipping repeated file '" << From << "'\n";
            }
            settings.curFile = oldFile;
            settings.curLine = oldLineNum;
            return;
        }

        settings.visitedFiles[From] = 0;

        if (settings.debug)
        {
            std::cout << debugTreePrefix << "Loading file '" << From << "'\n";
            debugTreePrefix.append("|");
        }

        // A: Load file
        std::ifstream file(From, std::ios::in | std::ios::ate);
        if (!file.is_open())
        {
            settings.curFile = oldFile;
            settings.curLine = oldLineNum;
            throw std::runtime_error("Could not open source file '" + From + "'");
        }

        long long size = file.tellg();

        file.clear();
        file.seekg(0, std::ios::beg);

        size -= file.tellg();

        std::string text, line;

        text.reserve(size);
        line.reserve(64);

        while (getline(file, line))
        {
            text.append(line);
            text.push_back('\n');
        }

        file.close();

        // B: Syntax check

        if (settings.debug)
        {
            std::cout << debugTreePrefix << "Syntax check\n";
            start = std::chrono::high_resolution_clock::now();
        }

        if (!(settings.ignoreSyntaxErrors || settings.isMacroCall))
        {
            ensureSyntax(text, true, settings.curFile);
        }

        if (settings.debug)
        {
            end = std::chrono::high_resolution_clock::now();
            // Log at 0
            settings.phaseTimes[curPhase] += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            curPhase++;
        }

        // C: Lex
        if (settings.debug)
        {
            std::cout << debugTreePrefix << "Lexing\n";
            start = std::chrono::high_resolution_clock::now();
        }

        lexer dfa_lexer;
        lexed = dfa_lexer.lex(text, From);
        lexedCopy = lexed;

        if (settings.debug)
        {
            end = std::chrono::high_resolution_clock::now();
            // Log at 1
            settings.phaseTimes[curPhase] += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            curPhase++;
        }

        // D: Scan for macro definitions and handle them
        // This erases them from lexed
        if (settings.debug)
        {
            std::cout << debugTreePrefix << "Macro definitions\n";
            start = std::chrono::high_resolution_clock::now();
        }

        for (int i = 1; i + 1 < lexed.size(); i++)
        {
            if (lexed[i].back() == '!' && lexed[i] != "!" && lexed[i - 1] == "let")
            {
                if (lexed[i + 1] != "=")
                {
                    // Full macro

                    std::string name, contents;
                    name = lexed[i];

                    // Safety checks
                    if (settings.preprocDefines.count(name) != 0)
                    {
                        throw sequencing_error("Macro  '" + name + "' cannot overwrite definition of same name.");
                    }
                    else if (hasMacro(name, settings))
                    {
                        throw sequencing_error("Macro '" + name + "' cannot be overridden.");
                    }

                    contents = "let main";

                    i--;
                    lexed.erase(lexed.begin() + i); // Let
                    lexed.erase(lexed.begin() + i); // Name

                    while (lexed.size() >= i && lexed[i] != "{" && lexed[i] != ";")
                    {
                        contents += " " + lexed[i].text;
                        lexed.erase(lexed.begin() + i);
                    }

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

                        if (lexed[i].line == settings.curLine)
                        {
                            contents += " " + lexed[i].text;
                        }
                        else
                        {
                            contents += "\n" + lexed[i].text;
                            settings.curLine = lexed[i].line;
                        }

                        lexed.erase(lexed.begin() + i);
                    }

                    // macros[name] = contents;
                    // macroSourceFiles[name] = settings.curFile;
                    addMacro(name, contents, settings);
                }
                else
                {
                    // Preproc definition; Not a full macro

                    // Safety checks
                    if (settings.preprocDefines.count(lexed[i]) != 0)
                    {
                        throw sequencing_error("Preprocessor definition '" + lexed[i].text + "' cannot be overridden.");
                    }
                    else if (hasMacro(lexed[i], settings))
                    {
                        throw sequencing_error("Preprocessor definition '" + lexed[i].text +
                                               "' cannot overwrite macro of same name.");
                    }

                    std::string name = lexed[i];

                    // Erase as needed
                    i--;
                    lexed.erase(lexed.begin() + i); // Let
                    lexed.erase(lexed.begin() + i); // Name!
                    lexed.erase(lexed.begin() + i); // =

                    // Scrape until next semicolon
                    std::string contents = "";
                    while (lexed[i] != ";")
                    {
                        contents.append(lexed[i]);
                        lexed.erase(lexed.begin() + i);
                    }

                    lexed.erase(lexed.begin() + i); // ;

                    // Insert
                    settings.preprocDefines[name] = contents;
                }
            }
        }

        if (settings.debug)
        {
            end = std::chrono::high_resolution_clock::now();
            settings.phaseTimes[curPhase] += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            curPhase++;
        }

        bool compilerMacrosLeft;
        int compilerMacroPos = curPhase;
        do
        {
            // E: Scan for compiler macros; Do these first
            // This erases them from lexed
            if (settings.debug)
            {
                curPhase = compilerMacroPos;
                std::cout << debugTreePrefix << "Compiler macro\n";
                start = std::chrono::high_resolution_clock::now();
            }

            for (int i = 0; i < lexed.size(); i++)
            {
                if (lexed[i] != "!" && lexed[i].back() == '!')
                {
                    // File handling / translation unit macros
                    if (lexed[i] == "include!")
                    {
                        std::vector<std::string> args = getMacroArgs(lexed, i);

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

                        // global_end = std::chrono::high_resolution_clock::now();
                        // elapsedms += std::chrono::duration_cast<std::chrono::milliseconds>(global_end -
                        // global_start).count();

                        for (std::string a : args)
                        {
                            // If local, do that
                            if (fs::exists(a))
                            {
                                if (fs::exists(OAK_DIR_PATH + a) &&
                                    settings.visitedFiles.count(OAK_DIR_PATH + a) == 0 &&
                                    settings.visitedFiles.count(a) == 0)
                                {
                                    std::cout << tags::yellow_bold << "Warning: Including './" << a
                                              << "' over package file '" << OAK_DIR_PATH << a << "'.\n"
                                              << tags::reset;
                                }

                                doFile(a, settings);
                            }

                            // Else, look in OAK_DIR_PATH
                            else
                            {
                                doFile(OAK_DIR_PATH + a, settings);
                            }
                        }

                        // global_start = std::chrono::high_resolution_clock::now();

                        i--;
                    }
                    else if (lexed[i] == "link!")
                    {
                        std::vector<std::string> args = getMacroArgs(lexed, i);

                        // Clean arguments
                        for (int j = 0; j < args.size(); j++)
                        {
                            args[j] = cleanMacroArgument(args[j]);
                        }

                        for (std::string a : args)
                        {
                            if (settings.debug)
                            {
                                std::cout << debugTreePrefix << "Inserting object " << a << '\n';
                            }

                            if (fs::exists(a) || a[0] == '-')
                            {
                                if (fs::exists(OAK_DIR_PATH + a))
                                {
                                    std::cout << tags::yellow_bold << "Warning: Including local file '" << a
                                              << "' over package file of same name.\n"
                                              << tags::reset;
                                }

                                settings.objects.insert(a);
                            }
                            else
                            {
                                settings.objects.insert(OAK_DIR_PATH + a);
                            }
                        }
                        i--;
                    }
                    else if (lexed[i] == "flag!")
                    {
                        std::vector<std::string> args = getMacroArgs(lexed, i);

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

                        for (std::string a : args)
                        {
                            if (settings.debug)
                            {
                                std::cout << debugTreePrefix << "Appending flag " << a << '\n';
                            }

                            settings.cflags.insert(a);
                        }
                        i--;
                    }
                    else if (lexed[i] == "package!")
                    {
                        std::vector<std::string> args = getMacroArgs(lexed, i);
                        std::vector<std::string> files;

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
                        std::vector<std::string> backupDialectRules = settings.dialectRules;
                        settings.dialectRules.clear();

                        for (std::string a : args)
                        {
                            files = getPackageFiles(a);

                            for (std::string f : files)
                            {
                                if (settings.debug)
                                {
                                    std::cout << debugTreePrefix << "Loading package file '" << f << "'...\n";
                                }

                                // Backup dialect rules; These do NOT
                                std::vector<std::string> backupDialectRules = settings.dialectRules;
                                settings.dialectRules.clear();

                                // global_end = std::chrono::high_resolution_clock::now();
                                // elapsedms +=
                                //     std::chrono::duration_cast<std::chrono::milliseconds>(global_end -
                                //     global_start).count();

                                doFile(f, settings);

                                // global_start = std::chrono::high_resolution_clock::now();

                                settings.dialectRules = backupDialectRules;
                            }
                        }
                        i--;

                        settings.dialectRules = backupDialectRules;
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

            if (settings.debug)
            {
                fileStartTimePoint = std::chrono::high_resolution_clock::now();

                end = std::chrono::high_resolution_clock::now();
                settings.phaseTimes[curPhase] +=
                    std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                std::cout << debugTreePrefix << "Compiler macro finished in "
                          << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns\n";
                curPhase = compilerMacroPos + 1;
            }

            // E: Rules
            if (settings.debug)
            {
                std::cout << debugTreePrefix << "Rules\n";
                start = std::chrono::high_resolution_clock::now();
            }

            doRules(lexed, settings);

            if (settings.debug)
            {
                end = std::chrono::high_resolution_clock::now();
                settings.phaseTimes[curPhase] +=
                    std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
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
        if (settings.debug)
        {
            std::cout << debugTreePrefix << "Macro calls\n";
            start = std::chrono::high_resolution_clock::now();
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
                else if (lexed[i] == "c_warn!")
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

                std::string name = lexed[i];

                // Erases all trace of the call in the process
                std::vector<std::string> args = getMacroArgs(lexed, i);

                std::string output = callMacro(name, args, settings);
                std::vector<Token> lexedOutput = dfa_lexer.lex(output, name);

                // Reset preproc defs, as they tend to break w/ macros
                settings.preprocDefines["prev_file!"] = (oldFile == "" ? "\"NULL\"" : ("\"" + oldFile + "\""));
                settings.preprocDefines["file!"] = '"' + From + '"';

                // Insert the new code
                for (int j = lexedOutput.size() - 1; j >= 0; j--)
                {
                    lexed.insert(lexed.begin() + i, lexedOutput[j]);
                }

                // Since we do not change i, this new code will be scanned next.
            }
        }

        if (settings.debug)
        {
            end = std::chrono::high_resolution_clock::now();
            settings.phaseTimes[curPhase] += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            curPhase++;
        }

        // H: Preproc definitions

        settings.preprocDefines["prev_file!"] = (oldFile == "" ? "\"NULL\"" : ("\"" + oldFile + "\""));
        settings.preprocDefines["file!"] = '"' + From + '"';
        settings.preprocDefines["comp_time!"] = std::to_string(time(NULL));
        settings.preprocDefines["oak_version!"] = "\"" + std::string(VERSION) + "\"";

        // OS definition
        if (settings.preprocDefines.count("sys!") == 0)
        {
#if (defined(_WIN32) || defined(_WIN64))
            settings.preprocDefines["sys!"] = "\"WINDOWS\"";
#elif (defined(LINUX) || defined(__linux__))
            settings.preprocDefines["sys!"] = "\"LINUX\"";
#elif (defined(__APPLE__))
            settings.preprocDefines["sys!"] = "\"MAC\"";
#else
            settings.preprocDefines["sys!"] = "\"UNKNOWN\"";
#endif
        }

        if (settings.debug)
        {
            std::cout << debugTreePrefix << "Preproc definitions\n";
            start = std::chrono::high_resolution_clock::now();
        }

        for (int i = 0; i < lexed.size(); i++)
        {
            settings.preprocDefines["line!"] = std::to_string(lexed[i].line);
            if (settings.preprocDefines.count(lexed[i]) != 0)
            {
                std::vector<Token> lexedDef = dfa_lexer.lex(settings.preprocDefines[lexed[i]]);
                lexed.erase(lexed.begin() + i);

                for (int j = lexedDef.size() - 1; j >= 0; j--)
                {
                    lexed.insert(lexed.begin() + i, lexedDef[j]);
                }

                i--;
            }
            else if (lexed[i].size() > 1 && lexed[i].back() == '!' && (i + 1 >= lexed.size() || lexed[i + 1] != "("))
            {
                throw sequencing_error("Unknown preprocessor definition '" + lexed[i].text + "'");
            }
        }

        // Clean out any C keywords
        for (int i = 0; i < lexed.size(); i++)
        {
            if (cKeywords.count(lexed[i]) != 0 && oakKeywords.count(lexed[i]) == 0)
            {
                // Is a c keyword but not an oak keyword: replace it.
                // The `__KWA` suffix stands for `key word avoidance`.
                lexed[i].text += "__KWA";
            }
        }

        if (settings.debug)
        {
            end = std::chrono::high_resolution_clock::now();
            settings.phaseTimes[curPhase] += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            curPhase++;
        }

        // I: Operator substitution (within parenthesis and between commas)
        if (settings.debug)
        {
            std::cout << debugTreePrefix << "Operator substitution\n";
            start = std::chrono::high_resolution_clock::now();
        }

        operatorSub(lexed);

        if (settings.debug)
        {
            end = std::chrono::high_resolution_clock::now();
            settings.phaseTimes[curPhase] += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            curPhase++;
        }

        // J: Sequencing
        if (settings.debug)
        {
            std::cout << debugTreePrefix << "Sequencing (AST & translation)\n";
            start = std::chrono::high_resolution_clock::now();
        }

        ASTNode fileSeq = createSequence(lexed, settings);

        if (settings.debug)
        {
            end = std::chrono::high_resolution_clock::now();
            settings.phaseTimes[curPhase] += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            curPhase++;
        }

        if (fileSeq.type != nullType)
        {
            std::cout << tags::yellow_bold << "Warning: File '" << From << "' has hanging type '"
                      << toStr(&fileSeq.type) << "'\n"
                      << tags::reset;
        }

        if (settings.alwaysDump)
        {
            std::string name = purifyStr(From) + ".oak.log";

            if (settings.debug)
            {
                std::cout << debugTreePrefix << "Saving dump file '" << name << "'\n";
            }

            dump(lexed, name, From, settings.curLine, fileSeq, lexedCopy, "alwaysDump is true (not an error).",
                 settings);
        }

        if (settings.debug)
        {
            auto now = std::chrono::high_resolution_clock::now();
            settings.visitedFiles[From] +=
                std::chrono::duration_cast<std::chrono::nanoseconds>(now - fileStartTimePoint).count();

            debugTreePrefix.pop_back();
            std::cout << debugTreePrefix << "Finished file '" << From << "'\n";
        }
    }
    catch (rule_error &e)
    {
        std::cout << tags::red_bold << "Caught rule error '" << e.what() << "'\n" << tags::reset;

        std::string name = purifyStr(From) + ".oak.log";
        std::cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, settings.curLine, ASTNode(), lexedCopy, e.what(), settings);

        throw std::runtime_error("Failure in file '" + From + "': " + e.what());
    }
    catch (sequencing_error &e)
    {
        std::cout << tags::red_bold << "Caught sequencing error '" << e.what() << "'\n" << tags::reset;

        std::string name = purifyStr(From) + ".oak.log";
        std::cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, settings.curLine, ASTNode(), lexedCopy, e.what(), settings);

        throw std::runtime_error("Failure in file '" + From + "': " + e.what());
    }
    catch (parse_error &e)
    {
        std::cout << tags::red_bold << "Caught parse error '" << e.what() << "'\n" << tags::reset;

        std::string name = purifyStr(From) + ".oak.log";
        std::cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, settings.curLine, ASTNode(), lexedCopy, e.what(), settings);

        throw std::runtime_error("Failure in file '" + From + "': " + e.what());
    }
    catch (package_error &e)
    {
        std::cout << tags::red_bold << "Caught package error '" << e.what() << "'\n" << tags::reset;

        std::string name = purifyStr(From) + ".oak.log";
        std::cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, settings.curLine, ASTNode(), lexedCopy, e.what(), settings);

        throw std::runtime_error("Failure in file '" + From + "': " + e.what());
    }
    catch (generic_error &e)
    {
        std::cout << tags::red_bold << "Caught generic error '" << e.what() << "'\n" << tags::reset;

        std::string name = purifyStr(From) + ".oak.log";
        std::cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, settings.curLine, ASTNode(), lexedCopy, e.what(), settings);

        throw std::runtime_error("Failure in file '" + From + "': " + e.what());
    }
    catch (std::runtime_error &e)
    {
        std::cout << tags::red_bold << "Caught runtime error '" << e.what() << "'\n" << tags::reset;

        std::string name = purifyStr(From) + ".oak.log";
        std::cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, settings.curLine, ASTNode(), lexedCopy, e.what(), settings);

        throw std::runtime_error("Failure in file '" + From + "': " + e.what());
    }
    catch (std::exception &e)
    {
        std::cout << tags::red_bold << "Caught exception '" << e.what() << "'\n" << tags::reset;

        std::string name = purifyStr(From) + ".oak.log";
        std::cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, settings.curLine, ASTNode(), lexedCopy, e.what(), settings);

        throw std::runtime_error("Failure in file '" + From + "': " + e.what());
    }
    catch (...)
    {
        std::string name = purifyStr(From) + ".oak.log";
        std::cout << "Dump saved in " << name << "\n";
        dump(lexed, name, From, settings.curLine, ASTNode(), lexedCopy, "Unknown failure.", settings);

        throw std::runtime_error("Unknown failure in file '" + From + "'");
    }

    settings.curLine = oldLineNum;
    settings.curFile = oldFile;

    return;
}

void makePackage(const std::string &RawName)
{
    std::string name = purifyStr(RawName);
    for (int i = 0; i < name.size(); i++)
    {
        name[i] = tolower(name[i]);
    }

    // mkdir NAME
    fs::create_directory(name);

    // touch NAME/oak_package_info.txt
    std::ofstream info(name + "/oak_package_info.txt");
    if (!info.is_open())
    {
        throw package_error("Could not open oak_package_info.txt in newly created package folder.");
    }

    auto now = time(NULL);
    std::string time = ctime(&now);
    time.pop_back();

    info << "NAME = '" << name << "'\n"
         << "VERSION = ''\n"
         << "LICENSE = ''\n"
         << "DATE = '" << time << "'\n"
         << "AUTHOR = ''\n"
         << "EMAIL = ''\n"
         << "SOURCE = ''\n"
         << "PATH = '.'\n"
         << "ABOUT = ''\n"
         << "INCLUDE = '" << name << ".oak'\n"
         << "SYS_DEPS = ''\n"
         << "OAK_DEPS = ''\n";

    info.close();

    // touch NAME/NAME.oak
    std::ofstream include(name + "/" + name + ".oak");
    if (!include.is_open())
    {
        throw package_error("Could not open main include file in newly created package folder.");
    }
    include.close();

    return;
}

void printSyntaxError(const std::string &what, const std::vector<char> &curLineVec, const int &curLine,
                      const std::string &curFile)
{
    std::cout << tags::yellow_bold << '\n' << "In line '";

    for (const auto &c : curLineVec)
    {
        std::cout << c;
    }

    std::cout << "'\n" << tags::reset;

    std::cout << '\n'
              << tags::red_bold << "Syntax error at " << curFile << ':' << curLine << '\n'
              << what << '\n'
              << "(Use -x to make syntax errors nonfatal)" << tags::reset << "\n\n";

    return;
}

void ensureSyntax(const std::string &text, const bool &fatal, const std::string &curFile)
{
    std::vector<char> curLineVec;
    curLineVec.reserve(96);
    unsigned long long int curLine = 1;

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
                    printSyntaxError("Comments must begin with either '// ' or '///'", curLineVec, curLine, curFile);
                    errorCount++;
                }
            }

            // Multi-line comment opening
            else if (curLineVec.size() > 1 && curLineVec[0] == '/' && curLineVec[1] == '*')
            {
                if (curLineVec.size() > 2)
                {
                    printSyntaxError("Symbol '/*' must occupy its own line", curLineVec, curLine, curFile);
                    errorCount++;
                }

                commentDepth++;
            }

            // Multi-line comment closing
            else if (curLineVec.size() > 1 && curLineVec[0] == '*' && curLineVec[1] == '/')
            {
                if (curLineVec.size() > 2)
                {
                    printSyntaxError("Symbol '*/' must occupy its own line", curLineVec, curLine, curFile);
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
                            printSyntaxError(
                                "Precedent has been std::set for single-quotes, but double-quotes were used.",
                                curLineVec, curLine, curFile);
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
                            printSyntaxError(
                                "Precedent has been std::set for double-quotes, but single-quotes were used.",
                                curLineVec, curLine, curFile);
                            errorCount++;
                        }
                    }
                    else if (stringMarker == ' ')
                    {
                        // Do actual checks here
                        if (c == '{')
                        {
                            bracketDepth++;
                        }
                        else if (c == '}')
                        {
                            bracketDepth--;

                            if (bracketDepth < 0)
                            {
                                printSyntaxError("Unmatched close curly bracket", curLineVec, curLine, curFile);
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
                                printSyntaxError("Unmatched end parenthesis", curLineVec, curLine, curFile);
                                errorCount++;
                            }
                        }

                        else if (c == '.' && j > 0 && curLineVec[j - 1] == ')')
                        {
                            printSyntaxError("Illegal inline access to return value. Use a temporary variable instead.",
                                             curLineVec, curLine, curFile);
                            errorCount++;
                        }
                    }
                    else
                    {
                        // Inside a string literal
                        if (c == '\n')
                        {
                            printSyntaxError("String literal may not cross multiple lines. Use two successive string "
                                             "literals instead.",
                                             curLineVec, curLine, curFile);
                            errorCount++;
                        }
                    }
                }

                if (stringMarker != ' ')
                {
                    printSyntaxError("Unclosed string", curLineVec, curLine, curFile);
                    errorCount++;
                }
            }

            // Reset line
            curLineVec.clear();
            curLine++;
        }
        else
        {
            // Append to curLineVec
            if (!(curLineVec.empty() && (text[i] == ' ' || text[i] == '\t')))
            {
                curLineVec.push_back(text[i]);
            }

            if (curLineVec.size() == 97 && !(curLineVec.front() == '\'' || curLineVec.front() == '"'))
            {
                printSyntaxError("Lines should not exceed 96 characters", curLineVec, curLine, curFile);
                errorCount++;
            }
        }
    }

    if (text.size() == 0 || text.back() != '\n')
    {
        printSyntaxError("File must end with newline", curLineVec, curLine, curFile);
        errorCount++;
    }

    if (bracketDepth > 0)
    {
        printSyntaxError("Unmatched open curly bracket", curLineVec, curLine, curFile);
        errorCount++;
    }

    if (parenthesisDepth > 0)
    {
        printSyntaxError("Unmatched open parenthesis", curLineVec, curLine, curFile);
        errorCount++;
    }

    if (fatal && errorCount > 0)
    {
        throw std::runtime_error(std::to_string(errorCount) + " syntax errors.");
    }

    return;
}
