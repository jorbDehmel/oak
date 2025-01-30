#include "lexer.hpp"
#include "oakc.hpp"
#include "type.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

int main(int _c, char *_v[]) {
  std::string text;

  if (_c == 2 && strcmp(_v[1], "--help") == 0) {
    // clang-format off
    std::cout
        << "oak2c\n"
           "Mangler for the Oak programming language\n"
           "github.com/jorbDehmel/oak\n"
           "\n"
           " Command        | Purpose\n"
           "----------------|--------------------------------------------\n"
           " `oak2c --help` | Prints this text and exits\n"
           " `oak2c -`      | Reads input from cin over args\n"
           " `oak2c ...`    | Lex, mangle, and output args\n"
           "\n";
    // clang-format on
    OakCompiler::print_version();

    return 0;
  }

  // cin mode
  if (_c == 2 && strcmp(_v[1], "-") == 0) {
    while (!std::cin.eof()) {
      text.push_back(std::cin.get());
    }
    text.pop_back();
  }

  // argv mode
  else {
    if (_c == 1) {
      std::cerr << "oak2c: No argv provided! Did you mean to "
                   "call w/ '-' to read from cin or '--help' "
                   "to see help text?\n";
      return 1;
    }

    for (int i = 1; i < _c; ++i) {
      text += _v[i];
      text.push_back('\n');
    }
  }

  // Run through lexer
  Lexer l;
  uint64_t line, col;
  const auto lexed = l.lex(text, _v[0], line, col);

  // Output mangled input
  for (auto it = lexed.begin(); it != lexed.end(); ++it) {
    if (it->text == "let") {
      ++it;
      const auto name = it->text;
      ++it;
      if (*it == ":") {
        ++it;
      }

      Type t;
      do {
        t.process_next(it->text);
        ++it;
      } while (!t.valid());

      std::cout << "// " << t.oak_repr(name) << "\n"
                << t.c_repr(name) << ";\n";

      --it;
    }
  }

  return 0;
}
