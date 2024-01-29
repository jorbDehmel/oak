/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author

Package loading for Oak.
*/

#ifndef PACKAGES_HPP
#define PACKAGES_HPP

#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "tags.hpp"

const static std::string PACKAGE_TEMP_LOCATION = "/tmp/oak_packages/";
const static std::string PACKAGE_INCLUDE_PATH = "/usr/include/oak/";
const static std::string INFO_FILE = "oak_package_info.txt";

const static std::string PACKAGES_LIST_PATH = "/usr/include/oak/packages_list.txt";

const static std::string CLONE_COMMAND = "git clone ";

struct packageInfo
{
    std::string name;    // Package name
    std::string version; // Package version
    std::string license; // Package license
    std::string date;    // Date the current version was released
    std::string author;  // Self-explanitory
    std::string email;   // See maintainer name
    std::string source;  // URL package was downloaded from
    std::string path;    // Path from URL to get to the install point
    std::string about;   // Package description

    std::string toInclude; // File within /usr/include/oak/$(PACKAGE_NAME) to include!();

    std::string sysDeps;
    std::string oakDeps;
};

extern std::string installCommand;
void install(const std::string &What);

std::ostream &operator<<(std::ostream &strm, const packageInfo &info);

class package_error : public std::runtime_error
{
  public:
    package_error(const std::string &What) : std::runtime_error(What)
    {
    }
};

extern std::map<std::string, packageInfo> packages;

// Loads a package info file
packageInfo loadPackageInfo(const std::string &Filepath);

void loadAllPackages();

// Saves a package info file
void savePackageInfo(const packageInfo &Info, const std::string &Filepath);

// Downloads a package from a URL via git
void downloadPackage(const std::string &URL, const bool &Reinstall = false, const std::string &Path = "");

// Get the include!() -ed files of a package given name and possibly URL
std::vector<std::string> getPackageFiles(const std::string &Name);

#endif
