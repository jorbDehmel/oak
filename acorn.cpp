/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "acorn_resources.hpp"
using namespace std;

int main(const int argc, const char *argv[])
{
    // OS warning if someone manages to compile this for windows or macOS
#if (defined(_WIN32) || defined(_WIN64) || defined(__APPLE__))
    cout << tags::red_bold
         << "Severe Warning! This version of Oak is designed for Linux!\n"
         << "No other operating systems are supported!\n"
         << tags::reset;
#endif

    auto start = chrono::high_resolution_clock::now(), end = start, compStart = start, compEnd = start;
    unsigned long long int oakElapsed = 0;

    vector<string> files;
    string out = "a.out";
    bool noSave = false;
    bool eraseTemp = true;

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
                    else if (cur == "--dump")
                    {
                        alwaysDump = !alwaysDump;
                    }
                    else if (cur == "--new")
                    {
                        if (i + 1 >= argc)
                        {
                            throw runtime_error("--new must be followed by a package name");
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
                        case 'c':
                            compile = true;
                            noSave = link = false;
                            break;
                        case 'd':
                            debug = !debug;
                            cout << "Set debug to " << (debug ? "true" : "false") << '\n';
                            break;
                        case 'D':
                            if (i + 1 >= argc)
                            {
                                throw runtime_error("-D must be followed by a dialect file");
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
                        case 'h':
                            cout << helpText << '\n'
                                 << "Version: " << VERSION << '\n'
                                 << "License: " << LICENSE << '\n'
                                 << INFO << '\n';

                            break;
                        case 'l':
                            noSave = false;
                            compile = link = true;
                            break;
                        case 'm':
                            manual = !manual;
                            break;
                        case 'n':
                            noSave = !noSave;

                            if (noSave)
                            {
                                compile = link = false;
                            }
                            break;
                        case 'o':
                            if (i + 1 >= argc)
                            {
                                throw runtime_error("-o must be followed by a filename");
                            }

                            out = argv[i + 1];
                            i++;
                            break;
                        case 'p':
                            pretty = !pretty;
                            break;
                        case 'q':
                            return 0;
                            break;
                        case 'r':
                            if (i + 1 >= argc)
                            {
                                throw runtime_error("-r must be followed by a package name");
                            }

                            downloadPackage(argv[i + 1], true);

                            i++;
                            break;
                        case 'R':
                            // Remove
                            if (i + 1 >= argc)
                            {
                                throw runtime_error("-R must be followed by a package name");
                            }

                            if (system((string("sudo rm -rf /usr/include/oak/") + argv[i + 1]).c_str()) != 0)
                            {
                                cout << tags::red_bold
                                     << "Warning! Failed to remove package '" << argv[i + 1] << "'\n"
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
                                throw runtime_error("-S must be followed by a package name");
                            }

                            downloadPackage(argv[i + 1]);

                            i++;
                            break;
                        case 't':
                            noSave = compile = link = false;
                            break;
                        case 'u':
                            alwaysDump = !alwaysDump;
                            break;
                        case 'v':
                            cout << "Version: " << VERSION << '\n'
                                 << "License: " << LICENSE << '\n'
                                 << INFO << '\n';
                            break;
                        case 'w':
                            if (i + 1 >= argc)
                            {
                                throw runtime_error("-w must be followed by a package name");
                            }

                            makePackage(argv[i + 1]);

                            i++;
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
            compStart = chrono::high_resolution_clock::now();

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

                    throw_assert(system(command.c_str()) == 0);
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

                    throw_assert(system(command.c_str()) == 0);
                }
            }

            compEnd = chrono::high_resolution_clock::now();
        }

        // Prettification of C++ files
        if (pretty)
        {
            prettify(names.second);
        }

        if (eraseTemp)
        {
            if (system("rm -rf .oak_build") != 0)
            {
                cout << "rm -rf .oak_build\n";

                cout << tags::yellow_bold
                     << "Warning! Failed to erase './.oak_build/'.\n"
                     << "If left unfixed, this could cause problems.\n"
                     << tags::reset;
            }
        }

        // Manual generation if requested
        if (manual)
        {
            string manualPath = out;
            if (manualPath.find(".") != string::npos)
            {
                manualPath = manualPath.substr(0, manualPath.find("."));
            }

            manualPath += ".md";

            if (debug)
            {
                cout << tags::green_bold
                     << "Generating manual '" << manualPath << "'.\n"
                     << tags::reset;
            }

            generate(files, manualPath);
        }
    }
    catch (parse_error &e)
    {
        if (oakElapsed != 0)
        {
            cout << "Elapsed Oak ns: " << oakElapsed;
        }

        cout << tags::red_bold
             << "\n"
             << curFile << " " << curLine << '\n'
             << "A parsing error occurred with message:\n"
             << e.what()
             << "\n"
             << tags::reset;

        return 1;
    }
    catch (runtime_error &e)
    {
        if (oakElapsed != 0)
        {
            cout << "Elapsed Oak ns: " << oakElapsed;
        }

        cout << tags::red_bold
             << "\n"
             << curFile << " " << curLine << '\n'
             << "A runtime error occurred with message:\n"
             << e.what()
             << "\n"
             << tags::reset;

        return 2;
    }
    catch (...)
    {
        if (oakElapsed != 0)
        {
            cout << "Elapsed Oak ns: " << oakElapsed;
        }

        cout << tags::red_bold
             << "\n"
             << curFile << " " << curLine << '\n'
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
        unsigned long long compElapsed = chrono::duration_cast<chrono::nanoseconds>(compEnd - compStart).count();

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
             << "Clang++-attributable time:\n"
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

        if (percentAcornTime < 25)
        {
            cout << tags::green_bold;
        }
        else if (percentAcornTime < 50)
        {
            cout << tags::violet_bold;
        }
        else if (percentAcornTime < 75)
        {
            cout << tags::yellow_bold;
        }
        else
        {
            cout << tags::red_bold;
        }

        cout << "Percent of time which was Acorn-attributable: "
             << percentAcornTime
             << "%\n\n"
             << tags::reset;

        vector<string> passNames = {
            "__CONTENTS__",
            "lexing\t",
            "preproc defs",
            "compile macros",
            "rules\t",
            "macro defs",
            "macro calls",
            "op subs\t",
            "sequencing",
        };

        // Get total according to this:
        unsigned long long int total = 0;
        for (auto t : phaseTimes)
        {
            total += t;
        }

        cout << "Total logged by compiler: " << total << '\n'
             << "By compiler pass (ns):\n";

        for (int j = 0; j < NUM_PHASES; j++)
        {
            cout << "\t" << j + 1
                 << "\t" << ((j < passNames.size()) ? passNames[j] : "UNNAMED\t")
                 << "\t" << phaseTimes[j]
                 << ((phaseTimes[j] < 10'000'000) ? "\t\t" : "\t")
                 << (100 * (double)phaseTimes[j] / total) << "%\n";
        }
        cout << "\t" << NUM_PHASES + 1
             << "\tC++ via Clang\t"
             << compElapsed
             << ((compElapsed < 10'000'000) ? "\t\t" : "\t") << '\n';

        cout << "\n"
             << "Output file: " << out << '\n';
    }

    return 0;
}
