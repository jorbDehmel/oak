/*
DFA-powered lexer for Oak.

File structure:
```
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

#include "tags.hpp"
#include <fstream>
#include <iostream>
#include <limits.h>
#include <list>
#include <stdexcept>
#include <string>
#include <vector>

/*
States for use in the lexer DFA later on.
*/
enum LexerState
{
    delim_state = 0,
    numerical_state,
    alpha_state,
    dot_state,
    singleton_state,
    operator_state,
    string_literal_state_single,
    string_literal_state_double,
    dollar_sign_state,
    colon_state,
    dash_state,
    open_square_bracket_state,
    close_square_bracket_state,
    whitespace_state, // Must be last
};

/*
Stats about the states above. Also for use with the DFA later.
*/
const static int number_states = whitespace_state + 1;
const static int number_chars = UCHAR_MAX;

/*
A more involved token structure. Meant to be a drop-in
replacement for strings, which were the earlier token structs.
*/
class Token
{
  public:
    std::string text;

    LexerState state;
    unsigned int line, pos;
    std::string file;

    Token() : text(""), state(alpha_state), line(-1), pos(-1), file("NULL")
    {
    }

    Token(const std::string &other) : text(other), state(alpha_state), line(-1), pos(-1), file("NULL")
    {
    }

    // Miscellaneous string-equivalence operators

    inline const Token &operator=(const Token &other) noexcept
    {
        text = other.text;
        state = other.state;
        line = other.line;
        pos = other.pos;
        file = other.file;

        return other;
    }

    inline operator std::string() const noexcept
    {
        return text;
    }

    inline Token operator+(const std::string &other) const noexcept
    {
        Token out = *this;
        out.text += other;
        return out;
    }

    inline Token operator+(const char *other) const noexcept
    {
        Token out = *this;
        out.text += other;
        return out;
    }

    inline Token operator+(const Token &other) const noexcept
    {
        Token out = *this;
        out.text += other.text;
        return out;
    }

    inline const char &front() const
    {
        return text.front();
    }

    inline const char &back() const
    {
        return text.back();
    }

    inline const char &operator[](const int &i) const
    {
        return text[i];
    }

    inline bool operator==(const std::string &other) const
    {
        return text == other;
    }

    inline bool operator==(const char *const other) const
    {
        return text == other;
    }

    inline bool operator!=(const std::string &other) const
    {
        return text != other;
    }

    inline bool operator!=(const char *const other) const
    {
        return text != other;
    }

    inline size_t size() const noexcept
    {
        return text.size();
    }

    inline const char *const c_str() const noexcept
    {
        return text.c_str();
    }

    inline std::string substr(const int &start, const unsigned long long &n) const noexcept
    {
        if (start + n > text.size())
        {
            return "";
        }

        return text.substr(start, n);
    }
};

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
    ~Lexer();

    // Load from a string
    void str(const std::string &from) noexcept;

    // Load from a file
    void file(const std::string &filepath);

    // Load from a file, but still with a filepath
    void str(const std::string &from, const std::string &filepath) noexcept;

    // Load a full token stream from a given string
    std::list<Token> str_all(const std::string &from) noexcept;

    // Load a full token stream from a given string (w/ file)
    std::list<Token> str_all(const std::string &from, const std::string &filepath) noexcept;

    // Yield a single token
    Token single();

    // Returns true when exhausted
    bool done() const noexcept;

    // std::vector<Token> lex(const std::string &What, const std::string &filepath = "");
    std::list<Token> lex_list(const std::string &What, const std::string &filepath = "");

  private:
    // Text handling members
    std::string text, memory, cur_file;
    unsigned long long pos;
    int line;

    // DFA handling members
    bool is_responsible = false;
    LexerState state;

    static bool is_initialized;
    static LexerState **dfa;
};

#endif
