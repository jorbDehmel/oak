#include <filesystem>
#include <std_oak_header.h>

extern "C" {

// fs_exists(path: []i8) -> bool
bool fs_exists_FN_ARR_i8_MAPS_bool(i8 *path) {
  return std::filesystem::exists((char *)path);
}

// fs_is_dir(path: []i8) -> bool
bool fs_is_dir_FN_ARR_i8_MAPS_bool(i8 *path) {
  return std::filesystem::is_directory((char *)path);
}

// fs_is_regular_file(path: []i8) -> bool
bool fs_is_regular_file_FN_ARR_i8_MAPS_bool(i8 *path) {
  return std::filesystem::is_regular_file((char *)path);
}

// fs_copy(path: []i8, path: []i8) -> void
void fs_copy_FN_ARR_i8_JOIN_ARR_i8_MAPS_void(i8 *src, i8 *dst) {
  std::filesystem::copy((char *)src, (char *)dst);
}

// fs_chdir(path: []i8) -> void
void fs_chdir_FN_ARR_i8_MAPS_void(i8 *path) {
  std::filesystem::current_path((char *)path);
}

// fs_mkdir(path: []i8) -> void
void fs_mkdir_FN_ARR_i8_MAPS_void(i8 *path) {
  std::filesystem::create_directory((char *)path);
}

// fs_remove(path: []i8) -> void
void fs_remove_FN_ARR_i8_MAPS_void(i8 *path) {
  std::filesystem::remove_all((char *)path);
}

} // extern "C"
