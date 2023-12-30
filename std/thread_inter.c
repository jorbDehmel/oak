#include "/usr/include/oak/std_oak_header.h"
#include <pthread.h>

struct thread
{
    pthread_t raw;
};

// Create new thread
void ExtInit_FN_PTR_thread_MAPS_void(struct thread *self)
{
    self->raw = 0;
}

// let start(self: ^thread, to_do: ^() -> void) -> void;
// I easily could have gotten this mangle wrong
void start_FN_PTR_thread_JOIN_PTR_FN_MAPS_void_MAPS_void(struct thread *self, void *(*to_do)(void *))
{
    pthread_create(&self->raw, NULL, to_do, NULL);
}

// End a thread
void join_FN_PTR_thread_MAPS_void(struct thread *self)
{
    if (self->raw != 0)
    {
        pthread_join(self->raw, NULL);
        self->raw = 0;
    }
}
