/**
 * @file debug.hpp
 * @brief Defines the debug_print() macro. This has no runtime
 * cost if DEBUG is not defined and prints the file and line if
 * it is defined. Use `-D DEBUG` at compile-time if you want it
 * to be used.
 * @author Jordan Dehmel
 * @year 2025
 */

#pragma once

#ifdef DEBUG
#include <iostream>
#define debug_print()                                          \
  {                                                            \
    std::cout << __FILE__ << ':' << __LINE__ << '\n'           \
              << std::flush;                                   \
  }
#else
#define debug_print() ;
#endif
