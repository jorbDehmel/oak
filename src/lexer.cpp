#include "lexer.hpp"
#include "debug.hpp"
#include <cstring>
#include <optional>
#include <set>
#include <stdexcept>

/**
 * @brief
 */
std::list<Lexer::Token>
Lexer::lex(const std::string &_text,
           const std::filesystem::path &_path, uint64_t &_line,
           uint64_t &_col) {
  debug_print();

  // Statics
  const static std::set<char> whitespace = {' ', '\t', '\n'};
  const static std::set<char> operators = {
      '~', '@', '#', '$', '%', '^', '&', '*', '-', '+',
      '=', '|', ';', ':', ',', '<', '.', '>', '/', '?'};
  const static std::set<char> singleton_operators = {
      '[', ']', '{', '}', '(', ')'};
  const static auto next_line = [&]() {
    ++_line;
    _col = 0;
  };

  std::list<Lexer::Token> out;

  for (size_t pos = 0; pos < _text.size(); ++pos, ++_col) {
    // Ignored cases
    if (whitespace.contains(_text.at(pos))) {
      // Ignore all whitespace
      if (_text.at(pos) == '\n') {
        next_line();
      }
      continue;
    } else if (_text.at(pos) == '/' && pos + 1 < _text.size() &&
               _text.at(pos + 1) == '/') {
      // Single-line comment
      while (pos + 1 < _text.size() &&
             _text.at(pos + 1) != '\n') {
        ++pos, ++_col;
      }
      if (_text.at(pos) == '\n') {
        next_line();
      }
    } else if (_text.at(pos) == '/' && pos + 1 < _text.size() &&
               _text.at(pos + 1) == '*') {
      // Multi-line comment
      while (
          pos + 1 < _text.size() &&
          !(_text.at(pos) == '*' && _text.at(pos + 1) == '/')) {
        if (_text.at(pos) == '\n') {
          next_line();
        }
        ++pos, ++_col;
      }
      ++pos, ++_col;
    }

    // Multi-character non-IDs
    else if (operators.contains(_text.at(pos))) {
      // Regular operators
      Lexer::Token to_append =
          Lexer::Token("", _path, _line, _col);
      to_append.type = "OPERATOR";
      while (pos + 1 < _text.size() &&
             operators.contains(_text.at(pos + 1))) {
        if (pos + 2 < _text.size() &&
            _text.at(pos + 1) == '/' &&
            (_text.at(pos + 2) == '/' ||
             _text.at(pos + 2) == '*')) {
          break;
        }

        to_append.text.push_back(_text.at(pos));
        ++pos, ++_col;
      }
      to_append.text.push_back(_text.at(pos));
      out.push_back(to_append);
    } else if (_text.at(pos) == '\'') {
      // Single string literal
      Token to_append = Token("", _path, _line, _col);
      to_append.type = "STRING";
      bool skip = false;
      ++pos, ++_col;
      while (pos < _text.size()) {
        if (skip) {
          skip = false;
          switch (_text.at(pos)) {
          case '\\':
          case '\'':
          case 'b':
          case 't':
          case 'n':
            to_append.text.push_back('\\');
            break;
          default:
            break;
          }
        } else if (_text.at(pos) == '\\') {
          skip = true;
          ++pos, ++_col;
          continue;
        } else if (_text.at(pos) == '\'' ||
                   _text.at(pos) == '\n') {
          break;
        } else if (_text.at(pos) == '"') {
          to_append.text.push_back('\\');
        }
        to_append.text.push_back(_text.at(pos));
        ++pos, ++_col;
      }
      to_append.text = '"' + to_append.text + '"';
      out.push_back(to_append);

    } else if (_text.at(pos) == '"') {
      // Double string literal
      Token to_append = Token("", _path, _line, _col);
      to_append.type = "STRING";
      bool skip = false;
      ++pos, ++_col;
      while (pos < _text.size()) {
        if (skip) {
          skip = false;
          switch (_text.at(pos)) {
          case '\\':
          case '"':
          case 'b':
          case 't':
          case 'n':
            to_append.text.push_back('\\');
            break;
          default:
            break;
          }
        } else if (_text.at(pos) == '\\') {
          skip = true;
          ++pos, ++_col;
          continue;
        } else if (_text.at(pos) == '"' ||
                   _text.at(pos) == '\n') {
          break;
        }
        to_append.text.push_back(_text.at(pos));
        ++pos, ++_col;
      }
      to_append.text = '"' + to_append.text + '"';
      out.push_back(to_append);

    } else if (_text.at(pos) == '`') {
      // Backtick string: Can be single or triple
      if (pos + 2 < _text.size() && _text.at(pos + 1) == '`' &&
          _text.at(pos + 2) == '`') {
        // Triple backtick string

        throw std::runtime_error(__FILE__ ":" +
                                 std::to_string(__LINE__) +
                                 "> UNIMPLEMENTED");
      } else {
        // Single backtick string
        Token to_append = Token("", _path, _line, _col);
        to_append.type = "STRING";
        bool skip = false;
        ++pos, ++_col;
        while (pos < _text.size()) {
          if (skip) {
            skip = false;
            switch (_text.at(pos)) {
            case '\\':
            case 'b':
            case 't':
            case 'n':
              to_append.text.push_back('\\');
              break;
            default:
              break;
            }
          } else if (_text.at(pos) == '\\') {
            skip = true;
            ++pos, ++_col;
            continue;
          } else if (_text.at(pos) == '`' ||
                     _text.at(pos) == '\n') {
            break;
          } else if (_text.at(pos) == '"') {
            to_append.text.push_back('\\');
          }
          to_append.text.push_back(_text.at(pos));
          ++pos, ++_col;
        }
        to_append.text = '"' + to_append.text + '"';
        out.push_back(to_append);
      }
    }

    // Singleton operators
    else if (singleton_operators.contains(_text.at(pos))) {
      out.push_back(
          Lexer::Token({_text.at(pos)}, _path, _line, _col));
      out.back().type = "OPERATOR";
    }

    // Everything else: IDs and numbers
    else {
      Lexer::Token to_append =
          Lexer::Token("", _path, _line, _col);
      while (pos + 1 < _text.size() &&
             !whitespace.contains(_text.at(pos + 1)) &&
             !operators.contains(_text.at(pos + 1)) &&
             !singleton_operators.contains(_text.at(pos + 1)) &&
             _text.at(pos + 1) != '\'' &&
             _text.at(pos + 1) != '"' &&
             _text.at(pos + 1) != '`') {
        to_append.text.push_back(_text.at(pos));
        ++pos, ++_col;
      }
      to_append.text.push_back(_text.at(pos));

      if ('0' <= to_append.text.front() &&
          to_append.text.front() <= '9') {
        to_append.type = "NUMBER";
      } else if (Type::float_literals.contains(
                     to_append.text) ||
                 Type::int_literals.contains(to_append.text) ||
                 Type::uint_literals.contains(to_append.text)) {
        to_append.type = "NUMBER";
      } else {
        to_append.type = "ID";
      }

      out.push_back(to_append);
    }
  }

  // Merge '.'s in float literals
  for (auto it = out.begin(); it != out.end(); ++it) {
    if (it->text == ".") {
      if (it != out.begin() &&
          std::prev(it)->type == "NUMBER" &&
          !std::prev(it)->text.empty() &&
          std::prev(it)->text.back() != '.') {
        std::prev(it)->text += ".";
        const auto to_erase = it;
        --it;
        out.erase(to_erase);
      } else if (std::next(it) != out.end() &&
                 std::next(it)->type == "NUMBER" &&
                 !std::next(it)->text.empty() &&
                 std::next(it)->text.front() != '.') {
        std::next(it)->text = "." + std::next(it)->text;
        const auto to_erase = it;
        --it;
        out.erase(to_erase);
      }
    }
  }

  // Merge successive literals
  for (auto it = out.begin(); it != out.end(); ++it) {
    if (it->type == "NUMBER" || it->type == "STRING") {
      while (std::next(it) != out.end() &&
             std::next(it)->type == it->type) {
        if (it->type == "STRING") {
          it->text.pop_back();
          std::next(it)->text = std::next(it)->text.substr(1);
        }
        it->text += std::next(it)->text;
        out.erase(std::next(it));
      }
    }
  }

  // Replace '::'s with '_'s
  for (auto it = out.begin(); it != out.end(); ++it) {
    while (it->type == "ID" && std::next(it) != out.end() &&
           std::next(it)->text == "::" &&
           std::next(it, 2) != out.end() &&
           std::next(it, 2)->type == "ID") {
      it->text += "_" + std::next(it, 2)->text;
      out.erase(std::next(it));
      out.erase(std::next(it));
    }
  }

  return out;
}

/**
 * @brief Returns true iff _what has the suffix _suffix, in
 * which case it is replaced.
 * @param _what The string to examine
 * @param _suffix The suffix desired
 * @param _with The item to replace the suffix with, if present
 */
const static bool replace_suffix(std::string &_what,
                                 const std::string &_suffix,
                                 const std::string &_with) {
  if (_what.size() < _suffix.size()) {
    return false;
  }

  bool has_suffix =
      (strncmp(_what.c_str() + _what.size() - _suffix.size(),
               _suffix.c_str(), _suffix.size()) == 0);

  if (has_suffix) {
    _what =
        _what.substr(0, _what.size() - _suffix.size()) + _with;
  }
  return has_suffix;
}

/**
 * @brief Gets the type of a given literal, given that it is
 * one. If not, returns nothing.
 * @param _t The possible literal to examine.
 */
std::optional<Type> Lexer::get_literal_type(Token &_t) {
  debug_print();
  if (_t.type == "STRING") {
    return Type({"^", "i8"});
  } else if (_t.text == "true" || _t.text == "false") {
    return Type({"bool"});
  }

  else if (_t.type == "NUMBER") {
    if (replace_suffix(_t.text, "u8", "")) {
      return Type({"u8"});
    } else if (replace_suffix(_t.text, "u16", "")) {
      return Type({"u16"});
    } else if (replace_suffix(_t.text, "u32", "U")) {
      return Type({"u32"});
    } else if (replace_suffix(_t.text, "u64", "UL")) {
      return Type({"u64"});
    } else if (replace_suffix(_t.text, "u128", "ULL")) {
      return Type({"u128"});
    } else if (replace_suffix(_t.text, "uint", "")) {
      return Type({"uint"});
    }

    else if (replace_suffix(_t.text, "i8", "")) {
      return Type({"i8"});
    } else if (replace_suffix(_t.text, "i16", "")) {
      return Type({"i16"});
    } else if (replace_suffix(_t.text, "i32", "")) {
      return Type({"i32"});
    } else if (replace_suffix(_t.text, "i64", "L")) {
      return Type({"i64"});
    } else if (replace_suffix(_t.text, "i128", "LL")) {
      return Type({"i128"});
    } else if (replace_suffix(_t.text, "int", "")) {
      return Type({"int"});
    }

    else if (replace_suffix(_t.text, "f32", "F")) {
      return Type({"f32"});
    } else if (replace_suffix(_t.text, "f64", "")) {
      return Type({"f64"});
    } else if (replace_suffix(_t.text, "f128", "L")) {
      return Type({"f128"});
    }

    throw std::runtime_error(
        "Untyped number literal '" + _t.text + "' at " +
        _t.file.string() + ":" + std::to_string(_t.line));
  }

  // The empty option
  return {};
}
