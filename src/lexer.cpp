#include "lexer.hpp"
#include "debug.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <optional>
#include <stdexcept>

/**
 * @brief Avoid C keywords
 * @param _raw The symbol name to KWA-mangle
 * @return The KWA-mangled symbol name (usually the same as inp)
 */
inline std::string
kwa_mangle(const std::string &_raw) noexcept {
  // Reserved symbols by C, with any that are also reserved Oak
  // removed
  const static std::set<std::string> reserved = {
      "alignas",
      "alignof",
      "auto",
      // "bool",
      "break",
      // "case",
      "char",
      "const",
      "constexpr",
      "continue",
      "default",
      "do",
      "double",
      // "else",
      // "enum",
      "extern",
      // "false",
      "float",
      "for",
      "goto",
      // "if",
      "inline",
      // "int",
      "long",
      "nullptr",
      "register",
      "restrict",
      // "return",
      "short",
      "signed",
      "sizeof",
      "static",
      "static_assert",
      // "struct",
      "switch",
      "thread_local",
      // "true",
      "typedef",
      "typeof",
      "typeof_unqual",
      "union",
      "unsigned",
      // "void",
      "volatile",
      // "while",
      "_Alignas",
      "_Alignof",
      "_Atomic",
      "_BitInt",
      "_Bool",
      "_Complex",
      "_Decimal128",
      "_Decimal32",
      "_Decimal64",
      "_Generic",
      "_Imaginary",
      "_Noreturn",
      "_Static_assert",
      "_Thread_local",
  };
  return (reserved.contains(_raw) ? _raw + "_KWA" : _raw);
}

const std::set<char> Lexer::whitespace = {' ', '\t', '\n'};
const std::set<char> Lexer::operators = {
    '~', '@', '$', '%', '^', '&', '*', '-',
    '+', '=', '|', ':', '.', '/', '?', '!'};
const std::set<char> Lexer::singleton_operators = {
    '[', ']', '{', '}', '(', ')', ',', ';', '<', '>'};

std::list<Lexer::Token>
Lexer::raw_lex(const std::string &_text,
               const std::filesystem::path &_path,
               uint64_t &_line, uint64_t &_col,
               const bool &_is_original) {
  debug_print();

  const auto next_line = [&]() {
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
    } else if (_text.at(pos) == '#' ||
               (_text.at(pos) == '/' &&
                pos + 1 < _text.size() &&
                _text.at(pos + 1) == '/')) {
      // Single-line comment
      auto start_pos = pos;
      while (pos + 1 < _text.size() &&
             _text.at(pos + 1) != '\n') {
        ++pos, ++_col;
      }
      out.push_back(Lexer::Token(
          _text.substr(start_pos, pos - start_pos + 1), _path,
          _line, _col));
      if (_text.at(pos) == '\n') {
        next_line();
      }
    } else if (_text.at(pos) == '/' && pos + 1 < _text.size() &&
               _text.at(pos + 1) == '*') {
      // Multi-line comment
      auto start_pos = pos;
      while (
          pos + 1 < _text.size() &&
          !(_text.at(pos) == '*' && _text.at(pos + 1) == '/')) {
        if (_text.at(pos) == '\n') {
          out.push_back(Lexer::Token(
              _text.substr(start_pos, pos - start_pos), _path,
              _line, _col));
          out.back().type = "COMMENT";
          start_pos = pos + 1;
          next_line();
        }
        ++pos, ++_col;
      }
      ++pos, ++_col;
      out.push_back(Lexer::Token(
          _text.substr(start_pos, pos - start_pos + 1), _path,
          _line, _col));
      out.back().type = "COMMENT";
    }

    // Multi-character non-IDs
    else if (operators.contains(_text.at(pos)) &&
             !(_text.at(pos) == '-' && pos + 1 < _text.size() &&
               '0' <= _text.at(pos + 1) &&
               _text.at(pos + 1) <= '9')) {

      // Regular operators
      std::string to_append;
      while (pos + 1 < _text.size() &&
             operators.contains(_text.at(pos + 1))) {
        if (pos + 2 < _text.size() &&
            _text.at(pos + 1) == '/' &&
            (_text.at(pos + 2) == '/' ||
             _text.at(pos + 2) == '*')) {
          break;
        }

        to_append.push_back(_text.at(pos));
        ++pos, ++_col;
      }
      to_append.push_back(_text.at(pos));
      out.push_back(
          Lexer::Token(to_append, _path, _line, _col));
    } else if (_text.at(pos) == '\'') {
      // Single string literal
      std::string to_append;
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
          case '0':
            to_append.push_back('\\');
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
          to_append.push_back('\\');
        }
        to_append.push_back(_text.at(pos));
        ++pos, ++_col;
      }
      out.push_back(Lexer::Token('"' + to_append + '"', _path,
                                 _line, _col));
    } else if (_text.at(pos) == '"') {
      // Double string literal
      std::string to_append;
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
          case '0':
            to_append.push_back('\\');
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
        to_append.push_back(_text.at(pos));
        ++pos, ++_col;
      }
      out.push_back(Lexer::Token('"' + to_append + '"', _path,
                                 _line, _col));
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
        std::string to_append;
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
            case '0':
              to_append.push_back('\\');
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
            to_append.push_back('\\');
          }
          to_append.push_back(_text.at(pos));
          ++pos, ++_col;
        }
        out.push_back(Lexer::Token('"' + to_append + '"', _path,
                                   _line, _col));
      }
    }

    // Singleton operators
    else if (singleton_operators.contains(_text.at(pos))) {
      out.push_back(
          Lexer::Token({_text.at(pos)}, _path, _line, _col));
    }

    // Everything else: IDs and numbers
    else {
      std::string to_append;
      while (pos + 1 < _text.size() &&
             !whitespace.contains(_text.at(pos + 1)) &&
             (!operators.contains(_text.at(pos + 1)) ||
              _text.at(pos + 1) == '!') &&
             !singleton_operators.contains(_text.at(pos + 1)) &&
             _text.at(pos + 1) != '\'' &&
             _text.at(pos + 1) != '"' &&
             _text.at(pos + 1) != '`') {
        to_append.push_back(_text.at(pos));
        ++pos, ++_col;
      }
      to_append.push_back(_text.at(pos));
      out.push_back(
          Lexer::Token(to_append, _path, _line, _col));
    }
  } // End main loop

  // Merge <=, >=, and -> operators
  for (auto it = out.begin(); std::next(it) != out.end();
       ++it) {
    if (it->text == "-" && std::next(it)->text == ">") {
      it->text = "->";
      out.erase(std::next(it));
    } else if (it->text == "<" && std::next(it)->text == "=") {
      it->text = "<=";
      out.erase(std::next(it));
    } else if (it->text == ">" && std::next(it)->text == "=") {
      it->text = ">=";
      out.erase(std::next(it));
    }
  }

  // Avoid C keywords, fix columns, and mark originality
  for (auto it = out.begin(); it != out.end(); ++it) {
    it->col -= it->text.size();
    it->text = kwa_mangle(it->text);
    it->original = _is_original;
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

  // Distinguish between less-than/greater-than and templates
  for (auto it = out.begin(); it != out.end(); ++it) {
    if (it->text == "<") {
      // Look ahead: If ">" occurs before ";", ")", "]", "}", it
      // is templating.
      for (auto t = std::next(it); t != out.end(); ++t) {
        if (*t == ">") {
          // Templating
          t->type = it->type = "TEMPLATE";
        } else if (*t == ";" || *t == ")" || *t == "]" ||
                   *t == "}") {
          // Not templating
          break;
        }
      }
    }
  }

  return out;
}

class TokenStream Lexer::lex(const std::string &_text,
                             const std::filesystem::path &_path,
                             uint64_t &_line, uint64_t &_col,
                             const bool &_is_original) {
  auto out = raw_lex(_text, _path, _line, _col, _is_original);

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

  // Remove comments
  std::erase_if(out, [](const Lexer::Token tok) -> bool {
    return tok.type == "COMMENT";
  });

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

  return TokenStream(out);
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
  debug_print();
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
std::optional<Type> Lexer::get_literal_type(Lexer::Token &_t) {
  debug_print();
  if (_t.type == "STRING") {
    _t.text = "((i8 *)" + _t.text + ")";
    return Type({"[", "]", "i8"});
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

void Lexer::classify_type(Lexer::Token &_t) {
  if (_t.type == "COMMENT") {
    ;
  } else if (_t.text.empty() || _t.text.starts_with("//") ||
             _t.text.starts_with("/*") ||
             _t.text.starts_with("#")) {
    _t.type = "COMMENT";
  } else if (_t.text.front() == '"') {
    _t.type = "STRING";
  } else if (operators.contains(_t.text.at(0)) &&
             !(_t.text.at(0) == '-' && 1 < _t.text.size() &&
               '0' <= _t.text.at(1) && _t.text.at(1) <= '9')) {
    _t.type = "OPERATOR";
  } else if (singleton_operators.contains(_t.text.front())) {
    _t.type = "OPERATOR";
  } else if (('0' <= _t.text.front() &&
              _t.text.front() <= '9') ||
             (_t.text.size() > 1 && _t.text.front() == '-' &&
              '0' <= _t.text[1] && _t.text[1] <= '9')) {
    _t.type = "NUMBER";
  } else if (Type::float_literals.contains(_t.text) ||
             Type::int_literals.contains(_t.text) ||
             Type::uint_literals.contains(_t.text)) {
    _t.type = "NUMBER";
  } else if (_t.text == "EOF" && _t.file == "N/A" &&
             _t.line == 0 && _t.col == 0) {
    _t.type = "EOF";
  } else {
    _t.type = "ID";
  }
}

TokenStream Lexer::tokify(const std::list<std::string> &_what,
                          const std::filesystem::path &_where,
                          const uint64_t &_line,
                          const uint64_t &_col) {
  std::list<Lexer::Token> out;
  for (const auto &item : _what) {
    out.push_back(Lexer::Token(item, _where, _line, _col));
    Lexer::classify_type(out.back());
  }
  return out;
}

void TokenStream::next() noexcept {
  if (!done()) {
    ++cur_pos;
  }
}

void TokenStream::prev() noexcept {
  if (cur_pos != raw_stream.begin()) {
    --cur_pos;
  }
}

bool TokenStream::done() const noexcept {
  return cur_pos == raw_stream.cend();
}

const Lexer::Token TokenStream::cur() const noexcept {
  if (done()) {
    return Lexer::Token("EOF", "N/A", 0, 0);
  } else {
    return *cur_pos;
  }
}

std::list<Lexer::Token>::iterator TokenStream::tell() noexcept {
  return cur_pos;
}

void TokenStream::seek(
    const std::list<Lexer::Token>::iterator &_where) noexcept {
  cur_pos = _where;
}

std::list<Lexer::Token>::iterator TokenStream::erase(
    const std::list<Lexer::Token>::iterator &_end) {
  return raw_stream.erase(_end);
}

void TokenStream::erase(
    const std::list<Lexer::Token>::iterator &_begin,
    const std::list<Lexer::Token>::iterator &_end) {
  raw_stream.erase(_begin, _end);
}

void TokenStream::replace(
    const std::list<Lexer::Token>::iterator &_begin,
    const std::list<Lexer::Token>::iterator &_end,
    const TokenStream &_with) {
  raw_stream.erase(_begin, _end);
  raw_stream.insert(_end, _with.raw_stream.begin(),
                    _with.raw_stream.end());
}

Lexer::Token TokenStream::peek(const uint &_n) const noexcept {
  const auto out = std::next(cur_pos, _n);
  if (out == raw_stream.end()) {
    return Lexer::Token("EOF", "N/A", 0, 0);
  } else {
    return *out;
  }
}

bool TokenStream::at_beg() const noexcept {
  return cur_pos == raw_stream.begin();
}

void TokenStream::insert(
    const std::list<Lexer::Token>::iterator &_end,
    const TokenStream &_with) {
  raw_stream.insert(_end, _with.raw_stream.begin(),
                    _with.raw_stream.end());
}

void TokenStream::insert(
    const std::list<Lexer::Token>::iterator &_end,
    const Lexer::Token &_what) {
  raw_stream.insert(_end, _what);
}
