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

#include "options.hpp"
#include "tags.hpp"

// Holds information about an installed package.
struct PackageInfo
{
    std::string name;      // Package name
    std::string version;   // Package version
    std::string license;   // Package license
    std::string date;      // Date the current version was released
    std::string author;    // The author(s) of the package
    std::string email;     // The email(s) of the author(s)
    std::string source;    // URL package was downloaded from
    std::string path;      // Path from URL to get to the install point
    std::string about;     // Package description
    std::string toInclude; // File within /usr/include/oak/$(PACKAGE_NAME) to include;
    std::string sysDeps;   // System dependencies
    std::string oakDeps;   // Oak dependencies
};

// Installs a given SYSTEM package; NOT an Oak one.
void install(const std::string &what);

// Output package information to an output stream in a pretty
// way.
std::ostream &operator<<(std::ostream &strm, const PackageInfo &info);

// An error which may occur during package loading.
class package_error : public std::runtime_error
{
  public:
    package_error(const std::string &what) : std::runtime_error(what)
    {
    }
};

// Loads a package info file.
PackageInfo loadPackageInfo(const std::string &filepath);

// Loads all packages which are currently known about.
void loadAllPackages();

// Saves a package info file.
void savePackageInfo(const PackageInfo &info, const std::string &filepath);

// Downloads a package from a URL via git.
void downloadPackage(const std::string &url, const bool &reinstall = false, const std::string &path = "");

// Get the include!() -ed files of a package given name and
// possibly URL.
std::vector<std::string> getPackageFiles(const std::string &name);

#endif
