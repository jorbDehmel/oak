/*
Jordan Dehmel
2023 - present
jdehmel@outlook.com
*/

#include "sizer.hpp"
#include <stdexcept>

unsigned long long int getSize(const std::string &FilePath)
{
#if (defined(_WIN32) || defined(_WIN64) || defined(__APPLE__))
    cout << "Cannot call the getSize function outside of Linux.\n";
    return 0;
#endif

    // Run command
    if (system(("du -s " + FilePath + " > sizer_temp.txt 2> /dev/null").c_str()) != 0)
    {
        system("rm -f sizer_temp.txt");

        // If file does not exist, 0 bytes
        return 0;
    }

    // Load file
    std::ifstream inp("sizer_temp.txt");
    if (!inp.is_open())
    {
        throw std::runtime_error("Filed to size file '" + FilePath + "'");
    }

    unsigned long long int kb;
    inp >> kb;

    inp.close();

    system("rm -f sizer_temp.txt");

    return kb;
}

std::string humanReadable(const unsigned long long int &Size)
{
    if (Size < 1'000)
    {
        // in kb
        return std::to_string(Size) + "K";
    }
    else if (Size < 1'000'000)
    {
        // in mb
        return std::to_string(Size / 1'000.0) + "M";
    }
    else if (Size < 1'000'000'000)
    {
        // in gb
        return std::to_string(Size / 1'000'000.0) + "G";
    }
    else if (Size < 1'000'000'000'000)
    {
        // in tb
        return std::to_string(Size / 1'000'000'000.0) + "T";
    }
    else
    {
        // in pb
        return std::to_string(Size / 1'000'000'000.0) + "P";
    }

    return "UNKNOWN SIZE";
}
