/**
 * @file
 * @brief Resources for resolving packages in Oak
 */

#pragma once

#include "settings.hpp"
#include <compare>
#include <cstdint>
#include <filesystem>
#include <string>

/**
 * @namespace PackageManager
 * @brief Provides resources for loading and managing packages
 */
namespace PackageManager {

/**
 * @brief A package version
 */
struct Version {
  /// First is major, each afterwards is less influential
  std::list<uintmax_t> version;

  /// Parse from a version string (e.g. "0.1.2.3")
  Version(const std::string &_text = "");

  /// Return the thing that should be appended to the package
  /// name in order for it to be valid. For instance, package
  /// "foo" with version "1.2.3" would become "foo.1.2.3", so
  /// this returns ".1.2.3".
  std::string package_suffix() const;

  /// Spaceship operator for ordering versions: The zeroth item
  /// in the version list is the most powerful, with decreasing
  /// influence from there. If two packages have versions of
  /// different length, the shorter is appended with zeros.
  std::strong_ordering operator<=>(const Version &_other) const;
};

/**
 * @brief Installs a local package such that it is accessible
 * via the `include!` macro.
 * @param _package The path to the local version of the
 * package.
 * @param _settings The settings for all of oak.
 * NOTE: The version will be derived from the spec file.
 */
void install_package(const std::filesystem::path &_package,
                     Settings &_settings);

/**
 * @brief Erases some package from the system
 * @param _name The package name
 * @param _oak_include The dir where all oak packages are
 * @param _settings The compiler settings
 * @param _version The version to uninstall
 * NOTE: Version {} will uninstall everything, version {0} will
 * uninstall all 0*, version {0, 1} will uninstall all 0.1*, etc
 */
void uninstall_package(
    const std::string &_name,
    const std::filesystem::path &_oak_include,
    Settings &_settings, const Version &_version = {});

/**
 * @brief List all installed packages
 */
void list_packages(std::ostream &_to,
                   const std::filesystem::path &_oak_include);

/**
 * @brief Loads the `dir/spec.oak` file
 * @param _path The DIRECTORY of the package
 * @param _settings The settings, which include the paths to
 * search
 */
std::map<std::string, std::string>
load_package_spec(const std::filesystem::path &_path,
                  Settings &_settings);

}; // namespace PackageManager
