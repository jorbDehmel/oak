/*
Jordan Dehmel, 2023
GPLv3

Standard mem object files with std_mem.oak
This file must be compiled, as symbols are
provided during linking for cross-language
compilation.
*/

/*
// Allocate heap memory
let alloc<T>(num: i128) -> *T;
let alloc<T>() -> *T;

// Free heap memory
let free<T>(data: *T);

*/

template <class T>
T *alloc(unsigned long long int num)
{
    T *out = new T[num];
    return out;
}

template <class T>
T *alloc()
{
    return alloc<T>(1);
}

template <class T>
void free(T *what)
{
    delete[] what;
    return;
}
