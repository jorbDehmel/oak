/**
 * @brief A header-only copyware ANSI color provider
 * github.com/jorbDehmel
 *
 * MIT License
 *
 * Copyright (c) 2022 - 2025 Jordan Dehmel
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall
 * be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <iostream>
#include <string>

/// ANSI colors for C++. These will only print into cout or
/// cerr, and only if Colors::allow_colors is true.
namespace Colors {

const static std::string version = "0.1.0";

/// If true, allows printing
#if (defined(_WIN32) || defined(_WIN64))
static bool allow_colors = false;
#else
static bool allow_colors = true;
#endif

/// Wrapper over a std::string that only prints if allow_colors
class ColorTag {
protected:
  /// The string to be printed to cout
  const std::string data;

  friend std::ostream &operator<<(std::ostream &_into,
                                  const ColorTag &_what);

public:
  /// Copy from some initializer
  ColorTag(const char *_data) : data(_data) {
  }
};

/// If colors are enabled and the target is std::cout or
/// std::cerr, prints the color tag. Otherwise, prints nothing.
inline std::ostream &operator<<(std::ostream &_into,
                                const ColorTag &_what) {
  if (Colors::allow_colors &&
      (_into.rdbuf() == std::cout.rdbuf() ||
       _into.rdbuf() == std::cerr.rdbuf())) {
    _into << _what.data;
  }
  return _into;
}

/// Resets to terminal defaults
const static ColorTag reset = "\033[0;0m";

/// Makes all text until `reset` red
const static ColorTag red = "\033[0;31m";

/// Makes all text until `reset` yellow
const static ColorTag yellow = "\033[0;33m";

/// Makes all text until `reset` green
const static ColorTag green = "\033[0;32m";

/// Makes all text until `reset` blue
const static ColorTag blue = "\033[0;34m";

/// Makes all text until `reset` violet
const static ColorTag violet = "\033[0;35m";

/// Makes all text until `reset` red and bold
const static ColorTag red_bold = "\033[1;31m";

/// Makes all text until `reset` yellow and bold
const static ColorTag yellow_bold = "\033[1;33m";

/// Makes all text until `reset` green and bold
const static ColorTag green_bold = "\033[1;32m";

/// Makes all text until `reset` blue and bold
const static ColorTag blue_bold = "\033[1;34m";

/// Makes all text until `reset` violet and bold
const static ColorTag violet_bold = "\033[1;35m";

/// Makes all text until `reset` italicized
const static ColorTag italics = "\033[0;3m";

/// Makes all text until `reset` bold
const static ColorTag bold = "\033[0;1m";

/// Makes all text until `reset` underscored
const static ColorTag underscore = "\033[0;4m";

/// Makes all text until `reset` struck out
const static ColorTag strikeout = "\033[0;9m";

} // namespace Colors
