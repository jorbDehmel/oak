/**
 * @brief C-side interface to Oak tokens.
 */

#ifndef __OAK_TokenList_H
#define __OAK_TokenList_H

#ifdef __cplusplus
extern "C" {
#endif

#include "./oak_string.h"

struct TokenList {
  String text;
  String type;
  String file;
  u64 line;
  u64 col;
  TokenList *__next;
};

// Del(self: ^TokenList) -> void
void Del_FN_PTR_TokenList_MAPS_void(struct TokenList *self);

// push(self: ^TokenList, other: TokenList) -> void
void push_FN_PTR_TokenList_JOIN_TokenList_MAPS_void(
    struct TokenList *self, struct TokenList other);

// push(self: ^TokenList, other: String) -> void
void push_FN_PTR_TokenList_JOIN_String_MAPS_void(
    struct TokenList *self, struct String other);

// let push(self: ^TokenList, other: String, type: String) ->
// void
void push_FN_PTR_TokenList_JOIN_String_JOIN_String_MAPS_void(
    struct TokenList *self, struct String other,
    struct String type);

// done(self: ^TokenList) -> bool
bool done_FN_PTR_TokenList_MAPS_bool(struct TokenList *self);

// next(self: ^^TokenList) -> void
void next_FN_PTR_PTR_TokenList_MAPS_void(
    struct Tokenstruct **self);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
