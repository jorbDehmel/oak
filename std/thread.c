#include "oak/std/std_oak_header.h"
#include <pthread.h>
#include <string.h>

struct __pthread_wrapper_data {
  void *(*fn)(void *);
  void *arg;
};

struct Thread {
  pthread_t t;

  // Dynamically allocated RAII
  struct __pthread_wrapper_data *wrapper;

  // Padding
  u8 _[sizeof(u64) * 2 - sizeof(pthread_t) - sizeof(void *)];
};
static_assert(sizeof(struct Thread) == sizeof(u64) * 2,
              "Malformed sizing for pthread interface");

void *__pthread_wrapper_fn(void *arg) {
  struct __pthread_wrapper_data *const call_data =
      (struct __pthread_wrapper_data *const)arg;
  pthread_exit(call_data->fn(call_data->arg));
}

// Del(self: ^Thread) -> void
void Del_FN_PTR_Thread_MAPS_void(struct Thread *self) {
  if (self->t != 0) {
    pthread_detach(self->t);
    self->t = 0;
  }
  free(self->wrapper);
  self->wrapper = NULL;
}

// New(self: ^Thread) -> void
void New_FN_PTR_Thread_MAPS_void(struct Thread *self) {
  self->t = 0;
  self->wrapper = (struct __pthread_wrapper_data *)malloc(
      sizeof(struct __pthread_wrapper_data));
}

void Copy_FN_PTR_Thread_JOIN_PTR_FN_PTR_void_MAPS_PTR_void_JOIN_PTR_void_MAPS_void(
    struct Thread *self, void *(*fn)(void *_), void *data) {
  if (self->t != 0) {
    pthread_detach(self->t);
    self->t = 0;
  }

  // To enable our pthread wrapping while not having to take
  // addresses of static allocations in fn call
  self->wrapper->fn = fn;
  self->wrapper->arg = data;
  pthread_create(&self->t, NULL, __pthread_wrapper_fn,
                 self->wrapper);
}

// join(self: ^Thread) -> ^void
void *join_FN_PTR_Thread_MAPS_PTR_void(struct Thread *self) {
  void *out = NULL;
  pthread_join(self->t, &out);
  return out;
}

// detach(self: ^Thread) -> void
void detach_FN_PTR_Thread_MAPS_void(struct Thread *self) {
  pthread_detach(self->t);
  self->t = 0;
}
