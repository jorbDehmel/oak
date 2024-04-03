/*
Unit testing resources for OakC.

Jordan Dehmel, 2024
jdehmel@outlook.com
*/

#ifndef TEST_HPP
#define TEST_HPP

#include "../oakc_fns.hpp"
#include <cassert>
#include <stdexcept>

#define STR_INTERNAL(INP) #INP
#define STR(INP) STR_INTERNAL(INP)

static void assertEqual(const std::list<Token> &A, const std::list<Token> &B)
{
    assert(A.size() == B.size());

    auto a = A.cbegin();
    auto b = B.cbegin();

    bool isEqual = true;
    while (a != A.cend() && b != B.cend())
    {
        if (a->text != b->text)
        {
            isEqual = false;
            break;
        }

        ++a;
        ++b;
    }

    if (!isEqual)
    {
        for (const auto &item : A)
        {
            std::cerr << item.text << ' ';
        }
        std::cerr << "\nIs not equal to\n";
        for (const auto &item : B)
        {
            std::cerr << item.text << ' ';
        }
        std::cerr << '\n';

        throw std::runtime_error(__FUNCTION__ STR(__LINE__));
    }
}

#endif
