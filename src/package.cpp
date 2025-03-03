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
  const static std::set<char> str_chars = {'\'', '"', '`'};

  // Not all of them, but the ones we need right now
  const static std::set<std::string> expected_key_suffixes = {
      "INSTALL!"};

  const auto spec_file = _path / "spec.oak";
  std::map<std::string, std::string> out;
  const std::string package_name =
      spec_file.parent_path().filename();

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
        std::string contents = junk.front();

        while (!contents.empty() &&
               contents.front() == contents.back() &&
               str_chars.contains(contents.front())) {
          contents = contents.substr(1, contents.size() - 2);
        }

        if (!contents.empty()) {
          out[suffix] = contents;
        }
      }
    }
  } else {
    throw std::runtime_error("Missing spec file " +
                             spec_file.string());
  }

  return out;
}

void PackageManager::install_package(
    const std::filesystem::path &_package,
    const OakCompiler::Settings::CompileSettings &_csettings) {
  if (!std::filesystem::exists(_package)) {
    throw std::runtime_error(_package.string() +
                             " does not exist.");
  } else if (!std::filesystem::is_directory(_package)) {
    throw std::runtime_error(_package.string() +
                             " is not a directory.");
  } else if (!std::filesystem::exists(
                 _csettings.include_path)) {
    // This is a non-issue: Just make it
    std::filesystem::create_directory(_csettings.include_path);
  } else if (!std::filesystem::is_directory(
                 _csettings.include_path)) {
    throw std::runtime_error(
        _csettings.include_path.string() +
        " include path is not a directory.");
  }

  const auto spec = load_package_spec(_package);

  // Validate / build
  if (spec.contains("INSTALL!")) {
    OakCompiler c;
    c.settings.compile_settings() = _csettings;
    OakCompiler::Settings::CompileSettings &settings =
        c.settings.compile_settings();

    settings.mode =
        OakCompiler::Settings::CompileSettings::TRANSLATE_ONLY;
    settings.entry_point = spec.at("INSTALL!");

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
      _package, _csettings.include_path / spec.at("name"),
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
