/*
Interfaces w/ oak input file
*/

#include "std_oak_header.h"
#include <stdio.h>

// IFile: struct
struct IFile {
  void *file_ptr;
};

// New(self: ^IFile) -> void
void New_FN_PTR_IFile_MAPS_void(struct IFile *self) {
  self->file_ptr = NULL;
}

// Del(self: ^IFile) -> void
void Del_FN_PTR_IFile_MAPS_void(struct IFile *self) {
  if (self->file_ptr != NULL) {
    fclose((FILE *)self->file_ptr);
    self->file_ptr = NULL;
  }
}

// open(self: ^IFile, path: []i8) -> void
void open_FN_PTR_IFile_JOIN_ARR_i8_MAPS_void(struct IFile *self,
                                             i8 *path) {
  // Open at beginning
  Del_FN_PTR_IFile_MAPS_void(self);
  self->file_ptr = fopen((char *)path, "r");
}

// open_at_end(self: ^IFile, path: []i8) -> void
void open_at_end_FN_PTR_IFile_JOIN_ARR_i8_MAPS_void(
    struct IFile *self, i8 *path) {
  // Open at end
  Del_FN_PTR_IFile_MAPS_void(self);
  self->file_ptr = fopen((char *)path, "r");
  fseek((FILE *)self->file_ptr, 0, SEEK_END);
}

// is_open(self: ^IFile) -> bool
bool is_open_FN_PTR_IFile_MAPS_bool(struct IFile *self) {
  return self->file_ptr != NULL;
}

// read(self: ^IFile, into: []i8, n: uint) -> void
void read_FN_PTR_IFile_JOIN_ARR_i8_JOIN_uint_MAPS_void(
    struct IFile *self, i8 *into, uint n) {
  fread((void *)into, 1, n, (FILE *)self->file_ptr);
}

// read(self: ^IFile, into: ^void, n: uint) -> void
void read_FN_PTR_IFile_JOIN_PTR_void_JOIN_uint_MAPS_void(
    struct IFile *self, void *into, uint n) {
  fread(into, 1, n, (FILE *)self->file_ptr);
}

// tell(self: ^IFile) -> uint
uint tell_FN_PTR_IFile_MAPS_uint(struct IFile *self) {
  return ftell((FILE *)self->file_ptr);
}

// seek(self: ^IFile, pos: uint) -> void
void seek_FN_PTR_IFile_JOIN_uint_MAPS_void(struct IFile *self,
                                           uint pos) {
  fseek((FILE *)self->file_ptr, pos, SEEK_SET);
}

// ignore(self: ^IFile, n: uint) -> void
void ignore_FN_PTR_IFile_JOIN_uint_MAPS_void(struct IFile *self,
                                             uint n) {
  fseek((FILE *)self->file_ptr, n, SEEK_CUR);
}

// read(self: ^IFile, into: ^int) -> void
void read_FN_PTR_IFile_JOIN_PTR_int_MAPS_void(
    struct IFile *self, int *into) {
  fscanf((FILE *)self->file_ptr, "%i", into);
}

// read(self: ^IFile, into: ^uint) -> void
void read_FN_PTR_IFile_JOIN_PTR_uint_MAPS_void(
    struct IFile *self, uint *into) {
  fscanf((FILE *)self->file_ptr, "%u", into);
}

// read(self: ^IFile, into: ^f64) -> void
void read_FN_PTR_IFile_JOIN_PTR_f64_MAPS_void(
    struct IFile *self, f64 *into) {
  fscanf((FILE *)self->file_ptr, "%lf", into);
}
