/**
 * @file
 * @brief Tests the Oak lexer
 */

#include "../src/lexer.hpp"
#include "../src/parse_helpers.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <sys/types.h>

void assert_match(const std::string &_path,
                  const std::list<std::string> &_texts,
                  const TokenStream &_observed) {
  TokenStream r = _observed;
  bool match = true;
  auto l = _texts.begin();
  while (l != _texts.end() && match) {
    if (r.done()) {
      match = false;
      break;
    }

    match &= (r.cur().text == *l);
    match &= (r.cur().file == _path);
    ++l;
    r.next();
  }

  if (!match) {
    // Write error message
    std::cerr << "With path '" << _path << "':\n"
              << "Expected: ";

    for (const auto &item : _texts) {
      std::cerr << item << ' ';
    }
    std::cerr << "\nObserved: ";
    for (const auto &tok : r) {
      std::cerr << tok.text << ' ';
    }
    std::cerr << '\n';

    throw std::runtime_error("Failed match!");
  }
}

void assert_match(const std::string &_observed,
                  const std::string &_expected) {
  bool match = _observed == _expected;

  if (!match) {
    // Write error message
    std::cerr << "Expected: [" << _expected << "], observed ["
              << _observed << "]\n";
    throw std::runtime_error("Failed match!");
  }
}

int main() {
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

    const auto observed = lex(text, path, line, col);
    assert(line == 9);
  }

  { // Test case 1
    const std::string path = "fizz.oak";
    const std::string text =
        "include!(/*fizz buzz*/\"std/io.oak\");\n"
        "let main() -> i32//hi there\n";

    const auto observed = lex(text, path, line, col);
    assert_match(path,
                 {"include!", "(", "\"std/io.oak\"", ")", ";",
                  "let", "main", "(", ")", "->", "i32"},
                 observed);
  }

  { // Test string literal operations
    assert_match(make_string_literal("Hello, world!"),
                 "\"Hello, world!\"");
    assert_match(make_string_literal("Hello, \"world\"!"),
                 "\"Hello, \\\"world\\\"!\"");
    assert_match(
        make_string_literal("\"Hello, \\\"world\\\"!\""),
        "\"\\\"Hello, \\\\\\\"world\\\\\\\"!\\\"\"");

    assert_match(strip_string_literal("Hello, \"world\"!"),
                 "Hello, \"world\"!");
    assert_match(
        strip_string_literal(
            "\"\\\"Hello, \\\\\\\"hamburger\\\\\\\"!\\\"\""),
        "Hello, \"hamburger\"!");
  }

  { // Testing string literal operations iteratively
    std::string cur = "The \"cat\" jumps gleefully over "
                      "\"lazy \\\"dogs\\\"\"";

    for (uint i = 1; i < 8; ++i) {
      std::string next = make_string_literal(cur);
      assert_match(strip_string_literal(next), cur);
      std::cout << "Passed iteration " << i << " w/ string:\n"
                << next << "\n"
                << std::flush;
      cur = next + ", yo";
    }
  }

  std::cout << "All lexer unit tests passed!\n";

  return 0;
}
