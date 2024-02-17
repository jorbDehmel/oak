/*
Front end for the Acorn Oak compiler.

File structure:
```
    lexer.hpp
    |
    oakc_structs.hpp
    |
    options.hpp
    |
    oakc_fns.hpp
    |
    <compiler frontend>   <---
```

Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "oakc_fns.hpp"
#include "options.hpp"

// Dummy wrapper function for updating
void update()
{
    fs::copy_file("/usr/include/oak/update.sh", "/tmp/update.sh");
    system("sudo bash /tmp/update.sh &");
}

// Dummy wrapper function for uninstalling
void uninstall()
{
    fs::copy_file("/usr/include/oak/uninstall.sh", "/tmp/uninstall.sh");
    system("sudo bash /tmp/uninstall.sh &");
}

void queryPackage(const std::string &name, AcornSettings &settings)
{
    std::string filepath = "/usr/include/oak/" + name + "/oak_package_info.txt";

    // loadPackageInfo
    try
    {
        PackageInfo info = loadPackageInfo(filepath, settings);

        // Print output
        std::cout << tags::green << info << '\n' << tags::reset;
    }
    catch (...)
    {
        std::cout << tags::red_bold << "Failed to query package " << name << ": It is most likely not installed.\n"
                  << tags::reset;
        throw package_error("Failed to query package");
    }

    return;
}

int main(const int argc, const char *argv[])
{
    auto start = std::chrono::high_resolution_clock::now(), end = start, compStart = start, compEnd = start;
    unsigned long long int oakElapsed = 0, reconstructionElapsed = 0;

    if (!fs::exists(".oak_build"))
    {
        fs::create_directory(".oak_build");
    }

    std::list<std::string> files;
    std::string out = "a.out";

    AcornSettings settings;

    try
    {
        // Argument parsing
        std::list<std::string> filesToAdd;
        for (int i = 1; i < argc; i++)
        {
            std::string cur = argv[i];
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
                        std::cout << helpText << '\n'
                                  << "All Oak data can be found at: " << OAK_DIR_PATH << '\n'
                                  << "Version: " << VERSION << '\n'
                                  << "License: " << LICENSE << '\n'
                                  << INFO << '\n';
                    }
                    else if (cur == "--query")
                    {
                        // Construct filepath
                        if (i + 1 >= argc)
                        {
                            throw std::runtime_error("--install must be followed by a package name");
                        }

                        queryPackage(argv[i + 1], settings);

                        i++;
                    }
                    else if (cur == "--version")
                    {
                        std::cout << "Version: " << VERSION << '\n' << "License: " << LICENSE << '\n' << INFO << '\n';
                    }
                    else if (cur == "--settings.debug")
                    {
                        settings.debug = !settings.debug;
                        std::cout << "Set settings.debug to " << (settings.debug ? "true" : "false") << '\n';
                    }
                    else if (cur == "--no_save")
                    {
                        settings.noSave = !settings.noSave;

                        if (settings.noSave)
                        {
                            settings.compile = settings.doLink = false;
                        }
                    }
                    else if (cur == "--translate")
                    {
                        settings.noSave = settings.compile = settings.doLink = false;
                    }
                    else if (cur == "--compile")
                    {
                        settings.compile = true;
                        settings.noSave = settings.doLink = false;
                    }
                    else if (cur == "--link")
                    {
                        settings.noSave = false;
                        settings.compile = settings.doLink = true;
                    }
                    else if (cur == "--clean")
                    {
                        settings.eraseTemp = !settings.eraseTemp;

                        if (settings.eraseTemp)
                        {
                            fs::remove_all(".oak_build");
                            fs::remove_all("*.log");
                        }
                    }
                    else if (cur == "--quit")
                    {
                        return 0;
                    }
                    else if (cur == "--test")
                    {
                        if (!settings.test)
                        {
                            settings.test = true;
                        }
                        else
                        {
                            if (!settings.testFail)
                            {
                                settings.testFail = true;
                            }
                            else
                            {
                                settings.testFail = settings.test = false;
                            }
                        }
                    }
                    else if (cur == "--execute")
                    {
                        settings.execute = !settings.execute;
                    }
                    else if (cur == "--output")
                    {
                        if (i + 1 >= argc)
                        {
                            throw std::runtime_error("--output must be followed by a filename");
                        }

                        out = argv[i + 1];
                        i++;
                    }
                    else if (cur == "--prettify")
                    {
                        settings.prettify = !settings.prettify;
                    }
                    else if (cur == "--install")
                    {
                        if (i + 1 >= argc)
                        {
                            throw std::runtime_error("--install must be followed by a package name");
                        }

                        downloadPackage(argv[1 + 1], settings);

                        i++;
                    }
                    else if (cur == "--reinstall")
                    {
                        if (i + 1 >= argc)
                        {
                            throw std::runtime_error("--reinstall must be followed by a package name");
                        }

                        downloadPackage(argv[1 + 1], settings, true);

                        i++;
                    }
                    else if (cur == "--exe_settings.debug")
                    {
                        if (settings.cflags.count("-g") == 0)
                        {
                            settings.cflags.insert("-g");
                        }
                    }
                    else if (cur == "--optimize")
                    {
                        if (settings.cflags.count("-O3") == 0)
                        {
                            settings.cflags.insert("-O3");
                        }
                    }
                    else if (cur == "--size")
                    {
                        getDiskUsage();
                    }
                    else if (cur == "--dump")
                    {
                        settings.alwaysDump = !settings.alwaysDump;
                    }
                    else if (cur == "--syntax")
                    {
                        settings.ignoreSyntaxErrors = !settings.ignoreSyntaxErrors;
                    }
                    else if (cur == "--new")
                    {
                        if (i + 1 >= argc)
                        {
                            throw std::runtime_error("--new must be followed by a package name");
                        }

                        makePackage(argv[i + 1]);
                        i++;
                    }
                    else if (cur == "--manual")
                    {
                        settings.manual = !settings.manual;
                    }
                    else
                    {
                        throw std::runtime_error(std::string("Invalid verbose option '") + cur + "'");
                    }
                }
                else
                {
                    // Compact options

                    // Iterate through single-letter options
                    std::string remainder = cur.substr(1);
                    for (char c : remainder)
                    {
                        switch (c)
                        {
                        case 'a':
                            std::cout << tags::green_bold << "Are you sure you want to update Acorn [y/N]? "
                                      << tags::reset << std::flush;

                            if (std::cin.peek() != 'y')
                            {
                                std::cout << tags::red_bold << "Update aborted.\n" << tags::reset;
                                return 10;
                            }

                            std::cout << tags::green_bold << "Updating...\n" << tags::reset << std::flush;

                            std::atexit(update);
                            exit(0);

                            break;
                        case 'A':
                            std::cout << tags::red_bold << "Are you sure you want to uninstall Acorn [y/N]? "
                                      << tags::reset << std::flush;

                            if (std::cin.peek() != 'y')
                            {
                                std::cout << tags::red_bold << "Uninstall aborted.\n" << tags::reset;
                                return 10;
                            }

                            std::atexit(uninstall);
                            exit(0);

                            break;
                        case 'c':
                            settings.compile = true;
                            settings.noSave = settings.doLink = false;
                            break;
                        case 'd':
                            settings.debug = !settings.debug;
                            std::cout << "Set settings.debug to " << (settings.debug ? "true" : "false") << '\n';
                            break;
                        case 'D':
                            if (i + 1 >= argc)
                            {
                                throw std::runtime_error("-D must be followed by a dialect file");
                            }

                            loadDialectFile(argv[i + 1], settings);
                            i++;

                            break;
                        case 'e':
                            settings.eraseTemp = !settings.eraseTemp;

                            if (settings.eraseTemp)
                            {
                                fs::remove_all(".oak_build");
                                fs::remove_all("*.log");
                            }

                            break;
                        case 'E':
                            settings.execute = !settings.execute;
                            break;
                        case 'g':
                            if (settings.cflags.count("-g") == 0)
                            {
                                settings.cflags.insert("-g");
                            }

                            break;
                        case 'h':
                            std::cout << helpText << '\n'
                                      << "All Oak data can be found at: " << OAK_DIR_PATH << '\n'
                                      << "Version: " << VERSION << '\n'
                                      << "License: " << LICENSE << '\n'
                                      << INFO << '\n';

                            break;
                        case 'l':
                            settings.noSave = false;
                            settings.compile = settings.doLink = true;
                            break;
                        case 'm':
                            settings.manual = !settings.manual;
                            break;
                        case 'M':
                            settings.isMacroCall = !settings.isMacroCall;
                            break;
                        case 'n':
                            settings.noSave = !settings.noSave;

                            if (settings.noSave)
                            {
                                settings.compile = settings.doLink = false;
                            }
                            break;
                        case 'o':
                            if (i + 1 >= argc)
                            {
                                throw std::runtime_error("-o must be followed by a filename");
                            }

                            out = argv[i + 1];
                            i++;
                            break;
                        case 'O':
                            if (settings.cflags.count("-O3") == 0)
                            {
                                settings.cflags.insert("-O3");
                            }

                            break;
                        case 'p':
                            settings.prettify = !settings.prettify;
                            break;
                        case 'q':
                            return 0;
                            break;
                        case 'Q':
                            if (i + 1 >= argc)
                            {
                                throw std::runtime_error("--install must be followed by a package name");
                            }

                            queryPackage(argv[i + 1], settings);
                            i++;

                            break;
                        case 'r':
                            if (i + 1 >= argc)
                            {
                                throw std::runtime_error("-r must be followed by a package name");
                            }

                            downloadPackage(argv[i + 1], settings, true);

                            i++;
                            break;
                        case 'R':
                            // Remove
                            if (i + 1 >= argc)
                            {
                                throw std::runtime_error("-R must be followed by a package name");
                            }

                            fs::remove_all(std::string("/usr/include/oak/") + argv[i + 1]);

                            i++;

                            break;
                        case 's':
                            getDiskUsage();
                            break;
                        case 'S':
                            if (i + 1 >= argc)
                            {
                                throw std::runtime_error("-S must be followed by a package name");
                            }

                            downloadPackage(argv[i + 1], settings);

                            i++;
                            break;
                        case 't':
                            settings.noSave = settings.compile = settings.doLink = false;
                            break;
                        case 'T':
                            if (!settings.test)
                            {
                                settings.test = true;
                            }
                            else
                            {
                                if (!settings.testFail)
                                {
                                    settings.testFail = true;
                                }
                                else
                                {
                                    settings.testFail = settings.test = false;
                                }
                            }
                            break;
                        case 'u':
                            settings.alwaysDump = !settings.alwaysDump;
                            break;
                        case 'U':
                            settings.doRuleLogFile = !settings.doRuleLogFile;
                            break;
                        case 'v':
                            std::cout << "Version: " << VERSION << '\n'
                                      << "License: " << LICENSE << '\n'
                                      << INFO << '\n';
                            break;
                        case 'w':
                            if (i + 1 >= argc)
                            {
                                throw std::runtime_error("-w must be followed by a package name");
                            }

                            makePackage(argv[i + 1]);

                            i++;
                            break;
                        case 'x':
                            settings.ignoreSyntaxErrors = !settings.ignoreSyntaxErrors;
                            break;

                        default:
                            throw std::runtime_error(std::string("Invalid option '") + c + "'");
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

        // Clean cache if not macro
        if (!settings.isMacroCall && getSize(COMPILED_PATH) > MAX_CACHE_KB)
        {
            std::cout << tags::yellow_bold << "Performing partial cache purge\n" << tags::reset;

            // Purge source .cpp, .hpp, and temp files
            fs::remove_all(COMPILED_PATH + "/*.c");
            fs::remove_all(COMPILED_PATH + "/*.h");
            fs::remove_all(COMPILED_PATH + "/*.txt");
            fs::remove_all(COMPILED_PATH + "/*.oak");

            if (getSize(COMPILED_PATH) > MAX_CACHE_KB)
            {
                std::cout << tags::yellow_bold << "Performing full cache purge\n" << tags::reset;

                // Purge all cache files
                fs::remove_all(COMPILED_PATH + "/*");
            }
        }

        if (!files.empty())
        {

            // Actual calls
            if (settings.debug)
            {
                std::cout << tags::green_bold << "\nPhase 1: File analysis\n" << tags::reset;
            }

            for (auto f : files)
            {
                doFile(f, settings);
            }

            // Reconstruct and save
            if (settings.debug)
            {
                std::cout << tags::green_bold << "\nLoaded all " << files.size() << " initial files.\n"
                          << settings.visitedFiles.size() - files.size() << " more files were included,\n"
                          << "For " << settings.visitedFiles.size() << " total files.\n"
                          << tags::reset;

                std::cout << tags::green_bold << "\nPhase 2: Reconstruction.\n" << tags::reset;
            }

            if (settings.table.count("main") == 0 && settings.doLink)
            {
                std::cout << tags::red_bold << "Error: No main function detected while in linking mode!\n"
                          << tags::reset;
                throw sequencing_error("A compilation unit must contain a main function.");
            }

            auto reconstructionStart = std::chrono::high_resolution_clock::now();

            std::pair<std::string, std::string> names = reconstructAndSave(out, settings);
            std::string toCompileFrom = names.second;

            end = std::chrono::high_resolution_clock::now();
            oakElapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            reconstructionElapsed =
                std::chrono::duration_cast<std::chrono::nanoseconds>(end - reconstructionStart).count();

            if (settings.debug)
            {
                std::cout << "Output header: '" << names.first << "'\n"
                          << "Output body:   '" << names.second << "'\n";
            }

            if (settings.noSave)
            {
                fs::remove_all(names.first);
                fs::remove_all(names.second);

                if (settings.debug)
                {
                    std::cout << "Erased output files.\n";
                }
            }
            else
            {
                compStart = std::chrono::high_resolution_clock::now();

                if (settings.compile)
                {
                    fs::create_directory(".oak_build");

                    if (settings.debug)
                    {
                        std::cout << tags::green_bold << "\nPhase 3: Compilation.\n"
                                  << "(via Clang)\n"
                                  << tags::reset;
                    }

                    std::string rootCommand = C_COMPILER + " -c ";

                    for (std::string flag : settings.cflags)
                    {
                        rootCommand += flag + " ";
                    }

                    std::string command = rootCommand + toCompileFrom + " -o " + toCompileFrom + ".o";

                    if (settings.debug)
                    {
                        std::cout << "System call `" << command << "`\n";
                    }

                    if (system(command.c_str()) != 0)
                    {
                        throw std::runtime_error("Failed to compile translated C files");
                    }

                    settings.objects.insert(toCompileFrom + ".o");

                    if (settings.doLink)
                    {
                        if (settings.debug)
                        {
                            std::cout << tags::green_bold << "\nPhase 4: Linking.\n"
                                      << "(via Clang)\n"
                                      << tags::reset;
                        }

                        std::string command = LINKER + " -o " + out + " ";
                        for (std::string object : settings.objects)
                        {
                            command += object + " ";
                        }

                        for (std::string flag : settings.cflags)
                        {
                            command += flag + " ";
                        }

                        if (settings.debug)
                        {
                            std::cout << "System call `" << command << "`\n";
                        }

                        if (system(command.c_str()) != 0)
                        {
                            throw std::runtime_error("Failed to link object files.");
                        }
                    }
                    else
                    {
                        if (settings.debug)
                        {
                            std::cout << tags::green_bold << "\nPhase 4 (compilation-only mode): Combining objects.\n"
                                      << "(via ld)\n"
                                      << tags::reset;
                        }

                        // Collate here
                        std::string command = "ld -r -o " + out + " ";
                        for (std::string object : settings.objects)
                        {
                            command += object + " ";
                        }

                        if (settings.debug)
                        {
                            std::cout << "System call `" << command << "`\n";
                        }

                        if (system(command.c_str()) != 0)
                        {
                            throw std::runtime_error("Failed to combine object files.");
                        }
                    }
                }

                compEnd = std::chrono::high_resolution_clock::now();
            }

            // Manual generation if requested
            if (settings.manual)
            {
                std::string manualPath = out;
                if (manualPath.find(".") != std::string::npos)
                {
                    manualPath = manualPath.substr(0, manualPath.find("."));
                }

                manualPath += ".md";

                if (settings.debug)
                {
                    std::cout << tags::green_bold << "Generating manual '" << manualPath << "'.\n" << tags::reset;
                }

                generate(files, manualPath);
            }
        }
    }
    catch (std::runtime_error &e)
    {
        if (oakElapsed != 0)
        {
            std::cout << "Elapsed Oak ns: " << oakElapsed;
        }

        std::cout << tags::red_bold << "\n" << settings.curFile << ":" << settings.curLine;

        if (settings.curLineSymbols.size() != 0)
        {
            std::cout << "\t(" << settings.curLineSymbols.front().file << ":" << settings.curLineSymbols.front().line
                      << ")\n";

            std::cout << tags::violet_bold << "In code line: `";

            for (auto s : settings.curLineSymbols)
            {
                std::cout << s.text << ' ';
            }
            std::cout << "`\n";
        }
        else
        {
            std::cout << '\n';
        }

        std::cout << tags::red_bold << "\nAn error occurred with message:\n" << e.what() << "\n" << tags::reset;

        return 2;
    }
    catch (...)
    {
        if (oakElapsed != 0)
        {
            std::cout << "Elapsed Oak ns: " << oakElapsed;
        }

        std::cout << tags::red_bold << "\n"
                  << settings.curFile << " " << settings.curLine << '\n'
                  << "\nAn unknown error ocurred.\n"
                  << "This is an issue with acorn, please report this bug!\n"
                  << tags::reset;

        return 3;
    }

    if (settings.debug)
    {
        auto size = getSize(".oak_build");

        if (size != 0)
        {
            std::cout << tags::green_bold << "\nThe build process took up " << humanReadable(size)
                      << " of local storage.\n"
                      << "(This is stored in './.oak_build/', which is now safe to delete.)\n"
                      << tags::reset;
        }
    }

    if (settings.debug)
    {
        end = std::chrono::high_resolution_clock::now();
        unsigned long long totalElapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        unsigned long long compElapsed =
            std::chrono::duration_cast<std::chrono::nanoseconds>(compEnd - compStart).count();

        float percentAcornTime = oakElapsed / (float)totalElapsed;
        percentAcornTime *= 100;

        if (settings.debug)
        {
            std::cout << tags::green_bold << "\nProcess finished with no errors.\n\n"
                      << tags::violet_bold << "Acorn-attributable time:\n"
                      << "Nanoseconds:  " << oakElapsed << '\n'
                      << "Microseconds: " << oakElapsed / 1'000.0 << '\n'
                      << "Milliseconds: " << oakElapsed / 1'000'000.0 << '\n'
                      << "Seconds:      " << oakElapsed / 1'000'000'000.0 << "\n\n"
                      << "Clang-attributable time:\n"
                      << "Nanoseconds:  " << compElapsed << '\n'
                      << "Microseconds: " << compElapsed / 1'000.0 << '\n'
                      << "Milliseconds: " << compElapsed / 1'000'000.0 << '\n'
                      << "Seconds:      " << compElapsed / 1'000'000'000.0 << "\n\n"
                      << "Total time:\n"
                      << "Nanoseconds:  " << totalElapsed << '\n'
                      << "Microseconds: " << totalElapsed / 1'000.0 << '\n'
                      << "Milliseconds: " << totalElapsed / 1'000'000.0 << '\n'
                      << "Seconds:      " << totalElapsed / 1'000'000'000.0 << "\n\n"
                      << "Acorn Milliseconds per file: " << (oakElapsed / 1'000'000.0) / settings.visitedFiles.size()
                      << "\n\n";
        }

        std::cout << tags::reset << "Note: This data is only accurate to one recursion.\n";

        if (percentAcornTime < 25)
        {
            std::cout << tags::green_bold;
        }
        else if (percentAcornTime < 50)
        {
            std::cout << tags::violet_bold;
        }
        else if (percentAcornTime < 75)
        {
            std::cout << tags::yellow_bold;
        }
        else
        {
            std::cout << tags::red_bold;
        }

        std::cout << "Percent of time which was Acorn-attributable: " << percentAcornTime << "%\n\n" << tags::reset;

        std::cout << "Post-compiler macro time usage by file (ns):\n";

        unsigned long long fileSum = 0;
        for (const auto &p : settings.visitedFiles)
        {
            fileSum += p.second;
        }

        for (const auto &p : settings.visitedFiles)
        {
            std::cout << std::setprecision(3) << std::setw(20) << 100.0 * (p.second / (double)fileSum) << "%\t"
                      << std::setw(15) << p.second << "\t" << p.first << '\n';
        }
        std::cout << '\n';

        std::vector<std::string> passNames = {
            "syntax check       ", "lexing             ", "macro defs         ",
            "compiler macros    ", "macro calls        ", "rules / dialect    ",
            "preproc defs       ", "op subs            ", "sequencing         ",
        };

        // Get total according to this:
        unsigned long long int total = 0;
        for (auto t : settings.phaseTimes)
        {
            total += t;
        }

        unsigned long long int totalPlusCompilation = total + reconstructionElapsed + compElapsed;

        std::cout << "Total logged by compiler: " << total << '\n' << "By compiler pass (ns):\n";
        std::cout.precision(3);

        for (int j = 0; j < passNames.size(); j++)
        {
            std::cout << std::left << "\t" << j + 1 << "\t" << std::setw(20) << passNames[j] << std::right
                      << std::setw(15) << settings.phaseTimes[j] << std::setw(10)
                      << (100 * (double)settings.phaseTimes[j] / total) << "%\t" << std::setw(10)
                      << (100 * (double)settings.phaseTimes[j] / totalPlusCompilation) << "%\n";
        }

        std::cout << std::left << "\t" << passNames.size() + 1 << "\t" << std::setw(20) << "reconstruction"
                  << std::right << std::setw(15) << reconstructionElapsed << std::setw(23)
                  << (100 * (double)reconstructionElapsed / totalPlusCompilation) << "%\n";

        std::cout << std::left << "\t" << passNames.size() + 2 << "\t"
                  << "C via " << std::setw(14) << C_COMPILER << std::right << std::setw(15) << compElapsed
                  << std::setw(23) << (100 * (double)compElapsed / totalPlusCompilation) << "%\n";

        std::cout << "Output file: " << out << '\n';
    }

    if (settings.execute && !settings.test)
    {
        if (out[0] != '/' && out[0] != '~' && out[0] != '.')
        {
            out = "./" + out;
        }

        std::cout << tags::reset << std::flush;

        int result = system(out.c_str());
        if (result != 0)
        {
            return result;
        }
    }

    // Run test suite ./tests/*
    if (settings.test)
    {
        if (!fs::exists("tests") || !fs::is_directory("tests"))
        {
            std::cout << tags::red_bold
                      << "Cannot run test suite when './tests' does not exist or exists and is not a directory.\n"
                      << tags::reset;
            return 15;
        }

        int good = 0, bad = 0, result;
        unsigned long long ms, totalMs = 0;
        std::chrono::_V2::high_resolution_clock::time_point start, end;
        std::ifstream file;
        std::string line;
        std::set<std::string> files;
        std::list<std::string> failed;

        // Get all files to run
        fs::create_directory(".oak_build");
        fs::remove_all("test_suite.tlog");

        // Build file vector
        for (auto test_iter : fs::directory_iterator("./tests"))
        {
            files.insert(test_iter.path().string());
        }

        std::cout << tags::violet_bold << "Running " << files.size() << " tests...\n"
                  << tags::reset << "[compiling, " << (settings.execute ? "" : "NOT ") << "executing,"
                  << (settings.testFail ? "" : " NOT") << " halting on failure]\n";

        // Iterate through files
        int i = 1;
        unsigned long long min = -1ull, max = 0ull;
        std::string nameOfMin, nameOfMax;
        for (auto test : files)
        {
            // Echo the current filename onto the test suite log
            system(("echo " + test + " >> test_suite.tlog").c_str());

            // Execute the command to test this file, either in compile or compile+execute mode.
            start = std::chrono::high_resolution_clock::now();
            if (settings.execute)
            {
                result = system(("acorn -o a.out --execute " + test + " >> test_suite.tlog 2>&1").c_str());
            }
            else
            {
                result = system(("acorn -o /dev/null " + test + " >> test_suite.tlog 2>&1").c_str());
            }
            end = std::chrono::high_resolution_clock::now();
            ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            // Update statistics
            totalMs += ms;
            if (ms < min)
            {
                nameOfMin = test;
                min = ms;
            }
            if (ms > max)
            {
                nameOfMax = test;
                max = ms;
            }

            // Update user on results of this test
            std::cout << "[" << i << "/" << files.size() << "]\t[" << (result == 0 ? tags::green : tags::red)
                      << std::left << std::setw(4) << result << tags::reset << "]" << std::right << std::setw(8) << ms
                      << " ms\t" << std::left << test << "\n";

            // Update other statistics
            if (result == 0)
            {
                good++;
            }
            else
            {
                failed.push_back(test);
                bad++;

                // If using the `-TT` flag, cease testing here.
                if (settings.testFail)
                {
                    break;
                }
            }

            i++;
        }

        // Print number of passed tests
        if (good != 0)
        {
            std::cout << tags::green_bold << "Passed:\t\t" << good << "\t(" << 100 * (double)good / (good + bad) << "%)"
                      << '\n';
        }

        // Print number of failed tests
        if (bad != 0)
        {
            std::cout << tags::red_bold << "Failed:\t\t" << bad << "\t(" << 100 * (double)bad / (good + bad) << "%)"
                      << '\n';
        }

        // Print final summary statistics
        std::cout << tags::reset << "Total:\t\t" << good + bad << '\n'
                  << "ms:\t\t" << totalMs << '\n'
                  << "min:\t\t" << min << "\t" << nameOfMin << " ms\n"
                  << "max:\t\t" << max << " \t" << nameOfMax << " ms\n"
                  << "mean test ms:\t" << (totalMs) / (double)(good + bad) << '\n';

        if (bad != 0)
        {
            std::cout << tags::red_bold << "\nFailed files:\n";
            for (auto item : failed)
            {
                std::cout << " - " << item << '\n';
            }
            std::cout << tags::reset;
        }

        std::cout << "\nAny output is in ./test_suite.tlog.\n";

        // Check for ansi2txt
        if (system("ansi2txt < /dev/null") == 0)
        {
            // Clean log file
            int result = system("ansi2txt < test_suite.tlog > tmp.tlog && mv tmp.tlog test_suite.tlog");

            if (result != 0)
            {
                std::cout << tags::yellow << "Warning: Failed to clean logs. They may be corrupted!\n" << tags::reset;
            }
        }
        else
        {
            std::cout << tags::yellow
                      << "Warning: `ansi2txt` is not installed, so output logs will look bad. Install `colorized-logs` "
                         "to silence this warning.\n"
                      << tags::reset;
        }

        // Exit w/ error if needed
        if (bad != 0)
        {
            if (settings.eraseTemp)
            {
                fs::remove_all(".oak_build");
                fs::remove_all("*.log");
            }
            return 19;
        }
    }

    if (settings.prettify)
    {
        if (system((PRETTIFIER + " .oak_build/*.c 2>&1 >/dev/null").c_str()) != 0)
        {
            throw sequencing_error("Failed to prettify output source files.");
        }

        if (system((PRETTIFIER + " .oak_build/*.h 2>&1 >/dev/null").c_str()) != 0)
        {
            throw sequencing_error("Failed to prettify output header files.");
        }
    }

    if (settings.eraseTemp)
    {
        fs::remove_all(".oak_build");
        fs::remove_all("*.log");
    }

    return 0;
}
