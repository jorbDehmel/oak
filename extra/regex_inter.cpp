#include "/usr/include/oak/std/string.c"
#include "/usr/include/oak/std_oak_header.h"

#define BOOST_REGEX_STANDALONE
#include <boost/regex.hpp>

struct extra_regex
{
    boost::regex *re;

    char padding[32 - sizeof(boost::regex *)]; // To get to 32 bytes
};

struct extra_regex_smatch
{
    boost::smatch m;

    char padding[96 - sizeof(boost::smatch)]; // To get to 96 bytes
};

extern "C"
{
    bool extra_regex_match_FN_PTR_string_JOIN_PTR_extra_regex_MAPS_bool(string *text, extra_regex *pattern)
    {
        return boost::regex_match(std::string(text->data), *pattern->re);
    }

    bool extra_regex_match_FN_PTR_string_JOIN_PTR_extra_regex_JOIN_PTR_extra_regex_smatch_MAPS_bool(
        string *text, extra_regex *pattern, extra_regex_smatch *into)
    {
        return boost::regex_match(std::string(text->data), into->m, *pattern->re);
    }

    bool extra_regex_search_FN_PTR_string_JOIN_PTR_extra_regex_MAPS_bool(string *text, extra_regex *pattern)
    {
        return boost::regex_search(std::string(text->data), *pattern->re);
    }

    bool extra_regex_search_FN_PTR_string_JOIN_PTRextra__regex_JOIN_PTR_extra_regex_smatch_MAPS_bool(
        string *text, extra_regex *pattern, extra_regex_smatch *into)
    {
        return boost::regex_search(std::string(text->data), into->m, *pattern->re);
    }

    //////////// Methods ////////////

    void New_FN_PTR_extra_regex_MAPS_void(extra_regex *self)
    {
        memset(self, 0, sizeof(extra_regex));
        self->re = nullptr;
    }

    void Del_FN_PTR_extra_regex_MAPS_void(extra_regex *self)
    {
        if (self->re != nullptr)
        {
            delete self->re;
        }
    }

    extra_regex Copy_FN_PTR_extra_regex_JOIN_PTR_string_MAPS_extra_regex(extra_regex *self, string *pattern)
    {
        if (self->re != nullptr)
        {
            delete self->re;
        }

        self->re = new boost::regex(pattern->data);
        return *self;
    }

    extra_regex Copy_FN_PTR_extra_regex_JOIN_str_MAPS_extra_regex(extra_regex *self, str pattern)
    {
        if (self->re != nullptr)
        {
            delete self->re;
        }

        self->re = new boost::regex(pattern);
        return *self;
    }

    void New_FN_PTR_extra_regex_smatch_MAPS_void(extra_regex_smatch *self)
    {
        memset(self, 0, sizeof(extra_regex_smatch));
    }

    u64 size_FN_PTR_extra_regex_smatch_MAPS_u64(extra_regex_smatch *self)
    {
        return self->m.size();
    }

    string str_FN_PTR_extra_regex_smatch_MAPS_string(extra_regex_smatch *self)
    {
        string out;
        out.current_length = self->m.str().size();
        out.capacity = out.current_length + 1;

        out.data = new i8[out.capacity];
        strcpy(out.data, self->m.str().c_str());
        out.data[out.current_length] = '\0';

        return out;
    }

    string Get_FN_PTR_extra_smatch_JOIN_u32_MAPS_string(extra_regex_smatch *self, u32 index)
    {
        string out;
        std::string from = self->m[index].str();

        out.current_length = from.size();
        out.capacity = out.current_length + 1;

        out.data = new i8[out.capacity];
        strcpy(out.data, from.c_str());
        out.data[out.current_length] = '\0';

        return out;
    }
}
