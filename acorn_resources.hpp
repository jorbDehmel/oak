/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#ifndef ACORN_RESOURCES_HPP
#define ACORN_RESOURCES_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <map>

#include "lexer.hpp"
#include "macros.hpp"
#include "reconstruct.hpp"
#include "symbol-table.hpp"
#include "type-builder.hpp"
#include "sequence.hpp"
#include "packages.hpp"
#include "op-sub.hpp"

#include "tags.hpp"
#include "sizer.hpp"

using namespace std;

#define VERSION "0.0.1"
#define LICENSE "GPLv3"
#define INFO "jdehmel@outlook.com"

#define DASHED_LINE "------------------------------------------\n"
const string helpText = "Acorn - Oak Standard Translator\n" DASHED_LINE
                        "Translates .oak files to .cpp.\n"
                        "Option | Verbose     | Purpose\n" DASHED_LINE
                        " -c    | --compile   | Produce object files\n"
                        " -d    | --debug     | Toggle debug mode\n"
                        " -e    | --clean     | Toggle erasure (default on)\n"
                        " -h    | --help      | Show this\n"
                        " -i    | --install   | Install a package\n"
                        " -l    | --link      | Produce executables\n"
                        " -n    | --no_save   | Produce nothing\n"
                        " -o    | --output    | Set the output file\n"
                        " -p    | --pretty    | Prettify C++ files\n"
                        " -q    | --quit      | Quit immediately\n"
                        " -r    | --reinstall | Reinstall a package\n"
                        " -s    | --size      | Show Oak disk usage\n"
                        " -t    | --translate | Produce C++ files\n"
                        " -u    | --dump      | Save dump files\n"
                        " -v    | --version   | Show version\n";

extern bool debug, compile, link, pretty, alwaysDump;
extern set<string> visitedFiles, cppSources, objects;
extern map<string, string> preprocDefines;

// Prints the cumulative disk usage of Oak (in human-readable)
void getDiskUsage();

// I is the point in Lexed at which a macro name was found
// CONSUMPTIVE!
vector<string> getMacroArgs(vector<string> &lexed, const int &i);

void doFile(const string &From);
void prettify(const string &Filename);

#endif
