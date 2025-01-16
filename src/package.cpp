#include "package.hpp"

Package PackageManager::install_local_package(
    const std::filesystem::path &_where) {
  throw std::runtime_error(__FUNCTION__);
}

Package
PackageManager::load_package(const Package::Spec &_spec) {
  throw std::runtime_error(__FUNCTION__);
}

void PackageManager::uninstall_package(
    const Package::Spec &_spec, const std::string &_engine) {
  throw std::runtime_error(__FUNCTION__);
}
