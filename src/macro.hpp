/**
 * @file macro.hpp
 * @brief Resources for managing macros
 * @author Jordan Dehmel
 * @year 2025
 */

#pragma once

#include "lexer.hpp"
#include <filesystem>
#include <map>
#include <optional>
#include <variant>

class MacroManager {
public:
  std::list<Lexer::Token>
  replace(std::list<Lexer::Token> &_whole,
          std::list<Lexer::Token>::iterator &_it,
          const std::list<Lexer::Token>::iterator &_end) const;

  /// Erases AND STRIPS QUOTES OFF OF a macro occurrence.
  static std::list<Lexer::Token>
  get_macro_args(std::list<Lexer::Token> &_whole,
                 std::list<Lexer::Token>::iterator &_it,
                 const std::list<Lexer::Token>::iterator &_end);

  void process_definition(
      std::list<Lexer::Token> &_whole,
      std::list<Lexer::Token>::iterator &_it,
      const std::list<Lexer::Token>::iterator &_end);

protected:
  struct Alias {
    std::list<Lexer::Token> contents;
  };
  struct Compiled {
    std::optional<std::filesystem::path> executable;
  };

  std::map<std::string, std::variant<Alias, Compiled>> macros;
};
