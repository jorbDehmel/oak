#include "parse_helpers.hpp"
#include "debug.hpp"
#include <cstring>

std::string build_generic_prefix(
    const std::string &_name,
    const TemplateInfo::Substitution &_substitutions) {
  debug_print();

  std::string prefix = _name + "_GEN";
  bool first = true;
  for (const auto &substitution : _substitutions) {
    if (first) {
      first = false;
    } else {
      prefix += "_JOIN";
    }
    for (const auto &tok : substitution) {
      prefix += "_" + tok;
    }
  }
  prefix += "_ENDGEN";
  return prefix;
}

void print_region(TokenStream &_pos, std::ostream &_where,
                  const uint &_n) {
  debug_print();
  auto start_pos = _pos.tell();

  const auto start_line = _pos.cur().line;
  const auto start_col = _pos.cur().col;

  // Go to first token BEFORE our region
  while (!_pos.at_beg() && _pos.cur().line + _n >= start_line) {
    _pos.prev();
  }

  // Go to first token OF our region
  _pos.next();

  // Top delim
  _where << "~~~~~~~~~~~~~~~~~~~~~~~~~~ In region: "
            "~~~~~~~~~~~~~~~~~~~~~~~~~~\n";

  // Print lines
  uint line = _pos.cur().line, col = 0;
  while (!_pos.done() && _pos.cur().line <= start_line) {
    // Get to correct line
    while (line < _pos.cur().line) {
      _where << '\n';
      ++line;
      col = 0;
    }

    // Get to correct column
    if (col > _pos.cur().col) {
      _where << ' ';
      col = _pos.cur().col;
    }
    while (col < _pos.cur().col) {
      _where << ' ';
      ++col;
    }

    // Print text
    _where << _pos.cur().text;

    // Advance
    col = _pos.cur().col + _pos.cur().text.size();
    _pos.next();
  }

  // Print indicator
  _where << '\n';
  for (uint i = 0; i < start_col; ++i) {
    _where << ' ';
  }
  _where << "^\n";
  _pos.seek(start_pos);
}

std::string concat(const std::list<Lexer::Token> &_what) {
  std::string out = "";
  bool first = true;
  for (const auto &tok : _what) {
    if (first) {
      first = false;
    } else {
      out += " ";
    }
    out += tok.text;
  }
  return out;
}

void check_camelcase(Settings &_warn_into,
                     const std::string &_type_str,
                     const std::string &_name,
                     const Lexer::Token &_where) noexcept {
  // The final chunk after any underscores/namespace operators
  const auto end = std::min(_name.find("_GEN"), _name.size());
  const auto pos = _name.find_last_of('_', end);
  uint i = (pos == std::string::npos ? 0 : pos);

  // Must be camelcase
  for (; i < end; ++i) {
    // A single uppercase
    if (std::isupper(_name[i])) {
      // Followed by zero or more non-uppercase
      while (i + 1 < _name.size() &&
             !std::isupper(_name[i + 1])) {
        ++i;
      }
    } else {
      // Do warning
      _warn_into.warn(_where.file, _where.line, _where.col,
                      _type_str + " name \"" + _name +
                          "\" does not seem to be camelcase");

      return;
    }
  }
}

std::string get_cmd_output(const std::string &_cmd) {
  debug_print();
  char buffer[128];
  std::string result;
  FILE *pipe = popen(_cmd.c_str(), "r");

  if (!pipe) {
    throw std::runtime_error("'popen' failed for command '" +
                             std::string(_cmd) + "'");
  }

  memset(buffer, '\0', 128);
  while (fgets(buffer, 128, pipe) != nullptr) {
    result.append(buffer, strnlen(buffer, 128));
    memset(buffer, '\0', 128);
  }

  int code = pclose(pipe) / 256;
  if (code != 0) {
    throw std::runtime_error("Command '" + std::string(_cmd) +
                             "' failed with error code " +
                             std::to_string(code));
  }

  return result;
}

std::list<Lexer::Token> TemplateInfo::replace(
    const std::list<Lexer::Token> &_to_augment,
    const std::list<std::string> &_generics,
    const std::list<std::list<std::string>> &_replacements) {
  debug_print();
  // Ensure valid substitutions
  if (_generics.size() < _replacements.size()) {
    throw std::runtime_error(
        "Too many generic substitutions provided! Expected "
        "<= " +
        std::to_string(_generics.size()) + ", but got " +
        std::to_string(_replacements.size()));
  }

  // Build substitution map
  std::map<std::string, std::list<std::string>>
      substitution_map;
  auto generic = _generics.begin();
  auto substitution = _replacements.begin();
  while (substitution != _replacements.end()) {
    substitution_map[*generic] = *substitution;
    ++generic;
    ++substitution;
  }

  // Replace
  std::list<Lexer::Token> out;
  for (const auto &t : _to_augment) {
    if (substitution_map.contains(t)) {
      for (const auto &replacement : substitution_map.at(t)) {
        out.push_back(Lexer::Token(t, replacement));
      }
    } else {
      out.push_back(t);
    }
  }
  return out;
}

std::string
Macros::strip_string_literal(const std::string &_str_lit) {
  debug_print();
  const static std::set<char> str_chars = {'\'', '"', '`'};

  // Strip \" and the likes from within
  std::string out = _str_lit;
  while (out.front() == out.back() &&
         str_chars.contains(out.front())) {
    std::string tmp;
    for (uint i = 1; i + 1 < out.size(); ++i) {
      if (i + 2 < out.size() && out[i] == '\\') {
        ++i;
      }
      tmp.push_back(out[i]);
    }
    out = tmp;
  }
  return out;
}

std::string
Macros::make_string_literal(const std::string &_contents) {
  std::string out = "\"";

  for (uint i = 0; i < _contents.size(); ++i) {
    if (_contents[i] == '"' || _contents[i] == '\\') {
      out += "\\";
    }
    out += _contents[i];
  }

  out += "\"";
  return out;
}

std::list<std::list<Lexer::Token>>
Macros::get_macro_args(TokenStream &_pos) {
  debug_print();

  uint depth = 0;
  std::list<std::list<Lexer::Token>> out;
  std::list<Lexer::Token> cur;

  do {
    _pos.next();

    if (_pos.done()) {
      break;
    } else if (_pos.cur() == "(") {
      ++depth;
      if (depth == 1) {
        continue;
      }
    } else if (_pos.cur() == ")") {
      --depth;
      if (depth == 0) {
        break;
      }
    }

    if (depth == 1 && _pos.cur() == ",") {
      if (!cur.empty()) {
        out.push_back(cur);
        cur.clear();
      }
    } else {
      cur.push_back(_pos.cur());
    }
  } while (!_pos.done());
  if (!cur.empty()) {
    out.push_back(cur);
  }
  _pos.next();

  // Leave pointing to item after call
  return out;
}
