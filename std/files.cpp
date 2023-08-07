/*
Jordan Dehmel, 2023
jdehmel@outlook.com

Interfacial file for Oak communication with C++'s
fstream library.
*/

#include "std_oak_header.hpp"
#include <fstream>
using namespace std;

// Struct definitions
struct i_file
{
    ifstream *raw_strm = nullptr;
};

struct o_file
{
    ofstream *raw_strm = nullptr;
};

// Method definitions
void New(i_file *self)
{
    if (self->raw_strm == nullptr)
    {
        self->raw_strm = new ifstream;
    }
}

void New(o_file *self)
{
    if (self->raw_strm == nullptr)
    {
        self->raw_strm = new ofstream;
    }
}

void Del(i_file *self)
{
    if (self->raw_strm != nullptr)
    {
        if (self->raw_strm->is_open())
        {
            self->raw_strm->close();
        }

        delete self->raw_strm;
    }
}

void Del(o_file *self)
{
    if (self->raw_strm != nullptr)
    {
        if (self->raw_strm->is_open())
        {
            self->raw_strm->close();
        }

        delete self->raw_strm;
    }
}

void open(i_file *self, str name)
{
    New(self);

    self->raw_strm->open(name);
}

void open(o_file *self, str name)
{
    New(self);

    self->raw_strm->open(name);
}

void close(i_file *self)
{
    if (self->raw_strm != nullptr)
    {
        self->raw_strm->close();
    }
}

void close(o_file *self)
{
    if (self->raw_strm != nullptr)
    {
        self->raw_strm->close();
    }
}

i8 read_char(i_file *self)
{
    i8 out = '\0';

    if (self->raw_strm != nullptr)
    {
        self->raw_strm->read(&out, 1);
    }

    return out;
}

void write_char(o_file *self, i8 what)
{
    i8 buf = what;

    if (self->raw_strm != nullptr)
    {
        self->raw_strm->write(&buf, 1);
    }

    return;
}

u128 tell(i_file *self)
{
    u128 out = 0;

    if (self->raw_strm != nullptr)
    {
        out = self->raw_strm->tellg();
    }

    return out;
}

u128 tell(o_file *self)
{
    u128 out = 0;

    if (self->raw_strm != nullptr)
    {
        out = self->raw_strm->tellp();
    }

    return out;
}

void seek(i_file *self, u128 pos)
{
    if (self->raw_strm != nullptr)
    {
        self->raw_strm->seekg(pos);
    }
}

void seek(o_file *self, u128 pos)
{
    if (self->raw_strm != nullptr)
    {
        self->raw_strm->seekp(pos);
    }
}
