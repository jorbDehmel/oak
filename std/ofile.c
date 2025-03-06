/*
Interfaces w/ oak output file
*/

#include "oak/std/std_oak_header.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// ofile: struct
struct ofile {
  void *file_ptr;
};

// New(self: ^ofile) -> ofile
struct ofile New_FN_PTR_ofile_MAPS_ofile(struct ofile *self) {
  self->file_ptr = NULL;
  return *self;
}

// Del(self: ^ofile) -> void
void Del_FN_PTR_ofile_MAPS_void(struct ofile *self) {
  if (self->file_ptr != NULL) {
    fclose((FILE *)self->file_ptr);
    self->file_ptr = NULL;
  }
}

// Copy(self: ^ofile, path: []i8) -> ofile
struct ofile
Copy_FN_PTR_ofile_JOIN_ARR_i8_MAPS_ofile(struct ofile *self,
                                         i8 *path) {
  Del_FN_PTR_ofile_MAPS_void(self);
  self->file_ptr = (void *)fopen((char *)path, "w");
  return *self;
}

// write(self: ^ofile, what: []i8) -> void
void write_FN_PTR_ofile_JOIN_ARR_i8_MAPS_void(
    struct ofile *self, i8 *what) {
  assert(self->file_ptr != NULL);
  fwrite(what, strlen((char *)what), 1, (FILE *)self->file_ptr);
}
