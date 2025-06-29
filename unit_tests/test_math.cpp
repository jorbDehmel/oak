/*
Tests fix_math in preprocessing
*/

#include "../src/oakc.hpp"
#include <cassert>

TokenStream lex(const std::string &_text) {
  static Lexer l;
  uint64_t line = 0, col = 0;
  return l.lex(_text, __FILE__, line, col);
}

bool operator==(const TokenStream &_lhs,
                const TokenStream &_rhs) {
  TokenStream l = _lhs;
  TokenStream r = _rhs;

  auto test = [&]() {
    for (l.reset(), r.reset(); !(l.done() && r.done());
         l.next(), r.next()) {
      if (l.done() || r.done()) {
        return false;
      } else if (l.cur().text != r.cur().text) {
        return false;
      }
    }
    return true;
  };

  if (test()) {
    return true;
  } else {
    std::cerr << "[";
    for (l.reset(); !l.done(); l.next()) {
      std::cerr << l.cur().text << ' ';
    }
    std::cerr << "] != [";
    for (r.reset(); !r.done(); r.next()) {
      std::cerr << r.cur().text << ' ';
    }
    std::cerr << "]\n";
    return false;
  }
}

int main() {
  OakCompiler oc;

  auto lexed = lex("1 + 2");
  oc.p.preprocess(lexed);
  assert(lexed == lex("Add(1, 2)"));

  lexed = lex("1 + 2 / 3.0 * 4");
  oc.p.preprocess(lexed);
  assert(lexed == lex("Add(1, Mult(Div(2, 3.0), 4))"));

  lexed = lex("1 + 2 / (3.0 * 4)");
  oc.p.preprocess(lexed);
  assert(lexed == lex("Add(1, Div(2, Mult(3.0, 4)))"));

  lexed = lex("a = b");
  oc.p.preprocess(lexed);
  assert(lexed == lex("Copy(a, b)"));

  lexed = lex("a = b, c");
  oc.p.preprocess(lexed);
  assert(lexed == lex("Copy(a, b), c"));

  lexed = lex("a = (b, c)");
  oc.p.preprocess(lexed);
  assert(lexed == lex("Copy(a, b, c)"));

  lexed = lex("a = (b, c + d / (e * f))");
  oc.p.preprocess(lexed);
  assert(lexed ==
         lex("Copy(a, b, Add(c, Div(d, Mult(e, f))))"));

  return 0;
}
