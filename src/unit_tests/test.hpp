/*
Unit testing resources for OakC.

Jordan Dehmel, 2024
jdehmel@outlook.com
*/

#ifndef TEST_HPP
#define TEST_HPP

#include <stdexcept>

#define fakeAssert(c)                                          \
    if (!(c))                                                  \
        throw std::runtime_error("Failed assertion: " #c);

#endif
