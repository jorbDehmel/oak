/*
Tests fix_math in preprocessing
*/

#include "../src/oakc.hpp"
#include <cassert>

std::list<Lexer::Token> lex(const std::string &_text) {
  static Lexer l;
  uint64_t line = 0, col = 0;
  return l.lex(_text, __FILE__, line, col);
}

bool operator==(const std::list<Lexer::Token> &_lhs,
                const std::list<Lexer::Token> &_rhs) {
  auto test = [&]() {
    if (_lhs.size() != _rhs.size()) {
      return false;
    }
    for (auto l = _lhs.begin(), r = _rhs.begin();
         l != _lhs.end() && r != _rhs.end(); ++l, ++r) {
      if (l->text != r->text) {
        return false;
      }
    }
    return true;
  };

  if (test()) {
    return true;
  } else {
    std::cerr << "[";
    for (const auto &t : _lhs) {
      std::cerr << t.text << ' ';
    }
    std::cerr << "] != [";
    for (const auto &t : _rhs) {
      std::cerr << t.text << ' ';
    }
    std::cerr << "]\n";
    return false;
  }
}

int main() {
  OakCompiler oc;

  auto lexed = lex("1 + 2");
  oc.preprocess(lexed);
  assert(lexed == lex("Add(1, 2)"));

  lexed = lex("1 + 2 / 3.0 * 4");
  oc.preprocess(lexed);
  assert(lexed == lex("Add(1, Mult(Div(2, 3.0), 4))"));

  lexed = lex("1 + 2 / (3.0 * 4)");
  oc.preprocess(lexed);
  assert(lexed == lex("Add(1, Div(2, Mult(3.0, 4)))"));

  lexed = lex("a = b");
  oc.preprocess(lexed);
  assert(lexed == lex("Copy(a, b)"));

  lexed = lex("a = b, c");
  oc.preprocess(lexed);
  assert(lexed == lex("Copy(a, b), c"));

  lexed = lex("a = (b, c)");
  oc.preprocess(lexed);
  assert(lexed == lex("Copy(a, b, c)"));

  lexed = lex("a = (b, c + d / (e * f))");
  oc.preprocess(lexed);
  assert(lexed ==
         lex("Copy(a, b, Add(c, Div(d, Mult(e, f))))"));

  return 0;
}
