#include "package.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include "oakc.hpp"
#include <compare>
#include <filesystem>
#include <map>
#include <stdexcept>
#include <string>

std::string PackageManager::Version::package_suffix() const {
  std::string out;
  for (const auto &i : version) {
    out += "." + std::to_string(i);
  }
  return out;
}

std::strong_ordering PackageManager::Version::operator<=>(
    const Version &_other) const {
  std::list<uintmax_t> lhs_version = version;
  std::list<uintmax_t> rhs_version = _other.version;

  // Make them the same size
  while (lhs_version.size() < rhs_version.size()) {
    lhs_version.push_back(0);
  }
  while (rhs_version.size() < lhs_version.size()) {
    rhs_version.push_back(0);
  }

  for (auto l = lhs_version.begin(), r = rhs_version.begin();
       l != lhs_version.end() && r != rhs_version.end();
       ++l, ++r) {
    if (*l < *r) {
      return std::strong_ordering::less;
    } else if (*l > *r) {
      return std::strong_ordering::greater;
    }
  }

  return std::strong_ordering::equal;
}

PackageManager::Version
PackageManager::Version::from(const std::string &from) {
  PackageManager::Version v;
  for (unsigned long pos = 0, next = from.find(".", pos + 1);
       pos != std::string::npos;
       pos = next, next = from.find(".", pos + 1)) {
    if (pos != 0) {
      ++pos;
    }
    const auto segment = from.substr(pos, next - pos);

    if (segment.empty()) {
      throw std::runtime_error("In version '" + from +
                               "': Invalid version segment '" +
                               segment + "'");
    }
    for (const char &c : segment) {
      if ('0' > c || c > '9') {
        throw std::runtime_error(
            "In version '" + from +
            "': Invalid version segment '" + segment + "'");
      }
    }

    v.version.push_back(std::stoull(segment));
  }
  return v;
}

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
      "VERSION!"};

  const auto spec_file = _path / "spec.oak";
  std::map<std::string, std::string> out;
  const std::string package_name =
      spec_file.parent_path().filename().stem();

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
        c.macros.replace(junk, it, junk.end(), c.settings);
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

  if (!spec.contains("VERSION!")) {
    throw std::runtime_error("Package '" + spec.at("name") +
                             "' has no version!");
  }

  // Validate version string
  const auto raw = spec.at("VERSION!");
  Version v = Version::from(raw);
  const Version full_version = v;

  // Copy to path
  const auto name = spec.at("name");
  const auto real_path =
      _csettings.include_path / (name + v.package_suffix());
  std::filesystem::copy(
      _package, real_path,
      std::filesystem::copy_options::update_existing |
          std::filesystem::copy_options::recursive);

  // Symlinks
  while (!v.version.empty()) {
    v.version.pop_back();
    const auto symlink =
        _csettings.include_path / (name + v.package_suffix());
    bool should_symlink = true;

    if (std::filesystem::exists(symlink) &&
        !std::filesystem::is_symlink(symlink)) {
      continue;
    }

    for (const auto &d : std::filesystem::directory_iterator{
             _csettings.include_path}) {
      // If this file begins with the symlink
      if (!d.path().string().starts_with(symlink.string())) {
        continue;
      }

      // But is not the symlink
      else if (d == symlink) {
        continue;
      }

      const auto candidate_version =
          Version::from(d.path().string().substr(
              d.path().string().find(".") + 1));

      // If it is larger than the symlink's target,
      // don't symlink and break
      if (candidate_version > full_version) {
        should_symlink = false;
        break;
      }
    }

    if (should_symlink) {
      if (std::filesystem::exists(symlink) &&
          std::filesystem::is_symlink(symlink)) {
        std::filesystem::remove(symlink);
      }
      std::filesystem::create_directory_symlink(real_path,
                                                symlink);
    }
  }
}

/**
 * @brief Erases some package from the system
 */
void PackageManager::uninstall_package(
    const std::string &_name,
    const std::filesystem::path &_oak_include,
    const Version &_version) {
  debug_print();
  const auto path =
      _oak_include / (_name + _version.package_suffix());

  if (std::filesystem::exists(path)) {
    std::filesystem::remove_all(path);
  }
}

/**
 * @brief List all installed packages
 */
void PackageManager::list_packages(
    std::ostream &_to,
    const std::filesystem::path &_oak_include) {
  for (const auto &d :
       std::filesystem::directory_iterator{_oak_include}) {
    if (std::filesystem::is_directory(d)) {
      _to << d.path().stem().string() << '\n';
    } else if (std::filesystem::is_symlink(d)) {
      _to << d.path().stem().string() << " -> "
          << std::filesystem::read_symlink(d).stem().string()
          << '\n';
    }
  }
}
