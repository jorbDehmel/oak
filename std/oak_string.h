/**
 * @brief C-side interface to Oak strings.
 */

#ifndef __OAK_STRING_H
#define __OAK_STRING_H

#include <oak/std/std_oak_header.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A self-resizing, dynamically-allocated, RAII string
 */
struct String {
  i8 *__c_str;
  uint __size;
};

// New(self: ^String) -> void
void New_FN_PTR_String_MAPS_void(struct String *self);

// Del(self: ^String) -> void
void Del_FN_PTR_String_MAPS_void(struct String *self);

// Copy(self: ^String, other: []i8) -> ^String
struct String *Copy_FN_PTR_String_JOIN_ARR_i8_MAPS_PTR_String(
    struct String *self, i8 *other);

// Copy(self: ^String, other: String) -> ^String
struct String *Copy_FN_PTR_String_JOIN_String_MAPS_PTR_String(
    struct String *self, struct String other);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
