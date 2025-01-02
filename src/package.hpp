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
#include <optional>
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
  std::list<Spec> dependencies;

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
  virtual Package install_package(const Package::Spec &_spec);
  virtual void uninstall_package(const Package::Spec &_spec);
  virtual std::optional<Package>
  query_package(const Package::Spec &_spec);
};

/**
 * @class PackageManager
 * @brief
 */
class PackageManager {};
