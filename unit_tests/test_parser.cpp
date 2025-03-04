/**
 * @file test_parser.cpp
 * @brief Tests the Oak parser
 */

#include "../src/parser.hpp"
#include <cassert>
#include <iostream>
#include <variant>

int main() {
  uint test_num = 0;
  Settings settings(std::cout);

  std::cout << "Running test #" << ++test_num << "...\n"
            << std::flush;
  { // Test 1: Functions
    Parser p;
    Lexer l;
    uint64_t line, col;
    const std::string file = "foo.oak";

    const std::string text =
        "let main() -> i32 { let var, var2: int; } "
        "let a(b: i32, c: []^bool) -> void;";

    const auto lexed = l.lex(text, file, line, col);

    p.parse_global(lexed, settings);
    p.reconstruct(std::cout);

    assert(!p.fetch_symbol("var").has_value());
    assert(!p.fetch_symbol("var2").has_value());

    { // Test main fn info
      const auto main_fn = p.fetch_symbol("main");

      assert(main_fn.has_value());
      const auto main_fn_value = main_fn.value();

      assert(std::holds_alternative<std::list<Parser::FnInfo>>(
          main_fn_value));
      const auto main_fn_info_list =
          std::get<std::list<Parser::FnInfo>>(main_fn_value);

      assert(main_fn_info_list.size() == 1);

      const auto main_fn_type = main_fn_info_list.front().t;
      assert(main_fn_type.is_fn());
      assert(main_fn_type.fn_return_type().exact_match(
          Type({"i32"})));

      const auto args = main_fn_type.fn_args();
      assert(args.empty());
    }

    { // Test "a" fn info
      const auto fn = p.fetch_symbol("a");

      assert(fn.has_value());
      const auto fn_value = fn.value();

      assert(std::holds_alternative<std::list<Parser::FnInfo>>(
          fn_value));
      const auto fn_info_list =
          std::get<std::list<Parser::FnInfo>>(fn_value);

      assert(fn_info_list.size() == 1);

      const auto fn_type = fn_info_list.front().t;
      assert(fn_type.is_fn());
      assert(
          fn_type.fn_return_type().exact_match(Type({"void"})));

      const auto args = fn_type.fn_args();
      assert(args.size() == 2);

      assert(args[0].first == "b");
      assert(args[1].first == "c");

      assert(args[0].second.exact_match(Type({"i32"})));
      assert(args[1].second.exact_match(
          Type({"[", "]", "^", "bool"})));
    }
  }

  std::cout << "Running test #" << ++test_num << "...\n"
            << std::flush;
  { // Test 2: Structs
    Parser p;
    Lexer l;
    uint64_t line, col;
    const std::string file = "fizz.oak";

    const std::string text =
        "let foo:struct{a:int,b,c:bool,}let main()->i32{let "
        "a:foo;}";

    const auto lexed = l.lex(text, file, line, col);

    p.parse_global(lexed, settings);
    p.reconstruct(std::cout);
  }

  std::cout << "Running test #" << ++test_num << "...\n"
            << std::flush;
  { // Test 3: Enums
    Parser p;
    Lexer l;
    uint64_t line, col;
    const std::string file = "fizz.oak";

    const std::string text =
        "let fizz:enum{a:int,b,c:bool,}let main()->i32{let "
        "a:fizz;}";

    const auto lexed = l.lex(text, file, line, col);

    p.parse_global(lexed, settings);
    p.reconstruct(std::cout);
  }

  std::cout << "Running test #" << ++test_num << "...\n"
            << std::flush;
  { // Test 4: Nonexistant structs
    Parser p;
    Lexer l;
    uint64_t line, col;
    const std::string file = "fizz.oak";

    const std::string text = "let main()->i32{let a:foo;}";

    const auto lexed = l.lex(text, file, line, col);

    bool did_throw = false;
    try {
      p.parse_global(lexed, settings);
    } catch (...) {
      did_throw = true;
    }
    assert(did_throw);
  }

  std::cout << "Running test #" << ++test_num << "...\n"
            << std::flush;
  { // Test 5: No-arg function calls
    Parser p;
    Lexer l;
    uint64_t line, col;
    const std::string file = "fizz.oak";

    const std::string text =
        // clang-format off
        "let fizz() -> i32 {}\n"
        "let main() -> i32\n"
        "{\n"
        "  fizz();\n"
        "}\n";
    // clang-format on

    const auto lexed = l.lex(text, file, line, col);

    p.parse_global(lexed, settings);
    p.reconstruct(std::cout);
  }

  std::cout << "Running test #" << ++test_num << "...\n"
            << std::flush;
  { // Test 6: Arg function calls
    Parser p;
    Lexer l;
    uint64_t line, col;
    const std::string file = "fizz.oak";

    const std::string text =
        // clang-format off
        "let fizz(a: i32, b: []i32) -> void {}\n"
        "let main() -> i32\n"
        "{\n"
        "  let c: i32;\n"
        "  let d: []i32;\n"
        "  fizz(c, d);\n"
        "}\n";
    // clang-format on

    const auto lexed = l.lex(text, file, line, col);

    p.parse_global(lexed, settings);
    p.reconstruct(std::cout);
  }

  std::cout << "Running test #" << ++test_num << "...\n"
            << std::flush;
  { // Test 7: Overloaded function calls
    Parser p;
    Lexer l;
    uint64_t line, col;
    const std::string file = "fizz.oak";

    const std::string text =
        // clang-format off
        "let fizz(a: i32) -> i32 {}\n"
        "let fizz(a: i64) -> i64 {}\n"
        "let main() -> i32\n"
        "{\n"
        "  let b: i32;\n"
        "  let c: i64;\n"
        "  fizz(b);\n"
        "  fizz(c);\n"
        "}\n";
    // clang-format on

    const auto lexed = l.lex(text, file, line, col);

    p.parse_global(lexed, settings);
    p.reconstruct(std::cout);
  }

  // std::cout << "Running test #" << ++test_num << "...\n"
  //           << std::flush;
  // { // Test 8: Implicit template instantiation
  //   Parser p;
  //   Lexer l;
  //   uint64_t line, col;
  //   const std::string file = "fizz.oak";

  //   const std::string text =
  //       // clang-format off
  //       "let fizz<t>(a: t) -> t {}\n"
  //       "let main() -> i32\n"
  //       "{\n"
  //       "  let b: i32;\n"
  //       "  let c: []^i32;\n"
  //       "  fizz(b);\n"
  //       "  fizz(c);\n"
  //       "}\n";
  //   // clang-format on

  //   const auto lexed = l.lex(text, file, line, col);

  //   p.parse_global(lexed);
  //   p.reconstruct(std::cout);
  // }

  std::cout << "All " << test_num
            << " parser unit tests passed!\n";

  return 0;
}
