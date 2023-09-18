#include "sizer.hpp"

unsigned long long int getSize(const string &FilePath)
{
#if (defined(_WIN32) || defined(_WIN64) || defined(__APPLE__))
    cout << "Cannot call the getSize function outside of Linux.\n";
    return 0;
#endif

    // Run command
    if (system(("du -s " + FilePath + " > sizer_temp.txt").c_str()) != 0)
    {
        throw_assert(system("rm -f sizer_temp.txt") == 0);

        // If file does not exist, 0 bytes
        return 0;
    }

    // Load file
    ifstream inp("sizer_temp.txt");
    throw_assert(inp.is_open());

    unsigned long long int kb;
    inp >> kb;

    inp.close();
    throw_assert(system("rm -f sizer_temp.txt") == 0);

    return kb;
}

string humanReadable(const unsigned long long int &Size)
{
    string out;

    if (Size < 1'000)
    {
        // in kb
        out = to_string(Size) + "K";
    }
    else if (Size < 1'000'000)
    {
        // in mb
        out = to_string(Size / 1'000.0) + "M";
    }
    else if (Size < 1'000'000'000)
    {
        // in gb
        out = to_string(Size / 1'000'000.0) + "G";
    }
    else if (Size < 1'000'000'000'000)
    {
        // in tb
        out = to_string(Size / 1'000'000'000.0) + "T";
    }
    else
    {
        // in pb
        out = to_string(Size / 1'000'000'000.0) + "P";
    }

    // Effectively cannot measure larger

    return out;
}
