#include "package.hpp"
#include "debug.hpp"
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
  debug_print();
  const static std::set<char> str_chars = {'\'', '"', '`'};

  // Not all of them, but the ones we need right now
  const static std::set<std::string> expected_key_suffixes = {
      "INSTALL!", "VERSION!"};

  const auto spec_file = _path / "spec.oak";
  std::map<std::string, std::string> out;
  const std::string package_name =
      spec_file.parent_path().filename();

  out["name"] = package_name;

  if (std::filesystem::exists(spec_file)) {
    OakCompiler c;
    Settings::CompileSettings &settings =
        c.settings.compile_settings();

    settings.mode = Settings::CompileSettings::NOTHING;
    settings.entry_point = _path / "spec.oak";

    c();

    Lexer::Token t("", _path, 1, 0);
    for (const auto &suffix : expected_key_suffixes) {
      std::list<Lexer::Token> junk, prev;
      junk.push_back(
          Lexer::Token(t, package_name + "_" + suffix));
      auto it = junk.begin();
      do {
        prev = junk;
        c.macros.replace(junk, it, junk.end());
      } while (junk != prev);

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
    const Settings::CompileSettings &_csettings) {
  debug_print();
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
    Settings::CompileSettings &settings =
        c.settings.compile_settings();

    settings.mode = Settings::CompileSettings::TRANSLATE_ONLY;
    settings.entry_point = spec.at("INSTALL!");

    try {
      c();
    } catch (std::runtime_error &e) {
      throw std::runtime_error("Error while building package " +
                               spec.at("name") + ":\n" +
                               e.what());
    } catch (...) {
      db_rethrow();
      throw std::runtime_error(
          "Unknown error while building package " +
          spec.at("name"));
    }
  }

  if (spec.contains("VERSION!")) {
    // Validate version string
    /*
    0.0.0.0 = 0.0.0 = 0.0 = 0
    1.0 > 1 = 0.1
    1.0.0 > 1.0 = 0.1.0 > 1 = 0.0.1
    Arbitrarily many segments, but usually three
    patch, minor, major, super-major, super-super-major, etc
    */
    char prev = '\0';
    for (const char &c : spec.at("VERSION!")) {
      if (c == '.') {
        if (prev == '.') {
          throw std::runtime_error("Invalid package version '" +
                                   spec.at("VERSION!") + "'");
        }
      } else if (c < '0' || '9' < c) {
        throw std::runtime_error("Illegal character '" +
                                 std::string({c}) +
                                 "' in package version '" +
                                 spec.at("VERSION!") + "'");
      }
      prev = c;
    }
    if (prev == '.') {
      throw std::runtime_error("Invalid package version '" +
                               spec.at("VERSION!") + "'");
    }
  } else {
    throw std::runtime_error("Package '" + spec.at("name") +
                             "' has no version!");
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
  debug_print();
  if (std::filesystem::exists(_oak_include / _name)) {
    std::filesystem::remove(_oak_include / _name);
  }
}
