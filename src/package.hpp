/**
 * @file package.hpp
 * @brief Resources for resolving packages in Oak
 */

#pragma once

#include "oakc.hpp"
static_assert(__cplusplus >= 2020'00ULL);

#include <filesystem>
#include <string>

/**
 * @namespace PackageManager
 * @brief Provides resources for loading and managing packages
 */
namespace PackageManager {

/**
 * @brief Installs a local package such that it is accessible
 * via the `package!` macro.
 * @param _package The path to the local version of the
 * package.
 * @param _csettings The settings for all of oak.
 */
void install_package(
    const std::filesystem::path &_package,
    const Settings::CompileSettings &_csettings);

/**
 * @brief Erases some package from the system
 */
void uninstall_package(
    const std::string &_name,
    const std::filesystem::path &_oak_include);

}; // namespace PackageManager
