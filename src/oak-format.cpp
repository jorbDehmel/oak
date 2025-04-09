/*
An Oak auto-formatter CLI. Provides a canonical un-lexing for
token streams.

Many valid Oak files -> 1 token stream
1 token stream -> 1 valid Oak file
*/

#include "lexer.hpp"
#include "oakc.hpp"
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>

////////////////////////////////////////////////////////////////

/**
 * @brief Write the canonical formatting of the given token
 * stream to the given stream
 */
void canonicalize(std::ostream &_into,
                  const std::list<Lexer::Token> &_lexed) {
  uint col = 0;
  uint line = 1;
  uint tab_depth = 0;
  bool owed_newline = false;

  for (auto it = _lexed.cbegin(); it != _lexed.cend(); ++it) {
    std::string tok = it->text;

    if (tok == "}") {
      --tab_depth;
      if (std::next(it) != _lexed.cend() &&
          (std::next(it)->text == "else" ||
           std::next(it)->text == "case")) {
        tok += " ";
      }
    } else if (tok == ")") {
      --tab_depth;
    }

    if (col > 64) {
      owed_newline = true;
    } else if (line < it->line) {
      for (uint i = line; i < it->line; ++i) {
        if (i < line + 2) {
          _into << "\n";
        }
      }
      for (uint i = 0; i < tab_depth; ++i) {
        _into << "  ";
      }
      col = 2 * tab_depth;
      owed_newline = false;
    }
    if (owed_newline) {
      _into << "\n";
      for (uint i = 0; i < tab_depth; ++i) {
        _into << "  ";
      }
      col = 2 * tab_depth;
      owed_newline = false;
    }
    line = it->line;

    if (tok == "{") {
      if (col != 2 * tab_depth) {
        tok = " " + tok;
      }
      ++tab_depth;
      owed_newline = true;
    } else if (tok == "let" || tok == ":" || tok == "if" ||
               tok == "while" || tok == "match" ||
               tok == "case" || tok == ",") {
      if (std::next(it) != _lexed.cend() &&
          std::next(it)->line == it->line) {
        tok = tok + " ";
      }
    } else if (tok == "else" &&
               std::next(it) != _lexed.cend() &&
               std::next(it)->text == "if") {
      if (std::next(it) != _lexed.cend() &&
          std::next(it)->line == it->line) {
        tok += " ";
      }
    } else if (tok == "return" &&
               std::next(it) != _lexed.cend() &&
               std::next(it)->text != ";") {
      tok += " ";
    } else if (tok == ";") {
      owed_newline = true;
    } else if (tok == "(") {
      ++tab_depth;
    } else if (it->type == "OPERATOR" && tok != ")" &&
               tok != "::" && tok != "." && tok != "^" &&
               tok != "[" && tok != "]") {
      if (col != 2 * tab_depth) {
        tok = " " + tok;
      }
      if (std::next(it) != _lexed.cend() &&
          std::next(it)->line == it->line) {
        tok += " ";
      }
    }

    _into << tok;
    col += tok.size();
  }
  _into << '\n';
}

////////////////////////////////////////////////////////////////

/**
 * @brief Print the oak-format help text
 */
void print_help() {
  // clang-format off
  std::cout <<
    "oak-format\n"
    "Source code formatter for the Oak programming language\n"
    "\n"
    "    | Verbose   | Function\n"
    "----|-----------|----------------------------------------\n"
    " -h | --help    | Print help text (this)\n"
    " -i | --inplace | Toggle output to input file (default off)\n"
    " -v | --verify  | Verify token stream maintanance\n"
    "\n"
    "Takes 1 input file and zero or more command-line flags.\n";
  // clang-format on
  OakCompiler::print_version();
}

bool validate(const std::string &_original_text,
              const std::string &_transformed_text) {
  static Lexer lexer;
  uint64_t dummy_line = 1, dummy_col = 0;
  auto fully_lexed_input = lexer.lex(_original_text, __FILE__,
                                     dummy_line, dummy_col);
  auto fully_lexed_transformed = lexer.lex(
      _transformed_text, __FILE__, dummy_line, dummy_col);
  if (fully_lexed_transformed.size() !=
      fully_lexed_input.size()) {
    return false;
  }
  for (auto l = fully_lexed_input.cbegin(),
            r = fully_lexed_transformed.cbegin();
       l != fully_lexed_input.cend() &&
       r != fully_lexed_transformed.cbegin();
       ++l, ++r) {
    if (l->text != r->text) {
      return false;
    }
  }
  return true;
}

int main(int c, char *v[]) {
  // The file to process
  std::filesystem::path file;
  bool saw_file = false;

  // If true, OVERWRITES the original file. If false, prints to
  // std::cout.
  bool inplace = false;

  // If true, ensures that lexing the transformed text yields
  // the same token stream as the input text
  bool verify = true;

  // Parse command-line arguments
  for (int i = 1; i < c; ++i) {
    if (strncmp(v[i], "--", 2) == 0) {
      if (strcmp(v[i], "--inplace") == 0) {
        inplace = !inplace;
      } else if (strcmp(v[i], "--verify") == 0) {
        verify = !verify;
      } else if (strcmp(v[i], "--help") == 0) {
        print_help();
        return 0;
      }

      // Invalid verbose tag
      else {
        std::cerr << "Unrecognized verbose tag '" << v[i]
                  << "'\n";
        return 4;
      }
    } else if (v[i][0] == '-') {
      const uint n = strlen(v[i]);
      for (uint j = 1; j < n; ++j) {
        switch (v[i][j]) {
        case 'i':
          inplace = !inplace;
          break;
        case 'v':
          verify = !verify;
          break;
        case 'h':
          print_help();
          return 0;
        default:
          std::cerr << "Unrecognized shorthand tag '" << v[i][j]
                    << "'\n";
          return 5;
        }
      }
    } else {
      saw_file = true;
      file = v[i];
    }
  }

  // Error case: Missing file
  if (!saw_file) {
    print_help();
    std::cerr << "Please enter an Oak file!\n";
    return 6;
  }

  // Open and lex file
  Lexer lexer;
  std::string text;
  uint64_t line = 1, col = 0;

  std::ifstream input_file(file);
  if (!input_file.is_open()) {
    std::cerr << "Failed to open input file '" + file.string() +
                     "'\n";
    return 1;
  }
  text.assign(std::istreambuf_iterator<char>(input_file),
              std::istreambuf_iterator<char>());
  auto lexed = lexer.raw_lex(text, file, line, col);
  input_file.close();

  // Create canonical text
  std::stringstream transformed;
  canonicalize(transformed, lexed);

  // Optionally check
  if (verify && !validate(text, transformed.str())) {
    std::cout << transformed.str();
    std::cerr << "/* Validation failed! */\n";
    return 3;
  }

  // Output
  if (inplace) {
    std::ofstream output_file(file);
    if (!output_file.is_open()) {
      std::cerr << "Failed to open output file '" +
                       file.string() + "'\n";
      return 2;
    }
    output_file << transformed.str();
  } else {
    std::cout << transformed.str();
  }

  return 0;
}
