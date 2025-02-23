#include "package.hpp"
#include "lexer.hpp"
#include "oakc.hpp"
#include <filesystem>
#include <map>
#include <stdexcept>
#include <string>

/**
 * @brief
 * @param _path The DIRECTORY of the package
 */
std::map<std::string, std::string>
load_package_spec(const std::filesystem::path &_path) {
  // Not all of them, but the ones we need right now
  const static std::set<std::string> expected_key_suffixes = {
      "BUILD!", "INCLUDE!", "SOURCE!", "INSTALL!"};

  const auto spec_file = _path / "spec.oak";
  std::map<std::string, std::string> out;
  const std::string package_name =
      _path.parent_path().filename();

  out["name"] = package_name;

  if (std::filesystem::exists(spec_file)) {
    OakCompiler c;
    OakCompiler::Settings::CompileSettings &settings =
        c.settings.compile_settings();

    settings.mode =
        OakCompiler::Settings::CompileSettings::NOTHING;
    settings.entry_point = _path / "spec.oak";

    c();

    Lexer::Token t("", _path, 1, 0);
    for (const auto &suffix : expected_key_suffixes) {
      std::list<Lexer::Token> junk;
      junk.push_back(
          Lexer::Token(t, package_name + "_" + suffix));
      auto it = junk.begin();
      c.macros.replace(junk, it, junk.end());

      if (junk.size() > 0) {
        out[suffix] = junk.front();
      }
    }
  }

  return out;
}

/**
 * @brief Installs a local package.
 * @param _package The path to the local version of the
 * package.
 * @param _oak_include The include directory for all of oak.
 */
void PackageManager::install_package(
    const std::filesystem::path &_package,
    const std::filesystem::path &_oak_include) {
  if (!std::filesystem::exists(_package)) {
    throw std::runtime_error(_package.string() +
                             " does not exist.");
  } else if (!std::filesystem::is_directory(_package)) {
    throw std::runtime_error(_package.string() +
                             " is not a directory.");
  } else if (!std::filesystem::exists(_oak_include)) {
    // This is a non-issue: Just make it
    std::filesystem::create_directory(_oak_include);
  } else if (!std::filesystem::is_directory(_oak_include)) {
    throw std::runtime_error(
        _oak_include.string() +
        " include path is not a directory.");
  }

  const auto spec = load_package_spec(_package / "spec.oak");

  // Validate / build
  if (spec.contains("INSTALL!")) {
    OakCompiler c;
    OakCompiler::Settings::CompileSettings &settings =
        c.settings.compile_settings();

    settings.mode =
        OakCompiler::Settings::CompileSettings::TRANSLATE_ONLY;
    settings.entry_point = _package / spec.at("INSTALL!");

    try {
      c();
    } catch (std::runtime_error &e) {
      throw std::runtime_error("Error while building package " +
                               spec.at("name") + ":\n" +
                               e.what());
    } catch (...) {
      throw std::runtime_error(
          "Unknown error while building package " +
          spec.at("name"));
    }
  }

  // Copy to path
  std::filesystem::copy(
      _package, _oak_include / spec.at("name"),
      std::filesystem::copy_options::update_existing |
          std::filesystem::copy_options::recursive);
}

/**
 * @brief Erases some package from the system
 */
void PackageManager::uninstall_package(
    const std::string &_name,
    const std::filesystem::path &_oak_include) {
  if (std::filesystem::exists(_oak_include / _name)) {
    std::filesystem::remove(_oak_include / _name);
  }
}
