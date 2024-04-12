/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "oakc_fns.hpp"
#include "oakc_structs.hpp"
#include "tags.hpp"
#include <filesystem>

// Prints the cumulative disk usage of Oak (in human-readable)
void getDiskUsage()
{
    const std::list<std::string> filesToCheck = {"/usr/bin/acorn", "/usr/include/oak"};
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

    std::chrono::high_resolution_clock::time_point fileStartTimePoint;
    std::chrono::high_resolution_clock::time_point start, end;

    // Clear active rules
    settings.activeRules.clear();

    unsigned long long oldLineNum = settings.curLine;
    fs::path oldFile = settings.curFile;

    settings.curLine = 1;
    settings.curFile = fs::canonical(From);

    std::list<Token> lexed, lexedCopy;

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
            if (('A' <= c && c <= 'Z') || c == ' ')
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
                std::cout << settings.debugTreePrefix << "Skipping repeated file '" << From << "'\n";
            }
            settings.curFile = oldFile;
            settings.curLine = oldLineNum;
            return;
        }

        settings.visitedFiles[fs::canonical(From)] = 0;

        if (settings.debug)
        {
            std::cout << settings.debugTreePrefix << "Loading file '" << From << "'\n";
            settings.debugTreePrefix.append("|");
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
            std::cout << settings.debugTreePrefix << "Syntax check\n";
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
            std::cout << settings.debugTreePrefix << "Lexing\n";
            start = std::chrono::high_resolution_clock::now();
        }

        Lexer dfa_lexer;
        lexed = dfa_lexer.lex_list(text, From);
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
            std::cout << settings.debugTreePrefix << "Macro definitions\n";
            start = std::chrono::high_resolution_clock::now();
        }

        auto it = lexed.begin();
        it++;

        for (; itIsInRange(lexed, it, 1); it++)
        {
            Token curTok = *it;

            if (curTok.back() == '!' && curTok != "!" && itCmp(lexed, it, -1, "let"))
            {
                if (!itCmp(lexed, it, 1, "="))
                {
                    // Full macro

                    std::string name, contents;
                    name = curTok;

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

                    it--;
                    it = lexed.erase(it); // Let
                    it = lexed.erase(it); // Name

                    while (it != lexed.end() && *it != "{" && *it != ";")
                    {
                        contents += " " + it->text;
                        it = lexed.erase(it);
                    }

                    int count = 1;
                    it = lexed.erase(it);
                    contents += "\n{";

                    while (count != 0)
                    {
                        if (*it == "{")
                        {
                            count++;
                        }
                        else if (*it == "}")
                        {
                            count--;
                        }

                        if (it->line == settings.curLine)
                        {
                            contents += " " + it->text;
                        }
                        else
                        {
                            contents += "\n" + it->text;
                            settings.curLine = it->line;
                        }

                        it = lexed.erase(it);
                    }

                    // macros[name] = contents;
                    // macroSourceFiles[name] = settings.curFile;
                    addMacro(name, contents, settings);
                }
            }
        }

        if (settings.debug)
        {
            end = std::chrono::high_resolution_clock::now();
            settings.phaseTimes[curPhase] += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            curPhase++;
        }

        // Preprocessor definition finding
        for (auto it = ++lexed.begin(); ++it != lexed.end();)
        {
            if (it->back() == '!' && *it != "!" && itCmp(lexed, it, -1, "let"))
            {
                if (itCmp(lexed, it, 1, "="))
                {
                    // Preproc definition; Not a full macro

                    // Safety checks
                    if (settings.preprocDefines.count(*it) != 0)
                    {
                        throw sequencing_error("Preprocessor definition '" + it->text + "' cannot be overridden.");
                    }
                    else if (hasMacro(*it, settings))
                    {
                        throw sequencing_error("Preprocessor definition '" + it->text +
                                               "' cannot overwrite macro of same name.");
                    }

                    std::string name = *it;

                    // Erase as needed
                    it--;
                    it = lexed.erase(it); // Let
                    it = lexed.erase(it); // Name!
                    it = lexed.erase(it); // =

                    // Scrape until next semicolon
                    std::string contents = "";
                    while (*it != ";")
                    {
                        contents.append(*it);
                        it = lexed.erase(it);
                    }

                    it = lexed.erase(it); // ;

                    // Insert
                    settings.preprocDefines[name] = contents;
                }
                else if (!itCmp(lexed, it, 1, "("))
                {
                    throw sequencing_error("Error: `let " + it->text +
                                           "` must be followed by `=` (preprocessor definition) or "
                                           "`(` (macro definition). Instead, `" +
                                           (++it)->text + "`.");
                }
            }
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
                std::cout << settings.debugTreePrefix << "Compiler macro\n";
                start = std::chrono::high_resolution_clock::now();
            }

            for (auto it = lexed.begin(); it != lexed.end(); it++)
            {
                if (*it != "!" && it->back() == '!' && itCmp(lexed, it, 1, "("))
                {
                    // File handling / translation unit macros
                    if (*it == "include!")
                    {
                        if (settings.debug)
                        {
                            std::cout << settings.debugTreePrefix << "`include!`\n";
                        }

                        std::list<std::string> args = getMacroArgs(lexed, it);

                        // Clean arguments
                        for (auto it = args.begin(); it != args.end(); it++)
                        {
                            while (!it->empty() && (it->front() == '"' || it->front() == '\''))
                            {
                                it->erase(0, 1);
                            }
                            while (!it->empty() && (it->back() == '"' || it->back() == '\''))
                            {
                                it->pop_back();
                            }
                        }

                        for (std::string a : args)
                        {
                            auto base = settings.curFile.parent_path();

                            // If local, do that
                            if (fs::exists(base / a))
                            {
                                if (fs::exists(OAK_DIR_PATH + a) &&
                                    settings.visitedFiles.count(OAK_DIR_PATH + a) == 0 &&
                                    settings.visitedFiles.count(a) == 0)
                                {
                                    std::cout << tags::yellow_bold << "Warning: Including '" << base / a
                                              << "' over package file '" << OAK_DIR_PATH << a << "'.\n"
                                              << tags::reset;
                                }

                                doFile(base / a, settings);
                            }

                            // Else, look in OAK_DIR_PATH
                            else
                            {
                                doFile(OAK_DIR_PATH + a, settings);
                            }
                        }

                        it--;
                    }
                    else if (*it == "tag!")
                    {
                        if (settings.debug)
                        {
                            std::cout << settings.debugTreePrefix << "`tag!`\n";
                        }

                        std::list<std::string> args = getMacroArgs(lexed, it);

                        if (args.size() == 1)
                        {
                            std::string name = cleanMacroArgument(args.front());

                            if (settings.file_tags[settings.curFile].count(name) == 0)
                            {
                                settings.file_tags[settings.curFile][name] = "";
                            }
                        }
                        else if (args.size() == 2)
                        {
                            std::string name = cleanMacroArgument(args.front());
                            args.pop_front();

                            settings.file_tags[settings.curFile][name] = cleanMacroArgument(args.front());
                        }
                        else
                        {
                            throw sequencing_error("Compiler macro `tag!` must take one or two arguments:"
                                                   "The tag, and optionally a value.");
                        }

                        it--;
                    }
                    else if (*it == "link!")
                    {
                        if (settings.debug)
                        {
                            std::cout << settings.debugTreePrefix << "`link!`\n";
                        }

                        std::list<std::string> args = getMacroArgs(lexed, it);

                        // Clean arguments
                        for (auto it = args.begin(); it != args.end(); it++)
                        {
                            *it = cleanMacroArgument(*it);
                        }

                        for (std::string a : args)
                        {
                            if (settings.debug)
                            {
                                std::cout << settings.debugTreePrefix << "Inserting object " << a << '\n';
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
                        it--;
                    }
                    else if (*it == "flag!")
                    {
                        if (settings.debug)
                        {
                            std::cout << settings.debugTreePrefix << "`flag!`\n";
                        }

                        std::list<std::string> args = getMacroArgs(lexed, it);

                        // Clean arguments
                        for (auto it = args.begin(); it != args.end(); it++)
                        {
                            while (!it->empty() && (it->front() == '"' || it->front() == '\''))
                            {
                                it->erase(0, 1);
                            }
                            while (!it->empty() && (it->back() == '"' || it->back() == '\''))
                            {
                                it->pop_back();
                            }
                        }

                        for (std::string a : args)
                        {
                            if (settings.debug)
                            {
                                std::cout << settings.debugTreePrefix << "Appending flag " << a << '\n';
                            }

                            settings.cflags.insert(a);
                        }
                        it--;
                    }
                    else if (*it == "package!")
                    {
                        if (settings.debug)
                        {
                            std::cout << settings.debugTreePrefix << "`package!`\n";
                        }

                        std::list<std::string> args = getMacroArgs(lexed, it);
                        std::list<std::string> files;

                        // Clean arguments
                        for (auto it = args.begin(); it != args.end(); it++)
                        {
                            while (!it->empty() && (it->front() == '"' || it->front() == '\''))
                            {
                                it->erase(0, 1);
                            }
                            while (!it->empty() && (it->back() == '"' || it->back() == '\''))
                            {
                                it->pop_back();
                            }
                        }

                        // Backup dialect rules; These do NOT
                        std::vector<std::string> backupDialectRules = settings.dialectRules;
                        settings.dialectRules.clear();

                        for (std::string a : args)
                        {
                            files = getPackageFiles(a, settings);

                            for (std::string f : files)
                            {
                                if (settings.debug)
                                {
                                    std::cout << settings.debugTreePrefix << "Loading package file '" << f << "'...\n";
                                }

                                // Backup dialect rules
                                std::vector<std::string> backupDialectRules = settings.dialectRules;
                                settings.dialectRules.clear();

                                doFile(f, settings);

                                settings.dialectRules = backupDialectRules;
                            }
                        }
                        it--;

                        settings.dialectRules = backupDialectRules;
                    }
                    else
                    {
                        // Non-compiler macro definition
                        // Nested stuff may be allowed within
                        if (it != lexed.begin() && itCmp(lexed, it, -1, "let"))
                        {
                            while (it != lexed.end() && *it != "{" && *it != ";")
                            {
                                it++;
                            }

                            if (*it == ";")
                            {
                                it++;
                            }
                            else
                            {
                                int count = 1;
                                it++;

                                while (count != 0)
                                {
                                    if (*it == "{")
                                    {
                                        count++;
                                    }
                                    else if (*it == "}")
                                    {
                                        count--;
                                    }

                                    it++;
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
                std::cout << settings.debugTreePrefix << "Compiler macro finished in "
                          << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << " ns\n";
                curPhase = compilerMacroPos + 1;
            }

            // E: Rules
            if (settings.debug)
            {
                std::cout << settings.debugTreePrefix << "Rules\n";
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
                if (COMPILER_MACROS.count(item) != 0)
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
            std::cout << settings.debugTreePrefix << "Macro calls\n";
            start = std::chrono::high_resolution_clock::now();
        }

        for (auto it = lexed.begin(); it != lexed.end(); it++)
        {
            // We can assume that no macro definitions remain
            if (*it != "!" && it->back() == '!' && itCmp(lexed, it, 1, "("))
            {
                const static std::set<std::string> specialCases = {
                    "alloc!", "free!",   "free_arr!", "erase!",    "c_print!",  "c_panic!",
                    "c_sys!", "c_warn!", "new_rule!", "use_rule!", "rem_rule!", "bundle_rule!",
                    "type!",  "size!",   "raw_c!",    "ptrcpy!",   "ptrarr!"};

                if (specialCases.count(*it) != 0)
                {
                    continue;
                }

                std::string name = *it;

                // Erases all trace of the call in the process
                std::list<std::string> args = getMacroArgs(lexed, it);

                std::string output = callMacro(name, args, settings);
                std::list<Token> lexedOutput = dfa_lexer.lex_list(output, name);

                // Reset preproc defs, as they tend to break w/ macros
                settings.preprocDefines["prev_file!"] = (oldFile == "" ? "\"NULL\"" : ("\"" + oldFile.string() + "\""));
                settings.preprocDefines["file!"] = '"' + From + '"';

                // Insert the new code
                it = lexed.insert(it, lexedOutput.begin(), lexedOutput.end());

                // Since we do not change i, this new code will be scanned next.
            }
        }

        if (settings.debug)
        {
            end = std::chrono::high_resolution_clock::now();
            settings.phaseTimes[curPhase] += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            curPhase++;
        }

        // H: Preproc definition insertion

        settings.preprocDefines["prev_file!"] = (oldFile == "" ? "\"NULL\"" : ("\"" + oldFile.string() + "\""));
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
            std::cout << settings.debugTreePrefix << "Preproc definitions\n";
            start = std::chrono::high_resolution_clock::now();
        }

        for (auto it = lexed.begin(); it != lexed.end(); it++)
        {
            settings.preprocDefines["line!"] = std::to_string(it->line);
            if (settings.preprocDefines.count(*it) != 0)
            {
                std::list<Token> lexedDef = dfa_lexer.lex_list(settings.preprocDefines[*it]);
                it = lexed.erase(it);

                it = lexed.insert(it, lexedDef.begin(), lexedDef.end());

                it--;
            }
            else if (it->size() > 1 && it->back() == '!' && !itCmp(lexed, it, 1, "("))
            {
                throw sequencing_error("Unknown preprocessor definition '" + it->text + "'");
            }
        }

        // Clean out any C keywords
        for (auto it = lexed.begin(); it != lexed.end(); it++)
        {
            if (C_KEYWORDS.count(*it) != 0 && OAK_KEYWORDS.count(*it) == 0)
            {
                // Is a c keyword but not an oak keyword: replace it.
                // The `__KWA` suffix stands for `key word avoidance`.
                it->text += "__KWA";
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
            std::cout << settings.debugTreePrefix << "Operator substitution\n";
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
            std::cout << settings.debugTreePrefix << "Sequencing (AST & translation)\n";
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
                std::cout << settings.debugTreePrefix << "Saving dump file '" << name << "'\n";
            }

            dump(lexed, name, From, settings.curLine, fileSeq, lexedCopy, "alwaysDump is true (not an error).",
                 settings);
        }

        if (settings.debug)
        {
            auto now = std::chrono::high_resolution_clock::now();
            settings.visitedFiles[From] +=
                std::chrono::duration_cast<std::chrono::nanoseconds>(now - fileStartTimePoint).count();

            settings.debugTreePrefix.pop_back();
            std::cout << settings.debugTreePrefix << "Finished file '" << From << "'\n";
        }
    }
    catch (rule_error &e)
    {
        if (settings.curFile == settings.entryPoint)
        {
            std::cout << tags::red_bold << "Caught rule error:\n" << e.what() << "\n" << tags::reset;
        }

        std::string name = purifyStr(From) + ".oak.log";
        dump(lexed, name, From, settings.curLine, ASTNode(), lexedCopy, e.what(), settings);

        if (settings.debug)
        {
            std::cout << "Dump saved in " << name << "\n";
        }

        throw std::runtime_error("Failure in file '" + From + "':\n" + e.what());
    }
    catch (sequencing_error &e)
    {
        if (settings.curFile == settings.entryPoint)
        {
            std::cout << tags::red_bold << "Caught sequencing error:\n" << e.what() << "\n" << tags::reset;
        }

        std::string name = purifyStr(From) + ".oak.log";
        dump(lexed, name, From, settings.curLine, ASTNode(), lexedCopy, e.what(), settings);

        if (settings.debug)
        {
            std::cout << "Dump saved in " << name << "\n";
        }

        throw std::runtime_error("Failure in file '" + From + "':\n" + e.what());
    }
    catch (parse_error &e)
    {
        if (settings.curFile == settings.entryPoint)
        {
            std::cout << tags::red_bold << "Caught parse error:\n" << e.what() << "\n" << tags::reset;
        }

        std::string name = purifyStr(From) + ".oak.log";
        dump(lexed, name, From, settings.curLine, ASTNode(), lexedCopy, e.what(), settings);

        if (settings.debug)
        {
            std::cout << "Dump saved in " << name << "\n";
        }

        throw std::runtime_error("Failure in file '" + From + "':\n" + e.what());
    }
    catch (package_error &e)
    {
        if (settings.curFile == settings.entryPoint)
        {
            std::cout << tags::red_bold << "Caught package error:\n" << e.what() << "\n" << tags::reset;
        }

        std::string name = purifyStr(From) + ".oak.log";
        dump(lexed, name, From, settings.curLine, ASTNode(), lexedCopy, e.what(), settings);

        if (settings.debug)
        {
            std::cout << "Dump saved in " << name << "\n";
        }

        throw std::runtime_error("Failure in file '" + From + "':\n" + e.what());
    }
    catch (generic_error &e)
    {
        if (settings.curFile == settings.entryPoint)
        {
            std::cout << tags::red_bold << "Caught generic error:\n" << e.what() << "\n" << tags::reset;
        }

        std::string name = purifyStr(From) + ".oak.log";
        dump(lexed, name, From, settings.curLine, ASTNode(), lexedCopy, e.what(), settings);

        if (settings.debug)
        {
            std::cout << "Dump saved in " << name << "\n";
        }

        throw std::runtime_error("Failure in file '" + From + "':\n" + e.what());
    }
    catch (std::runtime_error &e)
    {
        if (settings.curFile == settings.entryPoint)
        {
            std::cout << tags::red_bold << "Caught runtime error:\n" << e.what() << "\n" << tags::reset;
        }

        std::string name = purifyStr(From) + ".oak.log";
        dump(lexed, name, From, settings.curLine, ASTNode(), lexedCopy, e.what(), settings);

        if (settings.debug)
        {
            std::cout << "Dump saved in " << name << "\n";
        }

        throw std::runtime_error("Failure in file '" + From + "':\n" + e.what());
    }
    catch (std::exception &e)
    {
        if (settings.curFile == settings.entryPoint)
        {
            std::cout << tags::red_bold << "Caught exception:\n" << e.what() << "\n" << tags::reset;
        }

        std::string name = purifyStr(From) + ".oak.log";
        dump(lexed, name, From, settings.curLine, ASTNode(), lexedCopy, e.what(), settings);

        if (settings.debug)
        {
            std::cout << "Dump saved in " << name << "\n";
        }

        throw std::runtime_error("Failure in file '" + From + "':\n" + e.what());
    }
    catch (...)
    {
        std::string name = purifyStr(From) + ".oak.log";
        dump(lexed, name, From, settings.curLine, ASTNode(), lexedCopy, "Unknown failure.", settings);
        std::cout << "Dump saved in " << name << "\n";

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
    int emptyLines = 0;
    char stringMarker = ' ';

    char globalStringChoice = ' ';

    unsigned int commentLines = 0;

    // Iterate over lines
    for (int i = 0; i < text.size(); i++)
    {
        if (text[i] == '\n')
        {
            if (curLineVec.size() == 0)
            {
                emptyLines++;

                if (i > 0 && text[i - 1] == ' ')
                {
                    printSyntaxError("Trailing whitespace is not allowed", curLineVec, curLine, curFile);
                    errorCount++;
                }
            }

            // Single-line comment
            if (curLineVec.size() > 2 && curLineVec[0] == '/' && curLineVec[1] == '/')
            {
                if (curLineVec[2] != ' ' && curLineVec[2] != '/')
                {
                    printSyntaxError("Comments must begin with either '// ' or '///'", curLineVec, curLine, curFile);
                    errorCount++;
                }
                commentLines++;
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
                commentLines++;
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
                commentLines++;
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
                            printSyntaxError("Precedent has been set for single-quotes, but double-quotes were used.",
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
                            printSyntaxError("Precedent has been set for double-quotes, but single-quotes were used.",
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
                        else if (c == '\t')
                        {
                            printSyntaxError("Tab characters (\\t) are not allowed; Use spaces instead", curLineVec,
                                             curLine, curFile);
                            errorCount++;
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
            else
            {
                commentLines++;
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

            // Max suggested line width
            if (curLineVec.size() == 65)
            {
                std::cout << tags::yellow_bold << "Warning: Lines should not exceed 64 characters. (" << curFile << ":"
                          << curLine << ")\n"
                          << tags::reset;
            }

            // Max allowed line width
            if (curLineVec.size() == 97)
            {
                printSyntaxError("Lines must not exceed 96 characters", curLineVec, curLine, curFile);
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

    if (commentLines == 0)
    {
        printSyntaxError("No comments present in source code", curLineVec, curLine, curFile);
        errorCount++;
    }

    if ((curLine - emptyLines) > 0 && (double)commentLines / (double)(curLine - emptyLines) < 0.15)
    {
        double percentage = 100.0 * (double)commentLines / (double)(curLine - emptyLines);
        std::cout << tags::yellow_bold << "Warning: Less than 15% of source file " << curFile << " (" << percentage
                  << "%) is commenting.\n"
                  << tags::reset;
    }

    if (fatal && errorCount > 0)
    {
        throw std::runtime_error(std::to_string(errorCount) + " syntax errors.");
    }

    return;
}
