#include <thread>

struct thread
{
    std::thread *rawThread = nullptr;
};

// To be overloaded in Oak
void call(thread *self);

// Create new thread
void New(thread *self)
{
    self->rawThread = nullptr;
}

// let start(self: ^thread, to_do: ^() -> void) -> void;
void start(thread *self, void (*to_do)())
{
    self->rawThread = new std::thread(to_do);
}

// End a thread
void join(thread *self)
{
    if (self->rawThread != nullptr)
    {
        self->rawThread->join();
        delete self->rawThread;
        self->rawThread = nullptr;
    }
}

// Same as join
void Del(thread *self)
{
    if (self->rawThread != nullptr)
    {
        self->rawThread->join();
        delete self->rawThread;
        self->rawThread = nullptr;
    }
}
