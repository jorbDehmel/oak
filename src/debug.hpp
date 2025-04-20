/**
 * @file
 * @brief Defines the debug_print() macro. This has no runtime
 * cost if DEBUG is not defined and prints the file and line if
 * it is defined. Use `-D DEBUG` at compile-time if you want it
 * to be used.
 * @author Jordan Dehmel
 */

#pragma once

#ifdef DEBUG

#include <cassert>
#include <iostream>

/**
 * @brief If debug mode is enabled, print the current file and
 * line to cout, then flushing.
 */
#define debug_print()                                          \
  {                                                            \
    std::cout << __FILE__ << ':' << __LINE__ << '\n'           \
              << std::flush;                                   \
  }

/**
 * @brief If debug mode is enabled, do some assertion.
 */
#define db_assert assert

#else

/**
 * @brief If debug mode is enabled, print the current file and
 * line to cout, then flushing.
 */
#define debug_print() ;

/**
 * @brief If debug mode is enabled, do some assertion.
 */
#define db_assert(...) ;

#endif
