/*
Jordan Dehmel, 2023
jdehmel@outlook.com

Interfacial file for Oak communication with C++'s
fstream library.
*/

#include "oak/std_oak_header.h"
#include <stdio.h>

// Struct definitions
struct i_file
{
    FILE *raw;
};

struct o_file
{
    FILE *raw;
};

// Method definitions
void ExtInit_FN_PTR_i_file_MAPS_void(struct i_file *self)
{
    self->raw = 0;
}

void ExtInit_FN_PTR_o_file_MAPS_void(struct o_file *self)
{
    self->raw = 0;
}

void ExtDel_FN_PTR_i_file_MAPS_void(struct i_file *self)
{
    if (self->raw != 0)
    {
        fclose(self->raw);
        self->raw = 0;
    }
}

void ExtDel_FN_PTR_o_file_MAPS_void(struct o_file *self)
{
    if (self->raw != 0)
    {
        fclose(self->raw);
        self->raw = 0;
    }
}

void open_FN_PTR_i_file_JOIN_str_MAPS_void(struct i_file *self, str name)
{
    // Ensure no file is left open
    ExtDel_FN_PTR_i_file_MAPS_void(self);

    // Open file in read mode
    self->raw = fopen(name, "r");
}

void open_FN_PTR_o_file_JOIN_str_MAPS_void(struct o_file *self, str name)
{
    // Ensure no file is left open
    ExtDel_FN_PTR_o_file_MAPS_void(self);

    // Open file in write mode
    self->raw = fopen(name, "w");
}

void close_FN_PTR_i_file_MAPS_void(struct i_file *self)
{
    ExtDel_FN_PTR_i_file_MAPS_void(self);
}

void close_FN_PTR_o_file_MAPS_void(struct o_file *self)
{
    ExtDel_FN_PTR_o_file_MAPS_void(self);
}

i8 read_char_FN_PTR_i_file_MAPS_i8(struct i_file *self)
{
    i8 out = '\0';

    if (self->raw != 0)
    {
        fread(&out, 1, 1, self->raw);
    }

    return out;
}

void write_char_FN_PTR_o_file_JOIN_i8_MAPS_void(struct o_file *self, i8 what)
{
    if (self->raw != 0)
    {
        fwrite(&what, 1, 1, self->raw);
    }

    return;
}

u128 tell_FN_PTR_i_file_MAPS_u128(struct i_file *self)
{
    u128 out = 0;

    if (self->raw != 0)
    {
        out = ftell(self->raw);
    }

    return out;
}

u128 tell_FN_PTR_o_file_MAPS_u128(struct o_file *self)
{
    u128 out = 0;

    if (self->raw != 0)
    {
        out = ftell(self->raw);
    }

    return out;
}

void seek_FN_PTR_i_file_JOIN_u128_MAPS_void(struct i_file *self, u128 pos)
{
    if (self->raw != 0)
    {
        fseek(self->raw, 0, pos);
    }
}

void seek_FN_PTR_o_file_JOIN_u128_MAPS_void(struct o_file *self, u128 pos)
{
    if (self->raw != 0)
    {
        fseek(self->raw, 0, pos);
    }
}
