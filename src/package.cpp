#include "package.hpp"
#include "compiler.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include "symbols.hpp"
#include <compare>
#include <filesystem>
#include <map>
#include <stdexcept>
#include <string>
#include <variant>

/**
 * @brief Given some package information, add internal
 * versioning symlinks
 */
void add_symlinks(const std::string &_name,
                  const PackageManager::Version &_full_version,
                  const std::filesystem::path &_real_path,
                  const std::filesystem::path &_include_path) {
  PackageManager::Version v = _full_version;

  while (!v.version.empty()) {
    v.version.pop_back();
    const auto symlink =
        _include_path / (_name + v.package_suffix());
    bool should_symlink = true;

    if (std::filesystem::exists(symlink) &&
        !std::filesystem::is_symlink(symlink)) {
      continue;
    }

    for (const auto &d :
         std::filesystem::directory_iterator{_include_path}) {
      // If this file begins with the symlink
      if (!d.path().string().starts_with(symlink.string())) {
        continue;
      }

      // But is not the symlink
      else if (d == symlink) {
        continue;
      }

      const auto candidate_version =
          PackageManager::Version(d.path().string().substr(
              d.path().string().find(".") + 1));

      // If it is larger than the symlink's target,
      // don't symlink and break
      if (candidate_version > _full_version) {
        should_symlink = false;
        break;
      }
    }

    if (should_symlink) {
      if (std::filesystem::exists(symlink) &&
          std::filesystem::is_symlink(symlink)) {
        std::filesystem::remove(symlink);
      }
      std::filesystem::create_directory_symlink(_real_path,
                                                symlink);
    }
  }
}

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

PackageManager::Version::Version(const std::string &from) {
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

    version.push_back(std::stoull(segment));
  }
}

std::map<std::string, std::string>
PackageManager::load_package_spec(
    const std::filesystem::path &_path, Settings &_settings) {
  debug_print();
  if (_settings.debug) {
    _settings.ostream << "Loading package spec " << _path
                      << '\n';
  }

  std::map<std::string, std::string> out;

  const auto spec_file = _path / "spec.oak";
  if (!std::filesystem::exists(spec_file)) {
    throw std::runtime_error("Missing spec file " +
                             spec_file.string());
  }

  std::string package_name = spec_file.parent_path().filename();
  if (package_name.find('.') != std::string::npos) {
    package_name =
        package_name.substr(0, package_name.find('.'));
  }

  out["name"] = package_name;

  OakCompiler c(_settings.ostream);
  Settings::CompileSettings &settings =
      c.settings.compile_settings();
  settings = _settings.compile_settings();
  c.settings.debug = _settings.debug;
  settings.mode = Settings::CompileSettings::NOTHING;
  settings.entry_point = _path / "spec.oak";

  c();

  for (const auto &name : c.p.scope_manager.names()) {
    if (name.starts_with(package_name + "_") &&
        name.ends_with("!")) {

      TokenStream contents(
          {Lexer::Token(name, spec_file, 0, 0)});

      bool keep_going = true;
      while (keep_going) {
        keep_going = c.p.replace_macro(contents);
      }

      std::string to_add;
      for (const auto &tok : contents) {
        if (!to_add.empty()) {
          to_add.push_back(' ');
        }
        to_add += tok.text;
      }

      out[name.substr(package_name.size() + 1)] =
          Macros::strip_string_literal(to_add);
    }
  }

  return out;
}

void PackageManager::install_package(
    const std::filesystem::path &_package,
    Settings &_settings) {
  debug_print();
  if (!std::filesystem::exists(_package)) {
    throw std::runtime_error(_package.string() +
                             " does not exist.");
  } else if (!std::filesystem::is_directory(_package)) {
    throw std::runtime_error(_package.string() +
                             " is not a directory.");
  } else if (!std::filesystem::exists(
                 _settings.compile_settings().include_path)) {
    // This is a non-issue: Just make it
    std::filesystem::create_directory(
        _settings.compile_settings().include_path);
  } else if (!std::filesystem::is_directory(
                 _settings.compile_settings().include_path)) {
    throw std::runtime_error(
        _settings.compile_settings().include_path.string() +
        " include path is not a directory.");
  }

  const auto spec =
      PackageManager::load_package_spec(_package, _settings);

  // Validate / build
  if (spec.contains("INSTALL!")) {
    if (_settings.debug) {
      _settings.ostream << "Running prescribed install script "
                        << spec.at("INSTALL!") << '\n';
    }

    OakCompiler c(_settings.ostream);
    c.settings.debug = _settings.debug;
    c.settings.compile_settings() =
        _settings.compile_settings();
    Settings::CompileSettings &settings =
        c.settings.compile_settings();

    settings.mode = Settings::CompileSettings::TRANSLATE_ONLY;
    settings.entry_point = spec.at("INSTALL!");

    try {
      c();
    } catch (OutOfPPPLError &e) {
      throw OutOfPPPLError("Error while building package " +
                           spec.at("name") + ":\n" + e.what());
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

  if (!spec.contains("VERSION!")) {
    throw std::runtime_error("Package '" + spec.at("name") +
                             "' has no version!");
  }

  const auto raw = spec.at("VERSION!");
  const Version full_version = Version(raw);
  const auto name = spec.at("name");
  const auto real_path =
      _settings.compile_settings().include_path /
      (name + full_version.package_suffix());

  std::filesystem::copy(
      _package, real_path,
      std::filesystem::copy_options::update_existing |
          std::filesystem::copy_options::recursive);
  std::filesystem::permissions(real_path,
                               std::filesystem::perms::all);

  add_symlinks(name, full_version, real_path,
               _settings.compile_settings().include_path);
}

/**
 * @brief Erases some package from the system
 */
void PackageManager::uninstall_package(
    const std::string &_name,
    const std::filesystem::path &_oak_include,
    Settings &_settings, const Version &_version) {
  debug_print();
  const auto path =
      _oak_include / (_name + _version.package_suffix());

  // Erase all symlinks
  for (const auto &f :
       std::filesystem::directory_iterator{_oak_include}) {
    if (std::filesystem::is_symlink(f)) {
      std::filesystem::remove(f);
    } else if (f.path().string().starts_with(path.string())) {
      std::filesystem::remove_all(f);
    }
  }

  // Rebuild all symlinks
  for (const auto &f :
       std::filesystem::directory_iterator{_oak_include}) {
    if (std::filesystem::is_directory(f)) {
      const auto spec =
          PackageManager::load_package_spec(f, _settings);
      if (!spec.contains("VERSION!")) {
        throw std::runtime_error("Package '" + spec.at("name") +
                                 "' has no version!");
      }
      const Version full_version = Version(spec.at("VERSION!"));
      const auto name = spec.at("name");
      const auto real_path =
          _oak_include / (name + full_version.package_suffix());
      add_symlinks(name, full_version, real_path, _oak_include);
    }
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
    if (std::filesystem::is_symlink(d)) {
      _to << d.path().string() << " -> "
          << std::filesystem::read_symlink(d).string() << '\n';
    } else if (std::filesystem::is_directory(d)) {
      _to << d.path().string() << '\n';
    }
  }
}
