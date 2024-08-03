/*
Implementations for oakc / Acorn's DFA lexer.

Jordan Dehmel, 2024
jdehmel@outlook.com
*/

#include "lexer.hpp"
#include "tags.hpp"
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <stdexcept>

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
    passthrough_state,
    whitespace_state, // Must be last
};

/*
Stats about the states above. Also for use with the DFA later.
*/
const static int number_states = whitespace_state + 1;
const static int number_chars = UCHAR_MAX;

// DFA stuff for lexing.
namespace Lex
{
static bool is_initialized = false;
static LexerState dfa[number_states][number_chars];

// Not all of these will actually be used
const static std::map<LexerState, std::string> state_to_type = {
    {numerical_state, "NUMBER"},
    {alpha_state, "ID"},
    {dot_state, "DOT"},
    {singleton_state, "SINGLETON"},
    {operator_state, "OP"},
    {string_literal_state_single, "SINGLE_STR"},
    {string_literal_state_double, "DOUBLE_STR"},
    {dollar_sign_state, "DOLLAR_SIGN"},
    {colon_state, "COLON"},
    {dash_state, "DASH"},
    {open_square_bracket_state, "OPEN_SQUARE"},
    {close_square_bracket_state, "CLOSE_SQUARE"},
    {passthrough_state, "PASSTHROUGH"},
    {whitespace_state, "WHITESPACE"},
};

void init_dfa();
} // namespace Lex

// Initialize DFA
void Lex::init_dfa()
{
    const static char *alpha = "abcdefghijklmnopqrstuvwxyz_"
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ`";
    const static char *numer = "0123456789";
    const static char *oper = "!%&*=+|/~[]";
    const static char *singletons = "^@#(){};,?";
    const static char *whitespace = " \t\n";

    for (int i = 0; i < number_states; ++i)
    {
        // Set default to deliminator state
        for (int j = 0; j < number_chars; j++)
        {
            dfa[i][j] = delim_state;
        }
    }

    // Alphabetic stuff
    for (const char *i = &alpha[0]; *i != '\0'; ++i)
    {
        dfa[delim_state][(unsigned int)*i] = alpha_state;
        dfa[numerical_state][(unsigned int)*i] =
            numerical_state;
        dfa[alpha_state][(unsigned int)*i] = alpha_state;
        dfa[string_literal_state_single][(unsigned int)*i] =
            string_literal_state_single;
        dfa[string_literal_state_double][(unsigned int)*i] =
            string_literal_state_double;
        dfa[whitespace_state][(unsigned int)*i] = delim_state;
    }

    // Non-ascii alphabetic stuff
    for (int i = 1; i < number_chars; ++i)
    {
        if (i > 31 && i < 128)
        {
            continue;
        }
        else if (i == '\t' || i == '\n')
        {
            continue;
        }

        dfa[delim_state][i] = alpha_state;
        dfa[numerical_state][i] = numerical_state;
        dfa[alpha_state][i] = alpha_state;
        dfa[string_literal_state_single][i] =
            string_literal_state_single;
        dfa[string_literal_state_double][i] =
            string_literal_state_double;
        dfa[whitespace_state][i] = delim_state;
    }

    // Numerical stuff
    for (const char *i = &numer[0]; *i != '\0'; ++i)
    {
        dfa[delim_state][(unsigned int)*i] = numerical_state;
        dfa[numerical_state][(unsigned int)*i] =
            numerical_state;
        dfa[alpha_state][(unsigned int)*i] = alpha_state;
        dfa[dot_state][(unsigned int)*i] = numerical_state;
        dfa[string_literal_state_single][(unsigned int)*i] =
            string_literal_state_single;
        dfa[string_literal_state_double][(unsigned int)*i] =
            string_literal_state_double;
        dfa[dash_state][(unsigned int)*i] = numerical_state;
    }

    // Dot stuff
    dfa[delim_state][(unsigned int)'.'] = dot_state;
    dfa[numerical_state][(unsigned int)'.'] = numerical_state;
    dfa[string_literal_state_single][(unsigned int)'.'] =
        string_literal_state_single;
    dfa[string_literal_state_double][(unsigned int)'.'] =
        string_literal_state_double;

    // Singleton stuff
    for (const char *i = &singletons[0]; *i != '\0'; ++i)
    {
        dfa[delim_state][(unsigned int)*i] = singleton_state;
    }

    // Square bracket stuff
    dfa[delim_state][(unsigned int)'<'] =
        open_square_bracket_state;
    dfa[delim_state][(unsigned int)'>'] =
        close_square_bracket_state;
    dfa[open_square_bracket_state][(unsigned int)'='] =
        operator_state;
    dfa[close_square_bracket_state][(unsigned int)'='] =
        operator_state;

    // Dash stuff
    dfa[delim_state][(unsigned int)'-'] = dash_state;
    dfa[dash_state][(unsigned int)'-'] = dash_state;
    dfa[dash_state][(unsigned int)'>'] = operator_state;

    // Operator stuff
    for (const char *i = &oper[0]; *i != '\0'; ++i)
    {
        dfa[delim_state][(unsigned int)*i] = operator_state;
        dfa[operator_state][(unsigned int)*i] = operator_state;
        dfa[string_literal_state_single][(unsigned int)*i] =
            string_literal_state_single;
        dfa[string_literal_state_double][(unsigned int)*i] =
            string_literal_state_double;
        dfa[dash_state][(unsigned int)*i] = operator_state;
    }
    dfa[operator_state][(unsigned int)'['] = delim_state;

    // String literal stuff
    for (int i = 0; i < number_chars; ++i)
    {
        dfa[string_literal_state_single][i] =
            string_literal_state_single;
        dfa[string_literal_state_double][i] =
            string_literal_state_double;
    }

    dfa[delim_state][(unsigned int)'\''] =
        string_literal_state_single;
    dfa[delim_state][(unsigned int)'"'] =
        string_literal_state_double;

    dfa[delim_state][(unsigned int)':'] = colon_state;

    for (int i = 1; i < number_states; ++i)
    {
        if (i == singleton_state)
        {
            continue;
        }

        if (i != string_literal_state_double)
        {
            dfa[(LexerState)i][(unsigned int)'\''] =
                delim_state;
        }
        if (i != string_literal_state_single)
        {
            dfa[(LexerState)i][(unsigned int)'"'] = delim_state;
        }
    }

    // Whitespace stuff
    for (const char *i = &whitespace[0]; *i != '\0'; ++i)
    {
        dfa[delim_state][(unsigned int)*i] = whitespace_state;
        dfa[string_literal_state_single][(unsigned int)*i] =
            string_literal_state_single;
        dfa[string_literal_state_double][(unsigned int)*i] =
            string_literal_state_double;
        dfa[whitespace_state][(unsigned int)*i] = delim_state;
    }

    dfa[string_literal_state_single][(unsigned int)'\n'] =
        delim_state;
    dfa[string_literal_state_double][(unsigned int)'\n'] =
        delim_state;

    // Misc
    dfa[alpha_state][(unsigned int)'!'] = alpha_state;

    for (int i = 0; i < number_states; ++i)
    {
        if (i == string_literal_state_single ||
            i == string_literal_state_double ||
            i == whitespace_state || i == singleton_state)
        {
            continue;
        }

        dfa[i][(unsigned int)'$'] = dollar_sign_state;
    }
    for (int i = 0; i < number_chars; ++i)
    {
        dfa[dollar_sign_state][i] = dollar_sign_state;
        dfa[colon_state][i] = delim_state;
        dfa[singleton_state][i] = delim_state;
        dfa[passthrough_state][i] = delim_state;
    }

    dfa[dollar_sign_state][(unsigned int)' '] = delim_state;
    dfa[dollar_sign_state][(unsigned int)'\t'] = delim_state;
    dfa[dollar_sign_state][(unsigned int)'\n'] = delim_state;
    dfa[colon_state][(unsigned int)':'] = colon_state;

    dfa[string_literal_state_single][(unsigned int)'\''] =
        passthrough_state;
    dfa[string_literal_state_double][(unsigned int)'"'] =
        passthrough_state;

    Lex::is_initialized = true;
}

////////////////////////////////////////////////////////////////

std::string to_string(Token &what)
{
    if (what.text == " ")
    {
        return "\\s";
    }
    else if (what.text == "\n")
    {
        return "\\n";
    }
    else if (what.text == "\t")
    {
        return "\\t";
    }
    else
    {
        return what.text;
    }
}

Lexer::Lexer()
{
    // Initialize variables
    cur_file = "NULL";
    text = "\n";
    pos = col = 0;
    line = 1;

    // Initialize DFA if need be
    if (!Lex::is_initialized)
    {
        Lex::init_dfa();
        return;
    }
}

void Lexer::str(const std::string &from) noexcept
{
    pos = col = 0;
    line = 1;
    text = from;
    text.push_back('\n');
}

void Lexer::file(const std::string &filepath)
{
    pos = col = 0;
    line = 1;
    cur_file = filepath;

    std::string line;
    std::ifstream f(filepath, std::ios::ate);

    if (!f.is_open())
    {
        throw std::runtime_error("Failed to open file '" +
                                 filepath + "'");
    }

    text.clear();
    text.reserve(f.tellg());
    f.seekg(std::ios::beg);

    while (getline(f, line))
    {
        text.append(line + '\n');
    }

    f.close();
}

Token Lexer::single()
{
    if (done())
    {
        throw std::runtime_error("Cannot lex finished text.");
    }

    Token out;
    out.text.reserve(16); // This is an arbitrary number
    out.text.clear();
    out.col = col;
    out.line = line;
    out.file = cur_file;

    uint64_t starting_pos = pos;
    bool skip = false;
    LexerState state = delim_state, prev_state = delim_state;

    do
    {
        if (Lex::dfa[state]['\n'] != delim_state &&
            text[pos] == '\n')
        {
            ++line;
            col = 0;
        }
        out.text.push_back(text[pos]);

        if (skip)
        {
            skip = false;
        }
        else if (text[pos] == '\\')
        {
            skip = true;
        }
        else
        {
            if (state != passthrough_state)
            {
                prev_state = state;
            }
            state = Lex::dfa[state][(unsigned char)text[pos]];
        }

        ++col;
        ++pos;
    } while (state != delim_state && pos < text.size());

    // Correct off-by-one error
    out.text.pop_back();
    if (pos < text.size())
    {
        --pos;
        --col;
    }

    out.type = (Lex::state_to_type.count(prev_state) != 0)
                   ? Lex::state_to_type.at(prev_state)
                   : "NULL";

    // Error checking
    if (pos <= starting_pos)
    {
        int start, end;
        const auto r = 20;

        start = std::max(0ll, (long long)(col - r));
        end = std::min((long long)text.size(),
                       (long long)(col + r));

        std::string sub = text.substr(start, end - start);
        for (int i = 0; i < sub.size(); ++i)
        {
            if (sub[i] == '\n')
            {
                sub[i] = ' ';
            }
        }

        std::cout << tags::violet_bold << "In region:\n"
                  << sub << "\n";
        for (int i = start; i < col; ++i)
        {
            std::cout << ' ';
        }
        std::cout << "^\n\n" << tags::reset;

        throw std::runtime_error(
            "Invalid lex: Token cannot be of size " +
            std::to_string(pos - starting_pos));
    }

    return out;
}

bool Lexer::done() const noexcept
{
    return pos >= text.size();
}

void erase_comments(std::list<Token> &what)
{
    int count = 0;
    for (auto it = what.begin(); it != what.end(); it++)
    {
        if (it->text == "/*")
        {
            count++;
        }
        else if (it->text == "*/")
        {
            count--;
        }

        if (count != 0 || (count == 0 && it->text == "*/"))
        {
            it = what.erase(it);
            it--;
        }
        else
        {
            if (it->substr(0, 2) == "//" ||
                it->substr(0, 1) == "#")
            {
                while (it != what.end() && it->text != "\n")
                {
                    it = what.erase(it);
                }
                it--;
            }
        }
    }
}

void erase_whitespace(std::list<Token> &what)
{
    for (auto it = what.begin(); it != what.end(); it++)
    {
        if (it->type == "WHITESPACE")
        {
            it = what.erase(it);
            it--;
        }
    }
}

void join_numbers(std::list<Token> &what)
{
    const static std::set<std::string> type_suffixes = {
        "u8",  "u16", "u32", "u64",  "u128", "i8",
        "i16", "i32", "i64", "i128", "f32",  "f64"};

    for (auto it = what.begin(); it != what.end(); it++)
    {
        if (it->type == "NUMBER")
        {
            // Concatenate number literals
            ++it;
            while (it != what.end() && it->type == "NUMBER")
            {
                std::string temp = it->text;
                --it;
                it->text += temp;
                ++it;

                it = what.erase(it);
            }

            // Join numerical type-suffixes
            if (it != what.end() &&
                type_suffixes.count(it->text) != 0)
            {
                std::string temp = it->text;
                --it;
                it->text += temp;
                ++it;

                it = what.erase(it);
            }

            else
            {
                --it;
                // Check for type suffixes

                bool is_typed = false;
                for (const char c : it->text)
                {
                    if (isalpha(c))
                    {
                        is_typed = true;
                        break;
                    }
                }

                if (!is_typed)
                {
                    std::cout << tags::yellow_bold
                              << "Warning: Untyped number "
                              << "literal '" << it->text
                              << "' at " << it->file << ":"
                              << it->line << "." << it->col
                              << ".\n"
                              << tags::reset;
                }

                ++it;
            }

            --it;
        }
    }
}

/*
Applies the namespace rule, where '::' becomes '_'.
*/
void join_namespaces(std::list<Token> &what)
{
    for (auto it = what.begin(); it != what.end(); it++)
    {
        if (it != what.begin() && *it == "::")
        {
            it = what.erase(it);

            if (it != what.end())
            {
                std::string temp = it->text;
                it = what.erase(it);
                it--;
                it->text += "_" + temp;
            }
        }
    }
}

/*
Joins successive string literals together. Assumes that
whitespace is no longer present.
*/
void join_strings(std::list<Token> &what)
{
    // Single quote pass
    for (auto it = what.begin(); it != what.end(); it++)
    {
        if (it->type == "SINGLE_STR")
        {
            it++;

            while (it->type == "SINGLE_STR")
            {
                std::string temp = it->text.substr(1);
                it--;
                it->text.pop_back();
                it->text += temp;
                it++;

                it = what.erase(it);
            }

            it--;
        }
    }

    // Double quote pass
    for (auto it = what.begin(); it != what.end(); it++)
    {
        if (it->type == "DOUBLE_STR")
        {
            it++;

            while (it->type == "DOUBLE_STR")
            {
                std::string temp = it->text.substr(1);
                it--;
                it->text.pop_back();
                it->text += temp;
                it++;

                it = what.erase(it);
            }

            it--;
        }
    }
}

void join_bitshifts(std::list<Token> &what)
{
    for (auto it = what.begin(); it != what.end(); it++)
    {
        auto next = it;
        next++;

        if (next == what.end())
        {
            break;
        }

        // Possibly merge into a left bitshift
        if (*it == "<")
        {
            /*
            If a ) or ; occurs before the next >, do sub.
            Otherwise, is templating; Advance i past all of
            this.
            */

            bool isTemplating = false;
            int depth = 1;
            auto j = it;
            for (; j != what.end(); j++)
            {
                if (*j == ";" || *j == ")")
                {
                    isTemplating = false;
                    break;
                }
                else if (*j == ">")
                {
                    depth--;

                    if (depth == 0)
                    {
                        isTemplating = true;
                        break;
                    }
                }
                else if (*j == "<")
                {
                    depth++;
                }
            }

            if (isTemplating)
            {
                // Move past a valid template
                it = j;
            }
            else if (*next == "<")
            {
                // Merge into a left bit shift
                next->text = "<<";
                next->type = "OP";

                it = what.erase(it);
            }
        }

        // The above code will skip past any valid templating.
        // Therefore, no need to peek ahead for right bitshifts.

        else if (*it == ">" && *next == ">")
        {
            // Merge into a right bit shift
            next->text = ">>";
            next->type = "OP";

            it = what.erase(it);
        }
    }
}

void Lexer::str(const std::string &from,
                const std::string &filepath) noexcept
{
    pos = col = 0;
    line = 1;
    text = from;
    cur_file = filepath;

    text.push_back('\n');
}

std::list<Token> Lexer::str_all(
    const std::string &from) noexcept
{
    str(from, cur_file);

    std::list<Token> out;

    while (!done())
    {
        out.push_back(single());
    }

    return out;
}

std::list<Token> Lexer::str_all(
    const std::string &from,
    const std::string &filepath) noexcept
{
    str(from, filepath);

    std::list<Token> out;

    while (!done())
    {
        out.push_back(single());
    }

    return out;
}

std::list<Token> Lexer::lex_list(const std::string &What,
                                 const std::string &filepath)
{
    if (filepath != "")
    {
        cur_file = filepath;
    }

    str(What);

    std::list<Token> out;

    while (!done())
    {
        out.push_back(single());
    }

    erase_comments(out);
    erase_whitespace(out);
    join_namespaces(out);

    join_numbers(out);
    join_bitshifts(out);
    join_strings(out);

    return out;
}
