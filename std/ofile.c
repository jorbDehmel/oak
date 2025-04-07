/*
Interfaces w/ oak output file
*/

#include "std_oak_header.h"
#include <stdio.h>
#include <string.h>

// OFile: struct
struct OFile {
  void *file_ptr;
};

// New(self: ^OFile) -> OFile
void New_FN_PTR_OFile_MAPS_void(struct OFile *self) {
  self->file_ptr = NULL;
}

// Del(self: ^OFile) -> void
void Del_FN_PTR_OFile_MAPS_void(struct OFile *self) {
  if (self->file_ptr != NULL) {
    fclose((FILE *)self->file_ptr);
    self->file_ptr = NULL;
  }
}

// open(self: ^OFile, path: []i8) -> void
void open_FN_PTR_OFile_JOIN_ARR_i8_MAPS_void(struct OFile *self,
                                             i8 *path) {
  // Open at beginning
  Del_FN_PTR_OFile_MAPS_void(self);
  self->file_ptr = fopen((char *)path, "w");
}

// append(self: ^OFile, path: []i8) -> void
void append_FN_PTR_OFile_JOIN_ARR_i8_MAPS_void(
    struct OFile *self, i8 *path) {
  // Open at end
  Del_FN_PTR_OFile_MAPS_void(self);
  self->file_ptr = fopen((char *)path, "a");
}

// is_open(self: ^OFile) -> bool
bool is_open_FN_PTR_OFile_MAPS_bool(struct OFile *self) {
  return self->file_ptr != NULL;
}

// write(self: ^OFile, what: []i8) -> void
void write_FN_PTR_OFile_JOIN_ARR_i8_MAPS_void(
    struct OFile *self, i8 *what) {
  fwrite((char *)what, 1, strlen((char *)what),
         (FILE *)self->file_ptr);
}

// tell(self: ^OFile) -> uint
uint tell_FN_PTR_OFile_MAPS_uint(struct OFile *self) {
  return ftell((FILE *)self->file_ptr);
}

// seek(self: ^OFile, where: uint) -> void
void seek_FN_PTR_OFile_JOIN_uint_MAPS_void(struct OFile *self,
                                           uint where) {
  fseek((FILE *)self->file_ptr, where, 0);
}

// write(self: ^OFile, what: int) -> void
void write_FN_PTR_OFile_JOIN_int_MAPS_void(struct OFile *self,
                                           int what) {
  fprintf((FILE *)self->file_ptr, "%i", what);
}

// write(self: ^OFile, what: uint) -> void
void write_FN_PTR_OFile_JOIN_uint_MAPS_void(struct OFile *self,
                                            uint what) {
  fprintf((FILE *)self->file_ptr, "%u", what);
}

// write(self: ^OFile, what: f64) -> void
void write_FN_PTR_OFile_JOIN_f64_MAPS_void(struct OFile *self,
                                           f64 what) {
  fprintf((FILE *)self->file_ptr, "%lf", what);
}

// write(self: ^OFile, ptr: ^void, n: uint) -> void
void write_FN_PTR_OFile_JOIN_PTR_void_JOIN_uint_MAPS_void(
    struct OFile *self, void *ptr, uint n) {
  fwrite(ptr, 1, n, (FILE *)self->file_ptr);
}
