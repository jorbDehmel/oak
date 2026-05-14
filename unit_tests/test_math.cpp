/*
Tests fix_math in preprocessing
*/

#include "../src/compiler.hpp"
#include <cassert>

TokenStream lex(const std::string &_text) {
  uint64_t line = 0, col = 0;
  return lex(_text, __FILE__, line, col);
}

bool operator==(const TokenStream &_lhs,
                const TokenStream &_rhs) {
  TokenStream l = _lhs;
  TokenStream r = _rhs;
  const auto l_start = l.tell();
  const auto r_start = r.tell();

  auto test = [&]() {
    for (l.seek(l_start), r.seek(r_start);
         !(l.done() && r.done()); l.next(), r.next()) {
      if (l.done() || r.done()) {
        return false;
      } else if (l.cur().text != r.cur().text) {
        std::cout << "Failed on " << l.cur().text << " vs "
                  << r.cur().text << "\n";
        return false;
      }
    }
    return true;
  };

  if (test()) {
    return true;
  } else {
    std::cerr << "[";
    for (l.seek(l_start); !l.done(); l.next()) {
      std::cerr << '\'' << l.cur().text << "' ";
    }
    std::cerr << "] !=\n[";
    for (r.seek(r_start); !r.done(); r.next()) {
      std::cerr << '\'' << r.cur().text << "' ";
    }
    std::cerr << "]\n";
    return false;
  }
}

int main() {
  OakCompiler oc;

  // auto lexed = lex("1 + 2");
  // oc.p.fix_math(lexed);
  // assert(lexed == lex("Add(1, 2)"));

  // lexed = lex("1 + 2 / 3.0 * 4");
  // oc.p.fix_math(lexed);
  // assert(lexed == lex("Add(1, Mult(Div(2, 3.0), 4))"));

  // lexed = lex("1 + 2 / (3.0 * 4)");
  // oc.p.fix_math(lexed);
  // assert(lexed == lex("Add(1, Div(2, Mult(3.0, 4)))"));

  // lexed = lex("a = b");
  // oc.p.fix_math(lexed);
  // assert(lexed == lex("Copy(a, b)"));

  // lexed = lex("a = b, c");
  // oc.p.fix_math(lexed);
  // assert(lexed == lex("Copy(a, b), c"));

  // lexed = lex("a = (b, c)");
  // oc.p.fix_math(lexed);
  // assert(lexed == lex("Copy(a, b, c)"));

  // lexed = lex("a = (b, c + d / (e * f))");
  // oc.p.fix_math(lexed);
  // assert(lexed ==
  //        lex("Copy(a, b, Add(c, Div(d, Mult(e, f))))"));

  return 0;
}
