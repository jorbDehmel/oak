#include "lexer.hpp"
#include <cctype>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <variant>
#include <vector>

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

inline bool isnum(const std::string &_s,
                  const bool &_or_type = false) {
  if (_s.empty()) {
    return false;
  } else if ('0' <= _s.front() && _s.front() <= '9') {
    return true;
  } else if (_or_type && Type::is_built_in_type(_s)) {
    return true;
  }
  return false;
}

std::list<Lexer::Token>
Lexer::raw_lex(const std::string &_text,
               const std::filesystem::path &_path,
               uint64_t &_line, uint64_t &_col,
               const bool &_is_original) {
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
  }

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

  // Avoid C keywords and fix columns
  for (auto it = out.begin(); it != out.end(); ++it) {
    it->col -= it->text.size();
    it->text = kwa_mangle(it->text);
  }

  // Merge '.'s in float literals
  for (auto it = out.begin(); it != out.end(); ++it) {
    if (it->text == ".") {
      if (it != out.begin() && !std::prev(it)->text.empty() &&
          std::prev(it)->text.back() != '.') {
        std::prev(it)->text += ".";
        const auto to_erase = it;
        --it;
        out.erase(to_erase);
      } else if (std::next(it) != out.end() &&
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
    if (isnum(it->text)) {
      while (std::next(it) != out.end() &&
             isnum(std::next(it)->text, true)) {
        it->text += std::next(it)->text;
        out.erase(std::next(it));
      }
    } else if (it->text.starts_with('"')) {
      while (std::next(it) != out.end() &&
             std::next(it)->text.starts_with('"')) {
        it->text.pop_back();
        std::next(it)->text = std::next(it)->text.substr(1);
        it->text += std::next(it)->text;
        out.erase(std::next(it));
      }
    }
  }

  // Replace '::'s with '_'s
  for (auto it = out.begin(); it != out.end(); ++it) {
    while (std::next(it) != out.end() &&
           std::next(it)->text == "::" &&
           std::next(it, 2) != out.end()) {
      it->text += "_" + std::next(it, 2)->text;
      out.erase(std::next(it));
      out.erase(std::next(it));
    }
  }

  // For good measure
  for (auto it = out.begin(); it != out.end(); ++it) {
    while (it->text == "::" && std::next(it) != out.end()) {
      it->text = "_" + std::next(it)->text;
      out.erase(std::next(it));
    }
  }

  return out;
}

TokenStream Lexer::lex(const std::string &_text,
                       const std::filesystem::path &_path,
                       uint64_t &_line, uint64_t &_col,
                       const bool &_is_original) {
  return fix_math(TokenStream(
      raw_lex(_text, _path, _line, _col, _is_original)));
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
std::optional<Type> Lexer::get_literal_type(Lexer::Token &_t) {
  if (_t.text.starts_with('"')) {
    _t.text = "((i8 *)" + _t.text + ")";
    return Type(ASTNode("[]", {ASTNode("i8")}));
  } else if (_t.text == "true" || _t.text == "false") {
    return Type({"bool"});
  }

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

  // The empty option
  return {};
}

void TokenStream::next() noexcept {
  if (!done()) {
    ++cur_pos;
  }
}

void TokenStream::prev() noexcept {
  if (cur_pos != 0) {
    --cur_pos;
  }
}

bool TokenStream::done() const noexcept {
  return cur_pos >= raw_stream.size() ||
         raw_stream.at(cur_pos) == "EOF";
}

const Lexer::Token TokenStream::cur() const {
  if (done()) {
    throw std::runtime_error(
        "Cannot get current token past EOF");
  } else {
    return raw_stream.at(cur_pos);
  }
}

Lexer::Token &TokenStream::cur_mut() {
  if (done()) {
    throw std::runtime_error(
        "Cannot set token on overrun token stream");
  } else {
    return raw_stream.at(cur_pos);
  }
}

size_t TokenStream::tell() noexcept {
  return cur_pos;
}

TokenStream TokenStream::copy_snippet(
    const size_t &_start,
    const size_t &_first_after) const noexcept {
  std::list<Lexer::Token> out;
  for (size_t i = _start;
       i < _first_after && i < raw_stream.size(); ++i) {
    out.push_back(raw_stream.at(i));
  }
  return out;
}

void TokenStream::rangef(
    const size_t &_start, const size_t &_first_after,
    const std::list<std::variant<Lexer::Token, TokenStream>>
        &_fmt) {
  if (_start >= _first_after || _start >= raw_stream.size() ||
      _first_after > raw_stream.size()) {
    throw std::runtime_error("Invalid range given to rangef");
  }

  // Construct copy
  std::vector<Lexer::Token> copy;
  for (const auto &item : _fmt) {
    if (std::holds_alternative<Lexer::Token>(item)) {
      copy.push_back(std::get<Lexer::Token>(item));
    } else {
      bool fix_fields = (!copy.empty());

      for (const auto &tok : std::get<TokenStream>(item)) {
        Lexer::Token t = tok;

        // Fix line and col if desired
        if (fix_fields) {
          t.line = copy.back().line;
          t.col = copy.back().col;
          t.file = copy.back().file;
        }

        copy.push_back(t);
      }
    }
  }

  // Replace range
  if (copy.size() > _first_after - _start) {
    // Will need some insertion
    // Note: This could be done much more efficiently
    for (size_t i = 0;
         i < (_first_after - _start) - copy.size(); ++i) {
      raw_stream.insert(raw_stream.begin() + _first_after,
                        Lexer::Token("", "", 0, 0));
    }
  } else if (copy.size() < _first_after - _start) {
    // Will need some deletion
    raw_stream.erase(raw_stream.begin() + _start + copy.size(),
                     raw_stream.begin() + _first_after);
  }

  // Exact range match
  for (size_t i = 0; i < copy.size(); ++i) {
    raw_stream[_start + i] = copy[i];
  }

  seek(_start + copy.size());
}

void TokenStream::seek(const size_t &_where) noexcept {
  cur_pos = _where;
}

Lexer::Token TokenStream::peek(const int &_n) const {
  if (cur_pos + _n < 0 || cur_pos + _n >= raw_stream.size()) {
    throw std::runtime_error("Cannot peek past EOF");
  } else {
    return raw_stream.at(cur_pos + _n);
  }
}

bool TokenStream::at_beg() const noexcept {
  return cur_pos == 0;
}

/// An AST but maintaining token-hood
struct TokenAST {
  Lexer::Token text;
  std::vector<TokenAST> children;
};

std::ostream &operator<<(std::ostream &_into,
                         const TokenAST &_what) {
  _into << '(' << _what.text.text;
  for (const auto &child : _what.children) {
    _into << ' ' << child;
  }
  _into << ')';
  return _into;
}

TokenStream fix_math(const TokenStream &_ts) {
  std::list<TokenAST> stream;

  // Member access
  for (const auto &t : _ts) {
    if (stream.size() >= 2 && stream.back().text == ".") {
      const auto obs_op = stream.back();
      stream.pop_back(); // Get rid of operator
      const auto lhs = stream.back();
      stream.pop_back();
      const auto rhs = TokenAST(t);
      stream.push_back(TokenAST(Lexer::Token(obs_op.text, "_"),
                                {lhs, obs_op, rhs}));
    } else {
      stream.push_back(TokenAST(t));
    }
  }

  std::list<TokenAST> next_stream;

  // Prefix unaries (!, ~, ++, --)
  const std::list<std::pair<std::string, std::string>>
      prefix_unaries = {
          {"~", "Flip"},
          {"++", "Incr"},
          {"--", "Decr"},
          {"!", "Not"},
      };
  for (const auto &p : prefix_unaries) {
    const auto op = p.first;
    const auto fn = p.second;
    for (const auto &t : stream) {
      if (!next_stream.empty() &&
          next_stream.back().text == op) {
        const auto obs_op = next_stream.back();
        next_stream.pop_back();
        const auto operand = t;
        next_stream.push_back(
            TokenAST(Lexer::Token(obs_op.text, fn), {operand}));
      } else {
        next_stream.push_back(t);
      }
    }
    stream = next_stream;
    next_stream.clear();
  }

  // Infix binaries (looks gnarly, but is colinear-time)
  const std::list<
      std::list<std::pair<std::string, std::string>>>
      infix_binaries = {
          {{"*", "Mult"}, {"/", "Div"}, {"%", "Mod"}},
          {{"+", "Add"}, {"-", "Sub"}},
          {{"<", "Less"},
           {">", "Great"},
           {"<=", "Leq"},
           {">=", "Greq"},
           {"==", "Eq"},
           {"!=", "Neq"}},
          {{"&&", "Andd"}, {"||", "Orr"}},
          {{"=", "Copy"},
           {"+=", "PlusEq"},
           {"-=", "SubEq"},
           {"*=", "MultEq"},
           {"/=", "DivEq"},
           {"%=", "ModEq"}},
      };
  for (const auto &precedence_level : infix_binaries) {
    for (const auto &t : stream) {
      bool did_op = false;
      for (const auto &p : precedence_level) {
        const auto op = p.first;
        const auto fn = p.second;

        if (next_stream.size() >= 2 &&
            next_stream.back().text == op) {
          const auto obs_op = next_stream.back();
          next_stream.pop_back(); // Get rid of operator
          const auto lhs = next_stream.back();
          next_stream.pop_back();

          // Special case: Inline macros and decl-inst combos
          if (op == "=" &&
              next_stream.back().text.text == "let") {
            // Replace what we took off
            next_stream.push_back(lhs);
            next_stream.push_back(obs_op);
            continue;
          }

          const auto rhs = t;
          did_op = true;
          next_stream.push_back(TokenAST(
              Lexer::Token(obs_op.text, fn), {lhs, rhs}));
          break;
        }
      }

      if (!did_op) {
        next_stream.push_back(t);
      }
    }

    stream = next_stream;
    next_stream.clear();
  }

  // Flatten trees
  std::list<Lexer::Token> tokens;
  std::function<void(const TokenAST &)> flatten =
      [&](const TokenAST &_t) {
        if (_t.text != "_" || _t.children.empty()) {
          tokens.push_back(_t.text);
        }
        if (!_t.children.empty()) {
          tokens.push_back(Lexer::Token(tokens.back(), "("));
          bool first = true;
          for (const auto &arg : _t.children) {
            if (first) {
              first = false;
            } else {
              tokens.push_back(
                  Lexer::Token(tokens.back(), ","));
            }
            flatten(arg);
          }
          tokens.push_back(Lexer::Token(tokens.back(), ")"));
        }
      };
  for (const auto &item : stream) {
    flatten(item);
  }

  return TokenStream(tokens);
}
