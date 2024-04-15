/*
Unit testing resources for OakC.

Jordan Dehmel, 2024
jdehmel@outlook.com
*/

#ifndef TEST_HPP
#define TEST_HPP

#include "../oakc_fns.hpp"
#include <stdexcept>
#include <string>

#define fakeAssert(c)                                                                                                  \
    if (!(c))                                                                                                          \
        throw std::runtime_error("Failed assertion: " #c);

static void assertEqual(const std::list<Token> &obs, const std::list<Token> &exp)
{
    bool isEqual = true;

    if (obs.size() != exp.size())
    {
        isEqual = false;
    }
    else
    {
        auto a = obs.cbegin();
        auto b = exp.cbegin();

        while (a != obs.cend() && b != exp.cend())
        {
            if (a->text != b->text)
            {
                isEqual = false;
                break;
            }

            ++a;
            ++b;
        }
    }

    if (!isEqual)
    {
        std::cerr << "Observed:\n";
        for (const auto &item : obs)
        {
            std::cerr << item.text << ' ';
        }
        std::cerr << "\nIs not equal to expected:\n";
        for (const auto &item : exp)
        {
            std::cerr << item.text << ' ';
        }
        std::cerr << '\n';

        throw std::runtime_error(__FUNCTION__ + std::string(":") + std::to_string(__LINE__));
    }
}

#endif
