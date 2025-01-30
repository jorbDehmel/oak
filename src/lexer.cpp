#include "lexer.hpp"
#include <optional>
#include <set>

/**
 * @brief
 */
std::list<Lexer::Token>
Lexer::lex(const std::string &_text,
           const std::filesystem::path &_path, uint64_t &_line,
           uint64_t &_col) {
  // Statics
  const static std::set<char> whitespace = {' ', '\t', '\n'};
  const static std::set<char> operators = {
      '~', '!', '@', '#', '$', '%', '^', '&', '*', '-', '+',
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

    // Everything else: IDs
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
      } else {
        to_append.type = "ID";
      }

      out.push_back(to_append);
    }
  }

  return out;
}
