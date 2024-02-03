/*
Implementations for oakc / Acorn's DFA lexer.

Jordan Dehmel, 2024
jdehmel@outlook.com
*/

#include "lexer.hpp"

bool Lexer::is_initialized = false;
LexerState **Lexer::dfa = nullptr;

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
    const char *alpha = "abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ`";
    const char *numer = "0123456789";
    const char *oper = "!%&*=+|/~[]";
    const char *singletons = "^@#(){};,?";
    const char *whitespace = " \t\n";

    cur_file = "NULL";

    // Localize variables
    text = "";
    pos = 0;
    line = 1;
    state = delim_state;

    // Initialize singleton members
    if (Lexer::is_initialized)
    {
        is_responsible = false;
        return;
    }

    is_responsible = true;
    Lexer::is_initialized = true;

    // Initialize DFA
    dfa = new LexerState *[number_states];
    for (int i = 0; i < number_states; i++)
    {
        dfa[i] = new LexerState[number_chars];

        // Set default to deliminator state
        for (int j = 0; j < number_chars; j++)
        {
            dfa[i][j] = delim_state;
        }
    }

    // Alphabetic stuff
    for (const char *i = &alpha[0]; *i != '\0'; i++)
    {
        dfa[delim_state][(unsigned int)*i] = alpha_state;
        dfa[numerical_state][(unsigned int)*i] = numerical_state;
        dfa[alpha_state][(unsigned int)*i] = alpha_state;
        dfa[string_literal_state_single][(unsigned int)*i] = string_literal_state_single;
        dfa[string_literal_state_double][(unsigned int)*i] = string_literal_state_double;
        dfa[whitespace_state][(unsigned int)*i] = delim_state;
    }

    // Non-ascii alphabetic stuff
    for (int i = 1; i < number_chars; i++)
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
        dfa[string_literal_state_single][i] = string_literal_state_single;
        dfa[string_literal_state_double][i] = string_literal_state_double;
        dfa[whitespace_state][i] = delim_state;
    }

    // Numerical stuff
    for (const char *i = &numer[0]; *i != '\0'; i++)
    {
        dfa[delim_state][(unsigned int)*i] = numerical_state;
        dfa[numerical_state][(unsigned int)*i] = numerical_state;
        dfa[alpha_state][(unsigned int)*i] = alpha_state;
        dfa[dot_state][(unsigned int)*i] = numerical_state;
        dfa[string_literal_state_single][(unsigned int)*i] = string_literal_state_single;
        dfa[string_literal_state_double][(unsigned int)*i] = string_literal_state_double;
        dfa[dash_state][(unsigned int)*i] = numerical_state;
    }

    // Dot stuff
    dfa[delim_state][(unsigned int)'.'] = dot_state;
    dfa[numerical_state][(unsigned int)'.'] = numerical_state;
    dfa[string_literal_state_single][(unsigned int)'.'] = string_literal_state_single;
    dfa[string_literal_state_double][(unsigned int)'.'] = string_literal_state_double;

    // Singleton stuff
    for (const char *i = &singletons[0]; *i != '\0'; i++)
    {
        dfa[delim_state][(unsigned int)*i] = singleton_state;
    }

    // Square bracket stuff
    dfa[delim_state][(unsigned int)'<'] = open_square_bracket_state;
    dfa[delim_state][(unsigned int)'>'] = close_square_bracket_state;
    dfa[open_square_bracket_state][(unsigned int)'='] = operator_state;
    dfa[close_square_bracket_state][(unsigned int)'='] = operator_state;

    // Dash stuff
    dfa[delim_state][(unsigned int)'-'] = dash_state;
    dfa[dash_state][(unsigned int)'>'] = operator_state;

    // Operator stuff
    for (const char *i = &oper[0]; *i != '\0'; i++)
    {
        dfa[delim_state][(unsigned int)*i] = operator_state;
        dfa[operator_state][(unsigned int)*i] = operator_state;
        dfa[string_literal_state_single][(unsigned int)*i] = string_literal_state_single;
        dfa[string_literal_state_double][(unsigned int)*i] = string_literal_state_double;
        dfa[dash_state][(unsigned int)*i] = operator_state;
    }
    dfa[operator_state][(unsigned int)'['] = delim_state;

    // String literal stuff
    for (int i = 0; i < 256; i++)
    {
        dfa[string_literal_state_single][i] = string_literal_state_single;
        dfa[string_literal_state_double][i] = string_literal_state_double;
    }

    dfa[delim_state][(unsigned int)'\''] = string_literal_state_single;
    dfa[delim_state][(unsigned int)'"'] = string_literal_state_double;
    dfa[delim_state][(unsigned int)':'] = colon_state;
    dfa[string_literal_state_single][(unsigned int)'\''] = delim_state;
    dfa[string_literal_state_double][(unsigned int)'"'] = delim_state;

    for (int i = 1; i < number_states; i++)
    {
        if (i != string_literal_state_double)
        {
            dfa[(LexerState)i][(unsigned int)'\''] = delim_state;
        }
        if (i != string_literal_state_single)
        {
            dfa[(LexerState)i][(unsigned int)'"'] = delim_state;
        }
    }

    // Whitespace stuff
    for (const char *i = &whitespace[0]; *i != '\0'; i++)
    {
        dfa[delim_state][(unsigned int)*i] = whitespace_state;
        dfa[string_literal_state_single][(unsigned int)*i] = string_literal_state_single;
        dfa[string_literal_state_double][(unsigned int)*i] = string_literal_state_double;
    }

    dfa[string_literal_state_single][(unsigned int)'\n'] = delim_state;
    dfa[string_literal_state_double][(unsigned int)'\n'] = delim_state;

    // Misc
    dfa[alpha_state][(unsigned int)'!'] = alpha_state;

    for (int i = 0; i < number_states; i++)
    {
        if (i == string_literal_state_single || i == string_literal_state_double || i == whitespace_state)
        {
            continue;
        }

        dfa[i][(unsigned int)'$'] = dollar_sign_state;
    }
    for (int i = 0; i < number_chars; i++)
    {
        dfa[dollar_sign_state][i] = dollar_sign_state;
        dfa[colon_state][i] = delim_state;
    }

    dfa[dollar_sign_state][(unsigned int)' '] = delim_state;
    dfa[dollar_sign_state][(unsigned int)'\t'] = delim_state;
    dfa[dollar_sign_state][(unsigned int)'\n'] = delim_state;
    dfa[colon_state][(unsigned int)':'] = colon_state;
}

Lexer::~Lexer()
{
    if (!is_responsible)
    {
        return;
    }

    for (int i = 0; i < number_states; i++)
    {
        delete[] dfa[i];
    }
    delete[] dfa;
    dfa = 0;

    is_initialized = false;
}

void Lexer::str(const std::string &from) noexcept
{
    pos = 0;
    line = 1;
    text = from;
}

void Lexer::file(const std::string &filepath)
{
    pos = 0;
    line = 1;
    cur_file = filepath;

    std::string line;
    std::ifstream f(filepath, std::ios::ate);

    if (!f.is_open())
    {
        throw std::runtime_error("Failed to open file '" + filepath + "'");
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
    // Assume we are in null_state right now
    LexerState prev_state = delim_state;
    Token out;

    out.pos = pos;
    out.line = line;
    out.file = cur_file;

    bool skip = false;
    do
    {
        if (text[pos] == '\n' && pos != out.pos)
        {
            if (state == string_literal_state_single || state == string_literal_state_double)
            {
                state = delim_state;
                break;
            }
            line++;
        }

        if (skip)
        {
            pos++;
            skip = false;
            continue;
        }
        else if (text[pos] == '\\')
        {
            skip = true;
            pos++;
            continue;
        }

        prev_state = state;
        state = dfa[state][(unsigned char)text[pos]];
        pos++;
    } while (state != delim_state);

    if (prev_state != string_literal_state_single && prev_state != string_literal_state_double)
    {
        pos--;
    }

    if (pos <= out.pos)
    {
        int start, end;
        start = std::max(0ll, (long long)(pos - 20));
        end = std::min((long long)text.size(), (long long)(pos + 20));

        std::string sub = text.substr(start, end - start);
        for (int i = 0; i < sub.size(); i++)
        {
            if (sub[i] == '\n')
            {
                sub[i] = ' ';
            }
        }

        std::cout << tags::violet_bold << "In region:\n" << sub << "\n";
        for (int i = start; i < pos; i++)
        {
            std::cout << ' ';
        }
        std::cout << "^\n\n" << tags::reset;

        throw std::runtime_error("Invalid lex: Token cannot be of size zero!");
    }

    // Record the falling state and text
    out.text = text.substr(out.pos, pos - out.pos);
    out.state = prev_state;

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
            if (it->substr(0, 2) == "//" || it->substr(0, 1) == "#")
            {
                while (it->text != "\n")
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
        if (it->state == whitespace_state)
        {
            it = what.erase(it);
            it--;
        }
    }
}

void join_numbers(std::list<Token> &what)
{
    for (auto it = what.begin(); it != what.end(); it++)
    {
        if (it->state == numerical_state)
        {
            it++;

            while (it->state == numerical_state)
            {
                std::string temp = it->text;
                it--;
                it->text += temp;
                it++;

                it = what.erase(it);
            }

            // Join numerical type-suffixes
            if (*it == "u8" || *it == "u16" || *it == "u32" || *it == "u64" || *it == "u128" || *it == "i8" ||
                *it == "i16" || *it == "i32" || *it == "i64" || *it == "i128" || *it == "f32" || *it == "f64")
            {
                std::string temp = it->text;
                it--;
                it->text += temp;
                it++;

                it = what.erase(it);
            }

            it--;
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
        if (it->state == string_literal_state_single)
        {
            it++;

            while (it->state == string_literal_state_single)
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
        if (it->state == string_literal_state_double)
        {
            it++;

            while (it->state == string_literal_state_double)
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
            Otherwise, is templating; Advance i past all of this.
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
                next->state = operator_state;

                it = what.erase(it);
            }
        }

        // The above code will skip past any valid templating.
        // Therefore, no need to peek ahead for right bitshifts.

        else if (*it == ">" && *next == ">")
        {
            // Merge into a right bit shift
            next->text = ">>";
            next->state = operator_state;

            it = what.erase(it);
        }
    }
}

void Lexer::str(const std::string &from, const std::string &filepath) noexcept
{
    pos = 0;
    line = 1;
    text = from;
    cur_file = filepath;
}

std::list<Token> Lexer::str_all(const std::string &from) noexcept
{
    pos = 0;
    line = 1;
    text = from;

    std::list<Token> out;

    while (!done())
    {
        out.push_back(single());
    }

    return out;
}

std::list<Token> Lexer::str_all(const std::string &from, const std::string &filepath) noexcept
{
    pos = 0;
    line = 1;
    text = from;
    cur_file = filepath;

    std::list<Token> out;

    while (!done())
    {
        out.push_back(single());
    }

    return out;
}

std::vector<Token> Lexer::lex(const std::string &What, const std::string &filepath)
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

    join_numbers(out);
    join_bitshifts(out);
    join_strings(out);

    std::vector<Token> out_vec;
    out_vec.assign(out.begin(), out.end());
    return out_vec;
}
