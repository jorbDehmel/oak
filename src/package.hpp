/**
 * @file package.hpp
 * @brief
 */

#pragma once

static_assert(__cplusplus >= 2020'00ULL);

#include <compare>
#include <cstdint>
#include <filesystem>
#include <list>
#include <map>
#include <set>
#include <string>

/**
 * @struct Package
 * @brief
 */
struct Package {
  /**
   * @struct Spec
   * @brief
   */
  struct Spec {
    ///
    std::string name;

    ///
    uint64_t major, minor, patch;

    /**
     * @brief
     */
    inline std::strong_ordering
    operator<=>(const Spec &_o) const {
      const std::strong_ordering maj = (major <=> _o.major);
      const std::strong_ordering min = (minor <=> _o.minor);
      const std::strong_ordering pat = (patch <=> _o.patch);
      return (maj != 0) ? maj : (min != 0) ? min : pat;
    }
  };

  ///
  std::list<std::pair<Spec, std::strong_ordering>> dependencies;

  ///
  std::string source;

  ///
  std::filesystem::path include;
};

/**
 * @class PackageManagementEngine
 * @brief Virtual class to allow varied handling of packages
 */
class PackageManagementEngine {
public:
  // Exact versions only
  virtual Package
  install_package(const Package::Spec &_spec) = 0;

  // Exact versions only
  virtual void
  uninstall_package(const Package::Spec &_spec) = 0;

  // Returns a set of specific package instances which satisfy
  // the given set of requirements
  virtual std::set<Package> query_package(
      const std::string &_name,
      const std::set<std::pair<Package, std::strong_ordering>>
          &_restrictions) = 0;
};

/**
 * @class DefaultManager
 * @brief
 */
class DefaultManager : public PackageManagementEngine {
public:
  // Exact versions only
  Package install_package(const Package::Spec &_spec);

  // Exact versions only
  void uninstall_package(const Package::Spec &_spec);

  // Returns a set of specific package instances which satisfy
  // the given set of requirements
  std::set<Package> query_package(
      const std::string &_name,
      const std::set<std::pair<Package, std::strong_ordering>>
          &_restrictions);
};

/**
 * @class PackageManager
 * @brief
 */
class PackageManager {
public:
  DefaultManager default_manager;
  std::map<std::string, PackageManagementEngine *> engines;

  inline Package
  install_package(const Package::Spec &_spec,
                  const std::string &_engine = "") {
    if (_engine == "") {
      return default_manager.install_package(_spec);
    } else {
      return engines.at(_engine)->install_package(_spec);
    }
  }

  inline void
  uninstall_package(const Package::Spec &_spec,
                    const std::string &_engine = "") {
    if (_engine == "") {
      default_manager.uninstall_package(_spec);
    } else {
      engines.at(_engine)->uninstall_package(_spec);
    }
  }

  inline std::set<Package> query_package(
      const std::string &_name,
      const std::set<std::pair<Package, std::strong_ordering>>
          &_restrictions,
      const std::string &_engine = "") {
    if (_engine == "") {
      default_manager.query_package(_name, _restrictions);
    } else {
      return engines.at(_engine)->query_package(_name,
                                                _restrictions);
    }
  }
};
