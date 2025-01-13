#include "package.hpp"
#include <stdexcept>

// Exact versions only
Package
DefaultManager::install_package(const Package::Spec &_spec) {
  throw std::runtime_error(__FUNCTION__);
}

// Exact versions only
void DefaultManager::uninstall_package(
    const Package::Spec &_spec) {
  throw std::runtime_error(__FUNCTION__);
}

// Returns a set of specific package instances which satisfy
// the given set of requirements
std::set<Package> DefaultManager::query_package(
    const std::string &_name,
    const std::set<std::pair<Package, std::strong_ordering>>
        &_restrictions) {
  throw std::runtime_error(__FUNCTION__);
}
