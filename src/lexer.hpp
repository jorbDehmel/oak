/*
DFA-powered lexer for Oak.

File structure:
```
token.hpp
   |
   v
lexer.hpp   <---
   |
   v
oakc_structs.hpp
   |
   v
options.hpp
   |
   v
oakc_fns.hpp
   |
   v
<compiler frontend>
```

Jordan Dehmel, 2024
jdehmel@outlook.com
*/

#ifndef LEXER_HPP
#define LEXER_HPP

#include "token.hpp"
#include <cstdint>
#include <limits.h>
#include <list>
#include <string>

/*
Erase all in-line comments (beginning with '// ') and multi-line
comments (such as this one) from an Oak token stream. This is
in-place.
*/
void erase_comments(std::list<Token> &what);

/*
Erase all whitespace tokens from an Oak token stream. This is
specifically anything tagged by the lexer as whitespace. This
is spaces, tabs, and newlines (as well as any conglomerations of
these).
*/
void erase_whitespace(std::list<Token> &what);

/*
Takes a text, yields a token stream. Uses a global static DFA,
which is compiled and cleaned up by the first instance. Thus,
this instance should live as long as the compiler to avoid
unnecessary re-initializations.
*/
class Lexer
{
  public:
    // If this is the first instance, compile (and delete) DFA
    Lexer();

    // Load from a string
    void str(const std::string &from) noexcept;

    // Load from a file
    void file(const std::string &filepath);

    // Load from a file, but still with a filepath
    void str(const std::string &from,
             const std::string &filepath) noexcept;

    // Load a full token stream from a given string
    std::list<Token> str_all(const std::string &from) noexcept;

    // Load a full token stream from a given string (w/ file)
    std::list<Token> str_all(
        const std::string &from,
        const std::string &filepath) noexcept;

    // Yield a single token
    Token single();

    // Returns true when exhausted
    bool done() const noexcept;

    // std::vector<Token> lex(const std::string &What, const
    // std::string &filepath = "");
    std::list<Token> lex_list(const std::string &What,
                              const std::string &filepath = "");

  private:
    // Text handling members
    std::string text, cur_file;
    uint64_t line, col, pos;
};

#endif
