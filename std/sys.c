#include "/usr/include/oak/std_oak_header.h"
#include "string.c"
#include <stdio.h>

struct string s_sys_FN_str_MAPS_string(str command)
{
    struct string output;

    // Start process and open output as the file `pipe`
    FILE *pipe = popen(command, "r");

    // Seek to end and get the size of the file
    fseek(pipe, 0L, SEEK_END);
    output.capacity = ftell(pipe);

    // Allocate enough space to contain the file
    output.data = alloca(output.capacity);

    // Go back to the beginning
    fseek(pipe, 0L, SEEK_SET);

    // Load text
    fgets(output.data, output.capacity, pipe);

    // Close the pipe and get return value
    pclose(pipe);

    // Otherwise, return retrieved output
    return output;
}
