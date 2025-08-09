#include "compiler.hpp"
#include "lexer.hpp"
#include "type.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

const std::string help_text =
    "oak2c\n"
    "Mangler for the Oak programming language\n"
    "github.com/jorbDehmel/oak\n"
    "\n"
    " Flag    | Meaning\n"
    "---------|------------------------------\n"
    " --help  | Print this text and exit\n"
    " --fancy | Print in 'fancy' mode\n"
    " --      | No more flags follow\n"
    " -       | Read input from cin over argv\n";

int main(int _c, char *_v[]) {
  std::string text;
  bool fancy = false;
  bool use_cin = false;

  for (int i = 1; i < _c; ++i) {
    if (strcmp(_v[i], "--help") == 0) {
      std::cout << help_text << '\n';
      OakCompiler::print_version();
      return 0;
    } else if (strcmp(_v[i], "--fancy") == 0) {
      fancy = !fancy;
    } else if (strcmp(_v[i], "--") == 0) {
      while (i < _c) {
        if (!text.empty()) {
          text += "\n";
        }
        text += _v[i];
        ++i;
      }
    } else if (strcmp(_v[i], "-") == 0) {
      use_cin = true;
    } else {
      if (!text.empty()) {
        text += "\n";
      }
      text += _v[i];
    }
  }

  // cin mode
  if (use_cin) {
    while (!std::cin.eof()) {
      text.push_back(std::cin.get());
    }
    text.pop_back();
  }

  // argv mode error checking
  else if (text.empty()) {
    std::cerr << "oak2c: No argv provided! Did you mean to "
                 "call w/ '-' to read from cin or '--help' "
                 "to see help text?\n";
    return 1;
  }

  // Run through lexer
  uint64_t line = 0, col = 0;
  auto lexed = Lexer::lex(text, _v[0], line, col, true);

  // Output mangled input
  if (fancy) {
    std::cout << "/*\n*/\n\n"
              << "#include \"oak/std/std_oak_header.h\"\n\n";
  }
  for (; !lexed.done(); lexed.next()) {
    if (lexed.cur().text == "let") {
      std::set<std::string> names;

      lexed.next();
      names.insert(lexed.cur());
      lexed.next();

      while (lexed.cur() == ",") {
        lexed.next();
        names.insert(lexed.cur());
        lexed.next();
      }

      if (lexed.cur() == ":") {
        lexed.next();
      }

      Type t;
      do {
        t.process_next(lexed.cur());
        lexed.next();
      } while (!t.valid());

      for (const auto &name : names) {
        std::cout << "// " << t.oak_repr(name) << "\n"
                  << t.c_repr(name);

        if (fancy) {
          std::cout << " {\n}\n\n";
        } else {
          std::cout << ";\n";
        }
      }

      lexed.prev();
    }
  }

  return 0;
}
