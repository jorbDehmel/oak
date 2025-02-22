/**
 * @file test_lexer.cpp
 * @brief Tests the Oak lexer
 */

#include "../src/lexer.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>

void assert_match(const std::string &_path,
                  const std::list<std::string> &_texts,
                  const std::list<Lexer::Token> &_observed) {
  bool match = true;

  match &= (_texts.size() == _observed.size());

  auto l = _texts.begin();
  auto r = _observed.begin();
  while (l != _texts.end() && match) {
    match &= (r->text == *l);
    match &= (r->file == _path);
    ++l, ++r;
  }

  if (!match) {
    // Write error message
    std::cerr << "With path '" << _path << "':\n"
              << "Expected: ";

    for (const auto &item : _texts) {
      std::cerr << item << ' ';
    }
    std::cerr << "\nObserved: ";
    for (const auto &item : _observed) {
      std::cerr << item.text << ' ';
    }
    std::cerr << '\n';

    throw std::runtime_error("Failed match!");
  }
}

int main() {
  Lexer l;
  uintmax_t line = 1, col = 0;

  { // Test case 1
    const std::string path = "foo.oak";
    const std::string text =
        "package!(\"std\");\n"
        "use_rule!(\"std\");\n"
        "include!(/*fizz buzz*/\"std/io.oak\");\n"
        "let main() -> i32//hi there\n"
        "{\n"
        "    print(\"Hello, world!\");\n"
        "    0\n"
        "}\n";

    const auto observed = l.lex(text, path, line, col);
    assert(line == 9);
  }

  { // Test case 1
    const std::string path = "fizz.oak";
    const std::string text =
        "include!(/*fizz buzz*/\"std/io.oak\");\n"
        "let main() -> i32//hi there\n";

    const auto observed = l.lex(text, path, line, col);
    assert_match(path,
                 {"include!", "(", "\"std/io.oak\"", ")", ";",
                  "let", "main", "(", ")", "->", "i32"},
                 observed);
  }

  std::cout << "All lexer unit tests passed!\n";

  return 0;
}
