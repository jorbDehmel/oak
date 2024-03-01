/*
CLI tool for translating Oak to mangled C.
*/

#include "oakc_fns.hpp"
#include "oakc_structs.hpp"
#include "options.hpp"
#include <iostream>
#include <stdexcept>

void printHelp()
{
    std::cout << "oak2c: Converts Oak function signatures to C ones.\n"
              << "This is used for designing Oak/C interfacial files.\n\n"
              << "Usage:\n"
              << "oak2c --help\n"
              << "oak2c \"oak_fn_sig_here\"\n\n"
              << "Example:\n"
              << "Input:  oak2c \"let some_fn(what: structure) -> u128\"\n"
              << "Output: u128 some_fn_FN_structure_MAPS_u128(struct structure what)\n\n"
              << "Jordan Dehmel, 2024. jdehmel@outlook.com\n";
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        printHelp();
        return 1;
    }
    else if (argc > 2)
    {
        std::cout << "Only one argument is allowed: The function signature to convert.\n";
        return 2;
    }
    else if (strcmp(argv[1], "--help") == 0)
    {
        printHelp();
        return 0;
    }

    try
    {
        // Needed variables
        AcornSettings settings;
        Lexer lexer;

        // Lex the input
        auto lexed = lexer.lex_list(argv[1]);
        if (lexed.front() != "let")
        {
            std::cout << "Error: Must provide a valid Oak function signature (beginning w/ 'let').\n";
            return 3;
        }

        // Ensure that all structs referenced are seen as valid
        for (auto item : lexed)
        {
            settings.structData[item] = StructLookupData();
            settings.structData[item].members["__DUMMY"] = nullType;
        }

        // Fetch the name of the fn
        std::string name = *std::next(lexed.begin());

        // Get the type of the fn
        Type t = toType(lexed, settings);

        // Get mangled version of fn and output
        auto out = toStrCFunction(&t, settings, name);
        std::cout << out;
    }
    catch (std::runtime_error &e)
    {
        std::cout << "c::panic!(\"oak2c: Runtime error '" << e.what() << "'\");";
        return 4;
    }
    catch (...)
    {
        std::cout << "c::panic!(\"oak2c: Unknown error\");";
        return 4;
    }

    return 0;
}
