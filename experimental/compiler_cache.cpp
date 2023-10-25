#include "compiler_cache.hpp"

/*
// File outline:

UNIX_TIMESTAMP_LL NUM_FILES
dialect_file_or_NULL
source_file
goal_file
filepath1
    num_structs
        struct_name
        num_members
            member1 member1_type
    num_enums
        enum_name
        num_members
            member1 member1_type
    num_generics
        generic_name
        generic_type_vec
        """generic_guts"""
        """generic_needs"""
    num_symbols
        symbol_name
        symbol_signature
        """symbol_c_code"""
    num_macros
        macro_name
        """macro_guts"""
num_cpp_sources
    cpp_source1
    etc
num_packages
    package1
    etc
num_rules
    rule1name
    rule1inp
    rule1out
    etc
space separated compilation flags
*/

// Save the entire state of the compiler to a file
void saveCompilerCache(const string &where)
{
    // Open file
    ofstream file(where);
    if (!file.is_open())
    {
        throw cache_error("Failed to open compiler cache file " + where);
    }

    // Write all data

    // Close file
    file.close();
    return;
}

// Load the entire state of the compiler from a file
// WARNING: Completely overwrites all runtime data in
// this translation unit.
void loadCompilerCache(const string &where)
{
    // Open file
    ifstream file(where);
    if (!file.is_open())
    {
        throw cache_error("Failed to open compiler cache file " + where);
    }

    // Read all data

    // Close file
    file.close();
    return;
}
