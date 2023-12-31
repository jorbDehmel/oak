/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "acorn_resources.hpp"
#include "macros.hpp"
#include "packages.hpp"
#include "sequence.hpp"
#include "sequence_resources.hpp"
#include "tags.hpp"
#include <bits/chrono.h>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <ratio>
#include <stdexcept>

// Dummy wrapper function for updating
void update()
{
    system("cp /usr/include/oak/update.sh /tmp/update.sh");
    system("sudo bash /tmp/update.sh &");
}

// Dummy wrapper function for uninstalling
void uninstall()
{
    system("cp /usr/include/oak/uninstall.sh /tmp/uninstall.sh");
    system("sudo bash /tmp/uninstall.sh &");
}

void queryPackage(const std::string &name)
{
    std::string filepath = "/usr/include/oak/" + name + "/oak_package_info.txt";

    // loadPackageInfo
    try
    {
        packageInfo info = loadPackageInfo(filepath);

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

    if (!std::filesystem::exists(".oak_build"))
    {
        system("mkdir -p .oak_build");
    }

    std::vector<std::string> files;
    std::string out = "a.out";
    bool noSave = false;
    bool eraseTemp = false;
    bool prettify = false;

    bool execute = false;
    bool test = false;
    bool testFail = false;

    try
    {
        // Argument parsing
        std::vector<std::string> filesToAdd;
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

                        queryPackage(argv[i + 1]);

                        i++;
                    }
                    else if (cur == "--version")
                    {
                        std::cout << "Version: " << VERSION << '\n' << "License: " << LICENSE << '\n' << INFO << '\n';
                    }
                    else if (cur == "--debug")
                    {
                        debug = !debug;
                        std::cout << "Set debug to " << (debug ? "true" : "false") << '\n';
                    }
                    else if (cur == "--no_save")
                    {
                        noSave = !noSave;

                        if (noSave)
                        {
                            compile = doLink = false;
                        }
                    }
                    else if (cur == "--translate")
                    {
                        noSave = compile = doLink = false;
                    }
                    else if (cur == "--compile")
                    {
                        compile = true;
                        noSave = doLink = false;
                    }
                    else if (cur == "--link")
                    {
                        noSave = false;
                        compile = doLink = true;
                    }
                    else if (cur == "--clean")
                    {
                        eraseTemp = !eraseTemp;

                        if (eraseTemp)
                        {
                            system("rm -rf .oak_build");
                        }
                    }
                    else if (cur == "--quit")
                    {
                        return 0;
                    }
                    else if (cur == "--test")
                    {
                        if (!test)
                        {
                            test = true;
                        }
                        else
                        {
                            if (!testFail)
                            {
                                testFail = true;
                            }
                            else
                            {
                                testFail = test = false;
                            }
                        }
                    }
                    else if (cur == "--execute")
                    {
                        execute = !execute;
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
                        prettify = !prettify;
                    }
                    else if (cur == "--install")
                    {
                        if (i + 1 >= argc)
                        {
                            throw std::runtime_error("--install must be followed by a package name");
                        }

                        downloadPackage(argv[1 + 1]);

                        i++;
                    }
                    else if (cur == "--reinstall")
                    {
                        if (i + 1 >= argc)
                        {
                            throw std::runtime_error("--reinstall must be followed by a package name");
                        }

                        downloadPackage(argv[1 + 1], true);

                        i++;
                    }
                    else if (cur == "--exe_debug")
                    {
                        if (cflags.count("-g") == 0)
                        {
                            cflags.insert("-g");
                        }
                    }
                    else if (cur == "--optimize")
                    {
                        if (cflags.count("-O3") == 0)
                        {
                            cflags.insert("-O3");
                        }
                    }
                    else if (cur == "--size")
                    {
                        getDiskUsage();
                    }
                    else if (cur == "--dump")
                    {
                        alwaysDump = !alwaysDump;
                    }
                    else if (cur == "--syntax")
                    {
                        ignoreSyntaxErrors = !ignoreSyntaxErrors;
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
                        manual = !manual;
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
                            compile = true;
                            noSave = doLink = false;
                            break;
                        case 'd':
                            debug = !debug;
                            std::cout << "Set debug to " << (debug ? "true" : "false") << '\n';
                            break;
                        case 'D':
                            if (i + 1 >= argc)
                            {
                                throw std::runtime_error("-D must be followed by a dialect file");
                            }

                            loadDialectFile(argv[i + 1]);
                            i++;

                            break;
                        case 'e':
                            eraseTemp = !eraseTemp;

                            if (eraseTemp)
                            {
                                system("rm -rf .oak_build");
                            }

                            break;
                        case 'E':
                            execute = !execute;
                            break;
                        case 'g':
                            if (cflags.count("-g") == 0)
                            {
                                cflags.insert("-g");
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
                            noSave = false;
                            compile = doLink = true;
                            break;
                        case 'm':
                            manual = !manual;
                            break;
                        case 'M':
                            isMacroCall = !isMacroCall;
                            break;
                        case 'n':
                            noSave = !noSave;

                            if (noSave)
                            {
                                compile = doLink = false;
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
                            if (cflags.count("-O3") == 0)
                            {
                                cflags.insert("-O3");
                            }

                            break;
                        case 'p':
                            prettify = !prettify;
                            break;
                        case 'q':
                            return 0;
                            break;
                        case 'Q':
                            if (i + 1 >= argc)
                            {
                                throw std::runtime_error("--install must be followed by a package name");
                            }

                            queryPackage(argv[i + 1]);
                            i++;

                            break;
                        case 'r':
                            if (i + 1 >= argc)
                            {
                                throw std::runtime_error("-r must be followed by a package name");
                            }

                            downloadPackage(argv[i + 1], true);

                            i++;
                            break;
                        case 'R':
                            // Remove
                            if (i + 1 >= argc)
                            {
                                throw std::runtime_error("-R must be followed by a package name");
                            }

                            if (system((std::string("sudo rm -rf /usr/include/oak/") + argv[i + 1]).c_str()) != 0)
                            {
                                std::cout << tags::red_bold << "Warning! Failed to remove package '" << argv[i + 1]
                                          << "'\n"
                                          << tags::reset;
                            }

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

                            downloadPackage(argv[i + 1]);

                            i++;
                            break;
                        case 't':
                            noSave = compile = doLink = false;
                            break;
                        case 'T':
                            if (!test)
                            {
                                test = true;
                            }
                            else
                            {
                                if (!testFail)
                                {
                                    testFail = true;
                                }
                                else
                                {
                                    testFail = test = false;
                                }
                            }
                            break;
                        case 'u':
                            alwaysDump = !alwaysDump;
                            break;
                        case 'U':
                            doRuleLogFile = !doRuleLogFile;
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
                            ignoreSyntaxErrors = !ignoreSyntaxErrors;
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
        if (!isMacroCall && getSize(COMPILED_PATH) > MAX_CACHE_KB)
        {
            std::cout << tags::yellow_bold << DB_INFO << "Performing partial cache purge\n" << tags::reset;

            // Purge source .cpp, .hpp, and temp files
            system(("rm -rf " + COMPILED_PATH + "/*.c " + COMPILED_PATH + "/*.h " + COMPILED_PATH + "/*.txt " +
                    COMPILED_PATH + "/*.oak")
                       .c_str());

            if (getSize(COMPILED_PATH) > MAX_CACHE_KB)
            {
                std::cout << tags::yellow_bold << DB_INFO << "Performing full cache purge\n" << tags::reset;

                // Purge all cache files
                system(("rm -rf " + COMPILED_PATH + "/*").c_str());
            }
        }

        if (!files.empty())
        {

            // Actual calls
            if (debug)
            {
                std::cout << tags::green_bold << "\nPhase 1: File analysis\n" << tags::reset;
            }

            for (auto f : files)
            {
                doFile(f);
            }

            // Reconstruct and save
            if (debug)
            {
                std::cout << tags::green_bold << "\nLoaded all " << files.size() << " initial files.\n"
                          << visitedFiles.size() - files.size() << " more files were included,\n"
                          << "For " << visitedFiles.size() << " total files.\n"
                          << tags::reset;

                std::cout << tags::green_bold << "\nPhase 2: Reconstruction.\n" << tags::reset;
            }

            if (table.count("main") == 0 && compile)
            {
                std::cout << tags::red_bold << "Error! No main function detected while in compilation mode!\n"
                          << tags::reset;
                throw sequencing_error("A compilation unit must contain a main function.");
            }

            auto reconstructionStart = std::chrono::high_resolution_clock::now();

            std::pair<std::string, std::string> names = reconstructAndSave(out);
            cppSources.insert(names.second);

            end = std::chrono::high_resolution_clock::now();
            oakElapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            reconstructionElapsed =
                std::chrono::duration_cast<std::chrono::nanoseconds>(end - reconstructionStart).count();

            if (debug)
            {
                std::cout << "Output header: '" << names.first << "'\n"
                          << "Output body:   '" << names.second << "'\n";
            }

            if (noSave)
            {
                int result = system((std::string("rm ") + names.first + " " + names.second).c_str());

                if (result != 0)
                {
                    std::cout << tags::yellow_bold << "Warning: Could not erase generated files.\n" << tags::reset;
                }
                else if (debug)
                {
                    std::cout << "Erased output files.\n";
                }
            }
            else
            {
                compStart = std::chrono::high_resolution_clock::now();

                if (compile)
                {
                    smartSystem("mkdir -p .oak_build");

                    if (debug)
                    {
                        std::cout << tags::green_bold << "\nPhase 3: Compilation.\n"
                                  << "(via Clang)\n"
                                  << tags::reset;
                    }

                    std::string rootCommand = C_COMPILER + " -c ";

                    for (std::string flag : cflags)
                    {
                        rootCommand += flag + " ";
                    }

                    for (std::string source : cppSources)
                    {
                        std::string command = rootCommand + source + " -o " + source + ".o";

                        if (debug)
                        {
                            std::cout << "System call `" << command << "`\n";
                        }

                        if (system(command.c_str()) != 0)
                        {
                            throw std::runtime_error("Failed to compile translated C files");
                        }

                        objects.insert(source + ".o");
                    }

                    if (doLink)
                    {
                        if (debug)
                        {
                            std::cout << tags::green_bold << "\nPhase 4: Linking.\n"
                                      << "(via Clang)\n"
                                      << tags::reset;
                        }

                        std::string command = LINKER + " -o " + out + " ";
                        for (std::string object : objects)
                        {
                            command += object + " ";
                        }

                        for (std::string flag : cflags)
                        {
                            command += flag + " ";
                        }

                        if (debug)
                        {
                            std::cout << "System call `" << command << "`\n";
                        }

                        if (system(command.c_str()) != 0)
                        {
                            throw std::runtime_error("Failed to link object files.");
                        }
                    }
                }

                /*
                if (cacheOut != "")
                {
                    saveCompilerCache(cacheOut);
                }
                */

                compEnd = std::chrono::high_resolution_clock::now();
            }

            if (eraseTemp)
            {
                if (system("rm -rf .oak_build") != 0)
                {
                    std::cout << "rm -rf .oak_build\n";

                    std::cout << tags::yellow_bold << "Warning! Failed to erase './.oak_build/'.\n"
                              << "If left unfixed, this could cause problems.\n"
                              << tags::reset;
                }
            }

            // Manual generation if requested
            if (manual)
            {
                std::string manualPath = out;
                if (manualPath.find(".") != std::string::npos)
                {
                    manualPath = manualPath.substr(0, manualPath.find("."));
                }

                manualPath += ".md";

                if (debug)
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

        std::cout << tags::red_bold << "\n" << curFile << ":" << curLine << "\n";

        if (curLineSymbols.size() != 0)
        {
            std::cout << tags::violet_bold << "In code line: `";

            for (auto s : curLineSymbols)
            {
                std::cout << s << ' ';
            }
            std::cout << "`\n";
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
                  << curFile << " " << curLine << '\n'
                  << "\nAn unknown error ocurred.\n"
                  << "This is an issue with acorn, please report this bug!\n"
                  << tags::reset;

        return 3;
    }

    if (debug)
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

    if (debug)
    {
        end = std::chrono::high_resolution_clock::now();
        unsigned long long totalElapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        unsigned long long compElapsed =
            std::chrono::duration_cast<std::chrono::nanoseconds>(compEnd - compStart).count();

        float percentAcornTime = oakElapsed / (float)totalElapsed;
        percentAcornTime *= 100;

        if (debug)
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
                      << "Acorn Milliseconds per file: " << (oakElapsed / 1'000'000.0) / visitedFiles.size() << "\n\n";
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

        std::vector<std::string> passNames = {
            "syntax check       ", "lexing             ", "macro defs         ",
            "compiler macros    ", "macro calls        ", "rules / dialect    ",
            "preproc defs       ", "op subs            ", "sequencing         ",
        };

        // Get total according to this:
        unsigned long long int total = 0;
        for (auto t : phaseTimes)
        {
            total += t;
        }

        unsigned long long int totalPlusCompilation = total + reconstructionElapsed + compElapsed;

        std::cout << "Total logged by compiler: " << total << '\n' << "By compiler pass (ns):\n";
        std::cout.precision(3);

        for (int j = 0; j < passNames.size(); j++)
        {
            std::cout << std::left << "\t" << j + 1 << "\t" << std::setw(20) << passNames[j] << std::right
                      << std::setw(15) << phaseTimes[j] << std::setw(10) << (100 * (double)phaseTimes[j] / total)
                      << "%\t" << std::setw(10) << (100 * (double)phaseTimes[j] / totalPlusCompilation) << "%\n";
        }

        std::cout << std::left << "\t" << passNames.size() + 1 << "\t" << std::setw(20) << "reconstruction"
                  << std::right << std::setw(15) << reconstructionElapsed << std::setw(23)
                  << (100 * (double)reconstructionElapsed / totalPlusCompilation) << "%\n";

        std::cout << std::left << "\t" << passNames.size() + 2 << "\t" << std::setw(20) << "C via " << C_COMPILER
                  << std::right << std::setw(15) << compElapsed << std::setw(23)
                  << (100 * (double)compElapsed / totalPlusCompilation) << "%\n";

        std::cout << "Output file: " << out << '\n';
    }

    if (execute)
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
    if (test)
    {
        int good = 0, bad = 0, result;
        unsigned long long ms, totalMs = 0;
        std::chrono::_V2::high_resolution_clock::time_point start, end;
        std::ifstream file;
        std::string line;
        std::vector<std::string> files;
        std::vector<std::string> failed;

        // Get all files to run
        result = system("mkdir -p .oak_build && ls ./tests/*.oak > .oak_build/test_suite_temp.txt");
        file.open(".oak_build/test_suite_temp.txt");
        if (result != 0 || !file.is_open())
        {
            std::cout << tags::red_bold << "Error: Failed to find test suite `./tests/`.\n" << tags::reset;

            return 7;
        }

        while (!file.eof())
        {
            getline(file, line);

            if (line != "")
            {
                files.push_back(line);
            }
        }

        file.close();

        system("rm test_suite.log");
        std::cout << tags::violet_bold << "Running " << files.size() << " tests...\n" << tags::reset;

        std::cout << "[compiling " << (execute ? "and" : "but not") << " executing]\n";

        // Iterate through files
        int i = 1;
        for (auto test : files)
        {
            start = std::chrono::high_resolution_clock::now();
            if (execute)
            {
                result = system(("acorn -o a.out --execute " + test + " >> test_suite.log 2>&1").c_str());
            }
            else
            {
                result = system(("acorn -o /dev/null " + test + " >> test_suite.log 2>&1").c_str());
            }
            end = std::chrono::high_resolution_clock::now();

            ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            totalMs += ms;

            std::cout << "[" << i << "/" << files.size() << "]\t[" << (result == 0 ? tags::green : tags::red) << result
                      << tags::reset << "]" << std::right << std::setw(8) << ms << " ms\t" << std::left << test << "\n";
            if (result == 0)
            {
                good++;
            }
            else
            {
                failed.push_back(test);
                bad++;

                if (testFail)
                {
                    break;
                }
            }

            i++;
        }

        if (good != 0)
        {
            std::cout << tags::green_bold << "Passed:\t\t" << good << "\t(" << 100 * (double)good / (good + bad) << "%)"
                      << '\n';
        }
        if (bad != 0)
        {
            std::cout << tags::red_bold << "Failed:\t\t" << bad << "\t(" << 100 * (double)bad / (good + bad) << "%)"
                      << '\n';
        }

        std::cout << tags::reset << "Total:\t\t" << good + bad << '\n'
                  << "ms:\t\t" << totalMs << '\n'
                  << "ms per test:\t" << (totalMs) / (double)(good + bad) << '\n';

        if (bad != 0)
        {
            std::cout << tags::red_bold << "\nFailed files:\n";
            for (auto item : failed)
            {
                std::cout << " - " << item << '\n';
            }
            std::cout << tags::reset;
        }

        std::cout << "\nAny compiler is in ./test_suite.log.\n";
    }

    if (prettify)
    {
        if (system((PRETTIFIER + " .oak_build/*.c").c_str()) != 0)
        {
            throw sequencing_error("Failed to prettify output source files.");
        }

        if (system((PRETTIFIER + " .oak_build/*.h").c_str()) != 0)
        {
            throw sequencing_error("Failed to prettify output header files.");
        }
    }

    return 0;
}
