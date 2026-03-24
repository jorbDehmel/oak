/**
 * @brief C-side interface to Oak tokens.
 */

#include "token.h"
#include "oak_string.h"

// New(self: ^TokenList) -> void
void New_FN_PTR_TokenList_MAPS_void(struct TokenList *);

// Del(self: ^TokenList) -> void
void Del_FN_PTR_TokenList_MAPS_void(struct TokenList *self) {
  Del_FN_PTR_String_MAPS_void(&self->text);
  Del_FN_PTR_String_MAPS_void(&self->file);
  if (self->__next != 0) {
    Del_FN_PTR_TokenList_MAPS_void(self->__next);
    free(self->__next);
    self->__next = 0;
  }
}

// push(self: ^TokenList, other: TokenList) -> void
void push_FN_PTR_TokenList_JOIN_TokenList_MAPS_void(
    struct TokenList *self, struct TokenList other) {
  TokenList *to_append = (TokenList *)alloca(sizeof(TokenList));
  New_FN_PTR_TokenList_MAPS_void(to_append);

  Copy_FN_PTR_String_JOIN_String_MAPS_PTR_String(
      &to_append->text, other.text);
  Copy_FN_PTR_String_JOIN_String_MAPS_PTR_String(
      &to_append->file, other.file);

  to_append->col = other.col;
  to_append->line = other.line;

  to_append->__next = self->__next;
  self->__next = to_append;
}

// push(self: ^TokenList, other: String) -> void
void push_FN_PTR_TokenList_JOIN_String_MAPS_void(
    struct TokenList *self, struct String other) {
  TokenList *to_append = (TokenList *)alloca(sizeof(TokenList));
  New_FN_PTR_TokenList_MAPS_void(to_append);

  Copy_FN_PTR_String_JOIN_String_MAPS_PTR_String(
      &to_append->text, other);
  Copy_FN_PTR_String_JOIN_String_MAPS_PTR_String(
      &to_append->file, self->file);

  to_append->col = self->col;
  to_append->line = self->line;

  to_append->__next = self->__next;
  self->__next = to_append;
}

// done(self: ^TokenList) -> bool
bool done_FN_PTR_TokenList_MAPS_bool(struct TokenList *self) {
  return self != 0 && self->__next != 0;
}

// next(self: ^^TokenList) -> void
void next_FN_PTR_PTR_TokenList_MAPS_void(
    struct TokenList **self) {
  if (self != 0 && !done_FN_PTR_TokenList_MAPS_bool(*self)) {
    *self = (*self)->__next;
  }
}
