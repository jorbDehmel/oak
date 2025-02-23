/**
 * @file macro.hpp
 * @brief Resources for managing macros
 * @author Jordan Dehmel
 * @year 2025
 */

#include "macro.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include <set>
#include <stdexcept>

std::list<Lexer::Token> MacroManager::replace(
    std::list<Lexer::Token> &_whole,
    std::list<Lexer::Token>::iterator &_it,
    const std::list<Lexer::Token>::iterator &_end) const {
  debug_print();
  throw std::runtime_error(__FUNCTION__);
}

std::list<Lexer::Token> MacroManager::get_macro_args(
    std::list<Lexer::Token> &_whole,
    std::list<Lexer::Token>::iterator &_it,
    const std::list<Lexer::Token>::iterator &_end) {
  debug_print();

  // Points to name
  const auto range_start = _it;
  uint depth = 0;
  std::list<Lexer::Token> out;
  Lexer::Token cur(*_it, "");
  cur.text.clear();

  do {
    ++_it;

    if (*_it == "(") {
      ++depth;
      if (depth == 1) {
        continue;
      }
    } else if (*_it == ")") {
      --depth;
      if (depth == 0) {
        break;
      }
    }

    if (depth == 1 && *_it == ",") {
      if (!cur.text.empty()) {
        out.push_back(cur);
        cur = Lexer::Token(*_it, "");
      }
    } else {
      if (!cur.text.empty()) {
        cur.text.push_back(' ');
      }
      cur.text += _it->text;
    }
  } while (_it != _end);
  if (!cur.text.empty()) {
    out.push_back(cur);
  }

  ++_it;
  const auto first_after_range = _it;

  // Delete everything related to macro call, leave pointing to
  // item after call
  _whole.erase(range_start, first_after_range);

  // Clean quotes
  const static std::set<char> str_chars = {'\'', '"', '`'};
  for (auto it = out.begin(); it != out.end(); ++it) {
    while (it->text.front() == it->text.back() &&
           str_chars.contains(it->text.front())) {
      it->text = it->text.substr(1, it->text.size() - 2);
    }
  }

  return out;
}

void MacroManager::process_definition(
    std::list<Lexer::Token> &_whole,
    std::list<Lexer::Token>::iterator &_it,
    const std::list<Lexer::Token>::iterator &_end) {
  debug_print();
  throw std::runtime_error(__FUNCTION__);
}
