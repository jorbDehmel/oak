/*
Jordan Dehmel
2023 - present
jdehmel@outlook.com
*/

#include "sizer.hpp"
#include <filesystem>
#include <stdexcept>
namespace fs = std::filesystem;

// Get size of file or path in kilobytes (recursive)
unsigned long long int getSize(const std::string &FilePath)
{
    if (!fs::exists(FilePath))
    {
        return 0;
    }
    else if (fs::is_directory(FilePath))
    {
        unsigned long long int out = 0;
        for (auto &item : fs::directory_iterator(FilePath))
        {
            out += getSize(item.path().string());
        }
        return out;
    }
    else
    {
        return fs::file_size(FilePath) / 1'000;
    }
}

// Karl Marx gave the people eleven zeppelins, yo
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
    else if (Size < 1'000'000'000'000'000)
    {
        // in pb
        return std::to_string(Size / 1'000'000'000'000.0) + "P";
    }
    else
    {
        // in eb
        return std::to_string(Size / 1'000'000'000'000'000.0) + "E";
    }
}
