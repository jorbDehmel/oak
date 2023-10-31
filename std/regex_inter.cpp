#include <boost/regex.hpp>
#include <string>

#include "oak/std_oak_header.h"
#include "oak/std/string.c"

struct regex
{
    boost::regex re;

    char padding[16]; // To get to 32 bytes
};

struct regex_smatch
{
    boost::smatch m;

    char padding[16]; // To get to 96 bytes
};

extern "C"
{
    bool regex_match_FN_PTR_string_JOIN_PTR_regex_MAPS_bool(string *text, regex *pattern)
    {
        return boost::regex_match(std::string(text->data), pattern->re);
    }

    bool regex_match_FN_PTR_string_JOIN_PTR_regex_JOIN_PTR_regex_smatch_MAPS_bool(string *text, regex *pattern, regex_smatch *into)
    {
        return boost::regex_match(std::string(text->data), into->m, pattern->re);
    }

    bool regex_search_FN_PTR_string_JOIN_PTR_regex_MAPS_bool(string *text, regex *pattern)
    {
        return boost::regex_search(std::string(text->data), pattern->re);
    }

    bool regex_search_FN_PTR_string_JOIN_PTR_regex_JOIN_PTR_regex_smatch_MAPS_bool(string *text, regex *pattern, regex_smatch *into)
    {
        return boost::regex_search(std::string(text->data), into->m, pattern->re);
    }

    //////////// Methods ////////////

    // Copy_FN_PTR_regex_JOIN_PTR_string_MAPS_void
    void Copy_FN_PTR_regex_JOIN_PTR_string_MAPS_void(regex *self, string *pattern)
    {
        self->re = boost::regex(pattern->data);
    }

    void Copy_FN_PTR_regex_JOIN_str_MAPS_void(regex *self, str pattern)
    {
        self->re = pattern;
    }

    u64 size_FN_PTR_regex_smatch_MAPS_u64(regex_smatch *self)
    {
        return self->m.size();
    }

    string str_FN_PTR_regex_smatch_MAPS_string(regex_smatch *self)
    {
        string out;
        out.size = self->m.str().size();

        out.data = new i8[out.size];
        strcpy(out.data, self->m.str().c_str());

        return out;
    }

    string Get_FN_PTR_smatch_JOIN_u32_MAPS_string(regex_smatch *self, u32 index)
    {
        string out;
        std::string from = self->m[index].str();

        out.size = from.size();

        out.data = new i8[out.size];
        strcpy(out.data, from.c_str());

        return out;
    }
}
