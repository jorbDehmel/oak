/*
Jordan Dehmel, 2023
jdehmel@outlook.com

Interfacial file for Oak communication with C++'s
fstream library.
*/

#include "/usr/include/oak/std_oak_header.h"
#include <stdio.h>

struct string
{
    i8 *data;
    u128 size;
};

extern str c_str_FN_PTR_string_MAPS_str(struct string *self);

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

void open_FN_PTR_i_file_JOIN_ARR_i8_MAPS_void(struct i_file *self, i8 name[])
{
    // Ensure no file is left open
    ExtDel_FN_PTR_i_file_MAPS_void(self);

    // Open file in write mode
    self->raw = fopen(name, "r");
}

void open_FN_PTR_o_file_JOIN_ARR_i8_MAPS_void(struct o_file *self, i8 name[])
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

struct string getline_FN_PTR_i_file_JOIN_u128_MAPS_string(struct i_file *self, u128 max)
{
    struct string out;

    out.size = max;
    out.data = (i8 *)malloc(out.size + 1);
    out.data[out.size] = 0;

    fgets(out.data, out.size, self->raw);

    return out;
}

bool eof_FN_PTR_i_file_MAPS_bool(struct i_file *self)
{
    return (feof(self->raw) != 0);
}

u128 size_FN_PTR_i_file_MAPS_u128(struct i_file *self)
{
    if (self->raw == 0)
    {
        return 0;
    }

    long temp = ftell(self->raw);
    fseek(self->raw, 0, SEEK_END);
    long out = ftell(self->raw);
    fseek(self->raw, temp, SEEK_SET);

    return out;
}
