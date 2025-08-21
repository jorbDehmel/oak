/**
 * @file
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
    Settings s(std::cout);
    Parser p(s);
    uint64_t line = __LINE__, col = 0;
    const std::string file = "foo.oak";

    const std::string text =
        "let New(_: ^int) -> void; let main() -> i32 { "
        "let var, var2: int; } "
        "let a(b: i32, c: []^bool) -> void;";

    auto lexed = Lexer::lex(text, file, line, col);

    p.parse_global(lexed);
    // p.reconstruct(std::cout, settings.compile_settings());

    assert(!p.scope_manager.contains("var"));
    assert(!p.scope_manager.contains("var2"));

    { // Test main fn info
      const auto main_fn = p.scope_manager.get("main");

      assert(main_fn.has_value());
      const auto main_fn_value = main_fn.value();

      assert(std::holds_alternative<ScopeManager::FnValue>(
          main_fn_value));
      const auto main_fn_info_list =
          std::get<ScopeManager::FnValue>(main_fn_value);

      assert(main_fn_info_list.size() == 1);
      auto value = main_fn_info_list.front();

      const auto main_fn_type = value.t;
      assert(main_fn_type.is_fn());
      assert(main_fn_type.fn_return_type().exact_match(
          Type({"i32"})));

      const auto args = main_fn_type.fn_args();
      assert(args.empty());
    }

    { // Test "a" fn info
      const auto fn = p.scope_manager.get("a");

      assert(fn.has_value());
      const auto fn_value = fn.value();

      assert(std::holds_alternative<ScopeManager::FnValue>(
          fn_value));
      const auto fn_info_list =
          std::get<ScopeManager::FnValue>(fn_value);

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
    Settings s(std::cout);
    Parser p(s);
    uint64_t line = __LINE__, col = 0;
    const std::string file = "fizz.oak";

    const std::string text = "let foo:struct{a:int,b,c:bool,}"
                             "let main()->i32{let "
                             "a:foo;}";

    auto lexed = Lexer::lex(text, file, line, col);

    p.parse_global(lexed);
    // p.reconstruct(std::cout, settings.compile_settings());
  }

  std::cout << "Running test #" << ++test_num << "...\n"
            << std::flush;
  { // Test 3: Enums
    Settings s(std::cout);
    Parser p(s);
    uint64_t line = __LINE__, col;
    const std::string file = "fizz.oak";

    const std::string text = "let fizz:enum{a:int,b,c:bool,}"
                             "let main()->i32{let a: fizz;}";

    auto lexed = Lexer::lex(text, file, line, col);

    p.parse_global(lexed);
    // p.reconstruct(std::cout, settings.compile_settings());
  }

  std::cout << "Running test #" << ++test_num << "...\n"
            << std::flush;
  { // Test 4: Nonexistant structs
    Settings s(std::cout);
    Parser p(s);
    uint64_t line = __LINE__, col;
    const std::string file = "fizz.oak";

    const std::string text = "let main()->i32{let a:foo;}";

    auto lexed = Lexer::lex(text, file, line, col);

    bool did_throw = false;
    try {
      p.parse_global(lexed);
    } catch (...) {
      did_throw = true;
    }
    assert(did_throw);
  }

  std::cout << "Running test #" << ++test_num << "...\n"
            << std::flush;
  { // Test 5: No-arg function calls
    Settings s(std::cout);
    Parser p(s);
    uint64_t line = __LINE__, col;
    const std::string file = "fizz.oak";

    const std::string text =
        // clang-format off
        "let fizz() -> i32 {}\n"
        "let main() -> i32 {\n"
        "  fizz();\n"
        "}\n";
    // clang-format on

    auto lexed = Lexer::lex(text, file, line, col);

    p.parse_global(lexed);
    // p.reconstruct(std::cout, settings.compile_settings());
  }

  std::cout << "Running test #" << ++test_num << "...\n"
            << std::flush;
  { // Test 6: Arg function calls
    Settings s(std::cout);
    Parser p(s);
    uint64_t line = __LINE__, col;
    const std::string file = "fizz.oak";

    const std::string text =
        // clang-format off
        "let fizz(a: i32, b: []i32) -> void {}\n"
        "let New(_: ^i32) -> void;\n"
        "let main() -> i32\n"
        "{\n"
        "  let c: i32;\n"
        "  let d: []i32;\n"
        "  fizz(c, d);\n"
        "}\n";
    // clang-format on

    auto lexed = Lexer::lex(text, file, line, col);

    p.parse_global(lexed);
    // p.reconstruct(std::cout, settings.compile_settings());
  }

  std::cout << "Running test #" << ++test_num << "...\n"
            << std::flush;
  { // Test 7: Overloaded function calls
    Settings s(std::cout);
    Parser p(s);
    uint64_t line = __LINE__, col;
    const std::string file = "fizz.oak";

    const std::string text =
        // clang-format off
        "let fizz(a: i32) -> i32 {}\n"
        "let fizz(a: i64) -> i64 {}\n"
        "let New(_: ^i32) -> void {}\n"
        "let New(_: ^i64) -> void {}\n"
        "let main() -> i32 {\n"
        "  let b: i32;\n"
        "  let c: i64;\n"
        "  fizz(b);\n"
        "  fizz(c);\n"
        "}\n";
    // clang-format on

    auto lexed = Lexer::lex(text, file, line, col);

    p.parse_global(lexed);
    // p.reconstruct(std::cout, settings.compile_settings());
  }

  std::cout << "All " << test_num
            << " parser unit tests passed!\n";

  return 0;
}
