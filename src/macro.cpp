/**
 * @file
 * @brief Resources for managing macros
 * @author Jordan Dehmel
 */

#include "macro.hpp"
#include "debug.hpp"
#include "lexer.hpp"
#include "oakc.hpp"
#include "settings.hpp"
#include <cctype>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>

const std::set<std::string> MacroManager::reserved_macro_names =
    {"size!",
     "type!",
     "c!",
     "alloc!",
     "free!",
     "compile_time_error!",
     "compile_time_warning!",
     "compile_time_print!",
     "str!",
     "unstr!"};

/**
 * @brief Runs a command, asserts it succeeded, and captures its
 * stdout.
 * @param _cmd The command to run
 * @returns The string output of the command
 */
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

  int code = pclose(pipe);
  if (code != 0) {
    throw std::runtime_error("Command '" + std::string(_cmd) +
                             "' failed with error code " +
                             std::to_string(code));
  }

  return result;
}

std::string MacroManager::strip_string_literal(
    const std::string &_str_lit) {
  debug_print();
  const static std::set<char> str_chars = {'\'', '"', '`'};

  std::string out = _str_lit;
  while (out.front() == out.back() &&
         str_chars.contains(out.front())) {
    char removed = out.front();
    std::string tmp;

    // Strip \" and the likes from within
    for (uint i = 1; i + 1 < out.size(); ++i) {
      if (i + 2 < out.size() && out[i] == '\\' &&
          out[i + 1] == removed) {
        ++i;
      } else {
        tmp.push_back(out[i]);
      }
    }

    out = tmp;
  }
  return out;
}

/**
 * @brief Inverse of strip_string_literal.
 * @param _contents The contents to embed in double quotes
 * @returns The string literal
 */
std::string MacroManager::make_string_literal(
    const std::string &_contents) {
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

void MacroManager::replace(
    std::list<Lexer::Token> &_whole,
    std::list<Lexer::Token>::iterator &_it,
    const std::list<Lexer::Token>::iterator &_end,
    const Settings &_csettings, OakCompiler &_oakc) const {
  debug_print();

  const auto name_tok = *_it;
  const std::string name =
      _it->text.substr(0, _it->text.find('!') + 1);
  const std::string nonexistence_replacement =
      _it->text.substr(_it->text.find('!') + 1);

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
  } else if (name == "SYSTEM!") {
    _it->type = "STRING";
#if (defined(WIN32) || defined(WINNT))
    _it->text = "\"WINDOWS\"";
#elif (defined(unix) || defined(__unix__))
    _it->text = "\"UNIX\"";
#elif (defined(__APPLE__) || defined(__MACH__))
    _it->text = "\"OSX\"";
#else
    _it->text = "\"OTHER\"";
#endif
    return;
  }

  if (!macros.contains(name)) {
    if (!nonexistence_replacement.empty()) {
      _it->text = nonexistence_replacement;
      Lexer::classify_type(*_it);
      return;
    } else {
      throw std::runtime_error("Macro '" + name +
                               "' has no definition");
    }
  }

  if (std::holds_alternative<Alias>(macros.at(name))) {
    // Inline
    const auto to_remove = _it;
    for (const auto &item :
         std::get<Alias>(macros.at(name)).contents) {
      _whole.insert(to_remove, Lexer::Token(name_tok, item));
    }
    _it = std::prev(to_remove);
    _whole.erase(to_remove);
  } else if (std::next(_it) != _end &&
             std::next(_it)->text == "(") {
    // Functional
    auto args = get_macro_args(_whole, _it, _end);
    ++_it;

    const auto exe =
        std::get<Compiled>(macros.at(name)).executable;

    if (!std::filesystem::exists(exe)) {
      throw std::runtime_error("Compiled macro " +
                               exe.string() +
                               " does not exist!");
    }

    // Prepare call
    std::string command = exe;
    for (auto &arg : args) {
      _oakc.preprocess(arg);
      std::string arg_text;
      for (const auto &tok : arg) {
        if (!arg_text.empty()) {
          arg_text.push_back(' ');
        }
        arg_text += tok;
      }
      command +=
          " " + MacroManager::make_string_literal(arg_text);
    }

    if (_csettings.debug) {
      _csettings.ostream << "Running macro call '" << command
                         << "'\n";
    }

    // Run call and get replacement
    const auto replacement = get_cmd_output(command);

    if (_csettings.debug) {
      _csettings.ostream << "Macro returned text:\n```oak\n"
                         << replacement << "\n```\n";
    }

    // Lex replacement
    Lexer l;
    uint64_t junk_line = 0, junk_col = 0;
    const auto lexed_replacement =
        l.lex(replacement, name_tok.file, junk_line, junk_col);

    // Do replacement
    for (const auto &t : lexed_replacement) {
      Lexer::Token to_insert = t;
      to_insert.file = name_tok.file;
      to_insert.line = name_tok.line;
      to_insert.col = name_tok.col;
      _whole.insert(_it, to_insert);
    }

    // Decr one
    --_it;
  } else {
    // Error
    throw std::runtime_error(
        "Compiled macro '" + name +
        "' must be invoked as a function call.");
  }
}

/// STRIPS QUOTES OFF OF a macro occurrence's
/// args. Then returns those args WITHOUT ERASURE. NO RECURSE
std::list<Lexer::Token> MacroManager::get_macro_args(
    const std::list<Lexer::Token>::const_iterator &_beg,
    const std::list<Lexer::Token>::const_iterator &_end) {
  debug_print();

  // Points to name
  uint depth = 0;
  std::list<Lexer::Token> out;
  auto it = _beg;
  Lexer::Token cur(*_beg, "");
  cur.text.clear();

  do {
    ++it;

    if (*it == "(") {
      ++depth;
      if (depth == 1) {
        continue;
      }
    } else if (*it == ")") {
      --depth;
      if (depth == 0) {
        break;
      }
    }

    if (depth == 1 && *it == ",") {
      if (!cur.text.empty()) {
        out.push_back(cur);
        cur = Lexer::Token(*it, "");
      }
    } else {
      if (!cur.text.empty()) {
        cur.text.push_back(' ');
      }
      cur.text += it->text;
    }
  } while (it != _end);
  if (!cur.text.empty()) {
    out.push_back(cur);
  }

  return out;
}

std::list<std::list<Lexer::Token>> MacroManager::get_macro_args(
    std::list<Lexer::Token> &_whole,
    std::list<Lexer::Token>::iterator &_it,
    const std::list<Lexer::Token>::iterator &_end) {
  debug_print();

  // Points to name
  const auto range_start = _it;
  uint depth = 0;

  std::list<std::list<Lexer::Token>> out;
  std::list<Lexer::Token> cur;

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
      if (!cur.empty()) {
        out.push_back(cur);
        cur.clear();
      }
    } else {
      cur.push_back(*_it);
    }
  } while (_it != _end);
  if (!cur.empty()) {
    out.push_back(cur);
  }

  ++_it;
  const auto first_after_range = _it;

  // Delete everything related to macro call, leave pointing to
  // item after call
  _whole.erase(range_start, first_after_range);
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
    // Scrape definition
    std::list<Lexer::Token> contents;
    contents.push_back(Lexer::Token(*_it, "let"));
    contents.push_back(Lexer::Token(*_it, "main"));

    // Until first "{"
    while (_it->text != "{") {
      contents.push_back(*_it);
      ++_it;

      if (_it == _end) {
        throw std::runtime_error(
            "Functional macro definition '" + name +
            "' must be followed by body");
      }
    }

    contents.push_back(*_it);
    ++_it;

    uint count = 1;
    while (count != 0) {
      if (_it->text == "{") {
        ++count;
      } else if (_it->text == "}") {
        --count;
      }

      contents.push_back(*_it);
      if (_it == _end) {
        throw std::runtime_error("Functional macro '" + name +
                                 "' has no ending curly brace");
      }

      ++_it;
    }
    --_it;

    // Write to file
    const std::filesystem::path source_path =
        _it->file.string() + "." +
        name.substr(0, name.size() - 1) + ".macro.oak";
    const std::filesystem::path executable_path =
        source_path.string() + ".out";

    // Skip compilation if possible
    bool do_compile = true;
    if (std::filesystem::exists(executable_path)) {
      const auto src_last_write =
          std::filesystem::last_write_time(_it->file);
      const auto exe_last_write =
          std::filesystem::last_write_time(executable_path);

      if (src_last_write < exe_last_write) {
        do_compile = false;
      }
    }

    if (do_compile) {
      std::ofstream f(source_path);
      if (!f.is_open()) {
        throw std::runtime_error(
            "Failed to write macro source file '" +
            source_path.string() + "'");
      }

      uint64_t cur_line = 1;

      for (const auto &item : contents) {
        if (item.line != cur_line) {
          f << '\n';
          cur_line = item.line;
        } else {
          f << ' ';
        }
        f << item.text;
      }
      f.close();

      // Compile to executable
      std::stringstream macro_compilation_log;
      OakCompiler oc(macro_compilation_log);
      oc.settings.compile_settings().do_syntax_check = false;
      oc.settings.compile_settings().mode =
          Settings::CompileSettings::TRANSLATE_COMPILE_AND_LINK;
      oc.settings.compile_settings().entry_point = source_path;
      oc.settings.compile_settings().target = executable_path;

      if (oc.settings.debug) {
        oc.settings.ostream
            << "Compiling macro '" << name << "' from "
            << source_path << " to " << executable_path << '\n';
      }

      try {
        oc();
      } catch (std::runtime_error &e) {
        std::cerr << "From macro compiler:\n"
                  << macro_compilation_log.str() << '\n';
        throw std::runtime_error(
            "During compilation of macro '" + name + "':\n" +
            e.what());
      } catch (...) {
        std::cerr << "From macro compiler:\n"
                  << macro_compilation_log.str() << '\n';
        db_rethrow();
        throw std::runtime_error(
            "Unknown error occurred during "
            "compilation of macro '" +
            name + "'");
      }
    }

    // Save executable
    Compiled c;
    c.executable = executable_path;
    macros[name] = c;
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
