/**
 * @file macro.hpp
 * @brief Resources for managing macros
 * @author Jordan Dehmel
 * @year 2025
 */

#include "macro.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include "oakc.hpp"
#include <set>
#include <stdexcept>
#include <string>
#include <variant>

void MacroManager::replace(
    std::list<Lexer::Token> &_whole,
    std::list<Lexer::Token>::iterator &_it,
    const std::list<Lexer::Token>::iterator &_end) const {
  debug_print();

  const auto name = *_it;

  if (name == "LINE!") {
    _it->type = "NUMBER";
    _it->text = std::to_string(_it->line) + "u64";
    return;
  } else if (name == "COL!") {
    _it->type = "NUMBER";
    _it->text = std::to_string(_it->col) + "u64";
    return;
  } else if (name == "FILE!") {
    _it->type = "STRING";
    _it->text = '"' + _it->file.string() + '"';
    return;
  } else if (name == "oak_VERSION!") {
    _it->type = "STRING";
    _it->text = '"' + ACORN_VERSION + '"';
    return;
  }

  if (!macros.contains(name.text)) {
    throw std::runtime_error("Macro '" + name.text +
                             "' has no definition");
  }

  if (std::holds_alternative<Alias>(macros.at(name))) {
    // Inline
    const auto to_remove = _it;
    for (const auto &item :
         std::get<Alias>(macros.at(name)).contents) {
      _whole.insert(to_remove, Lexer::Token(name, item.text));
    }
    _it = std::prev(to_remove);
    _whole.erase(to_remove);
  } else if (std::next(_it) != _end &&
             std::next(_it)->text == "(") {
    // Functional
    throw std::runtime_error(
        "Functional macro replacement is unimplemented");
  } else {
    // Error
    throw std::runtime_error(
        "Compiled macro '" + name.text +
        "' must be invoked as a function call.");
  }
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

// Points to macro name after 'let'. Can be inline or
// functional. Erases all traces after done
void MacroManager::process_definition(
    std::list<Lexer::Token> &_whole,
    std::list<Lexer::Token>::iterator &_it,
    const std::list<Lexer::Token>::iterator &_end) {
  debug_print();

  // let
  const auto range_start = std::prev(_it);

  // name!
  const auto name = _it->text;

  ++_it;

  // Either '=' or '('
  if (_it->text == "=") {
    // Scan until ;
    Alias info;

    ++_it;
    while (_it != _end && *_it != ";") {
      info.contents.push_back(*_it);
      ++_it;
    }

    macros[name] = info;
  } else if (_it->text == "(") {
    throw std::runtime_error(
        "Functional macro definition is unimplemented");
  } else {
    throw std::runtime_error(
        "Malformed macro definition for " + name +
        ": Expected '=' or '(', but saw '" + _it->text + "'");
  }

  // Erase range
  ++_it;
  const auto first_after_range = _it;
  _whole.erase(range_start, first_after_range);
}
