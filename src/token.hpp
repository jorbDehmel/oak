/*
Token class for Oak lexing.

File structure:
```
token.hpp   <---
   |
   v
lexer.hpp
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

#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <cstdint>
#include <string>

/*
A more involved token structure. Meant to be a drop-in
replacement for strings, which were the earlier token structs.
*/
class Token
{
  public:
    std::string text, type;
    uint64_t line, col;
    std::string file;

    Token()
        : text(""), type("NULL"), line(0), col(0), file("NULL")
    {
    }

    Token(const std::string &other)
        : text(other), type("NULL"), line(0), col(0),
          file("NULL")
    {
    }

    // Miscellaneous string-equivalence operators

    inline const Token &operator=(const Token &other) noexcept
    {
        text = other.text;
        line = other.line;
        file = other.file;
        type = other.type;

        return other;
    }

    inline operator std::string() const noexcept
    {
        return text;
    }

    inline Token operator+(
        const std::string &other) const noexcept
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

    inline std::string substr(
        const int &start,
        const unsigned long long &n) const noexcept
    {
        if (start + n > text.size())
        {
            return "";
        }

        return text.substr(start, n);
    }
};

#endif
