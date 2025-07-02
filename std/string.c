#include "oak_string.h"
#include <alloca.h>
#include <string.h>

// New(self: ^String) -> void
void New_FN_PTR_String_MAPS_void(struct String *self) {
  self->__size = (uint)0;
}

// Del(self: ^String) -> void
void Del_FN_PTR_String_MAPS_void(struct String *self) {
  if (self->__size != (uint)0) {
    free(self->__c_str);
    self->__size = (uint)0;
  }
}

// Copy(self: ^String, other: []i8) -> ^String
struct String *Copy_FN_PTR_String_JOIN_ARR_i8_MAPS_PTR_String(
    struct String *self, i8 *other) {
  Del_FN_PTR_String_MAPS_void(self);
  self->__size = strlen((char *)other);
  self->__c_str = (i8 *)alloca(self->__size);
  strcpy((char *)self->__c_str, (char *)other);
  self->__c_str[self->__size] = (i8)0;
  return self;
}

// Copy(self: ^String, other: String) -> ^String
struct String *Copy_FN_PTR_String_JOIN_String_MAPS_PTR_String(
    struct String *self, struct String other) {
  Copy_FN_PTR_String_JOIN_ARR_i8_MAPS_PTR_String(self,
                                                 other.__c_str);
  return self;
}
