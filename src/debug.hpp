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
 * @macro debug_print
 * @brief If debug mode is enabled, print the current file and
 * line to cout, then flushing.
 */
#define debug_print()                                          \
  {                                                            \
    std::cout << __FILE__ << ':' << __LINE__ << '\n'           \
              << std::flush;                                   \
  }

/**
 * @macro db_assert
 * @brief If debug mode is enabled, do some assertion.
 */
#define db_assert assert

/**
 * @macro db_rethrow
 * @brief If debug mode is enabled, rethrow the dangling
 * exception
 */
#define db_rethrow() throw

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

/**
 * @macro db_rethrow
 * @brief If debug mode is enabled, rethrow the dangling
 * exception
 */
#define db_rethrow() ;

#endif
