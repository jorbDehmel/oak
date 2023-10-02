/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "lexer.hpp"
using namespace std;

const string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
const string numbers = "0123456789";

vector<string> lex(const string &What)
{
    vector<string> out;
    string cur = "";
    unsigned long long line = 1;

    char c;

    for (unsigned int i = 0; i < What.size(); i++)
    {
        c = What[i];

        // Universal deliminators ignored
        // Ex: whitespace
        if (c == ' ' || c == '\t' || c == '\n')
        {
            if (cur != "")
            {
                out.push_back(cur);
                cur = "";
            }

            if (c == '\n')
            {
                // Newline. Increment line count and insert line special symbol
                line++;
                out.push_back("//__LINE__=" + to_string(line));
            }

            continue;
        }

        // Rule processing; Use whole word
        else if (c == '$')
        {
            while (i < What.size() && What[i] != ' ' && What[i] != '\t')
            {
                cur += What[i];
                i++;
            }

            out.push_back(cur);
            cur = "";

            continue;
        }

        // Preproc, should NOT get to this point!
        else if (c == '#')
        {
            // The whole line is garbage for our concerns
            while (i < What.size() && What[i] != '\n')
            {
                i++;
            }
            continue;
        }

        // Single-line comments, also shouldn't get to this point
        else if (c == '/' && i + 1 < What.size() && What[i + 1] == '/')
        {
            while (What[i] != '\n')
            {
                i++;
            }

            line++;
            out.push_back("//__LINE__=" + to_string(line));

            continue;
        }

        // Multi-line comments, ditto as above
        else if (c == '/' && i + 1 < What.size() && What[i + 1] == '*')
        {
            int count = 0;

            while (true)
            {
                if (i + 2 < What.size() && What.substr(i, 2) == "/*")
                {
                    count++;
                }
                else if (i + 2 < What.size() && What.substr(i, 2) == "*/")
                {
                    count--;
                }

                if (i + 1 >= What.size())
                {
                    break;
                }
                else if (count == 0 && What[i] == '*' && What[i + 1] == '/')
                {
                    break;
                }
                else
                {
                    if (What[i] == '\n')
                    {
                        line++;
                        out.push_back("//__LINE__=" + to_string(line));
                    }

                    i++;
                }
            }
            i++;
            continue;
        }

        // Conditional deliminators
        // Ex: -> <= >= == != && ||
        // -= += /= *= %= ^= &= |=
        else if (c == '-')
        {
            if (i + 1 >= What.size())
            {
                cur += c;
                out.push_back(cur);
                cur = "";
            }
            else if (What[i + 1] != '>' && What[i + 1] != '=')
            {
                out.push_back(cur);

                cur = c;
                out.push_back(cur);
                cur = "";
            }
            else
            {
                if (cur != "")
                {
                    out.push_back(cur);
                }

                i++;
                cur = c;
                cur += What[i];
                out.push_back(cur);
                cur = "";
            }
            continue;
        }

        else if (c == '<')
        {
            if (i + 1 >= What.size())
            {
                cur += c;
                out.push_back(cur);
                cur = "";
            }
            else if (What[i + 1] != '=' && What[i + 1] != '<')
            {
                out.push_back(cur);

                cur = c;
                out.push_back(cur);
                cur = "";
            }
            else
            {
                if (cur != "")
                {
                    out.push_back(cur);
                }

                i++;
                cur = c;

                // Nested templates fix
                if (What[i] == '<' && i + 1 < What.size() && What[i + 1] != ' ')
                {
                    out.push_back(cur);
                    cur = "";
                }

                cur += What[i];
                out.push_back(cur);
                cur = "";
            }
            continue;
        }

        else if (c == '>')
        {
            if (i + 1 >= What.size())
            {
                out.push_back(cur);

                cur = c;
                out.push_back(cur);
                cur = "";
            }
            else if (What[i + 1] != '=' && What[i + 1] != '>')
            {
                out.push_back(cur);

                cur = c;
                out.push_back(cur);
                cur = "";
            }
            else
            {
                if (cur != "")
                {
                    out.push_back(cur);
                }

                i++;
                cur = c;

                // Nested templates fix
                if (What[i] == '>' && i + 1 < What.size() && What[i + 1] != ' ')
                {
                    out.push_back(cur);
                    cur = "";
                }

                cur += What[i];
                out.push_back(cur);
                cur = "";
            }
            continue;
        }

        else if (c == '=')
        {
            if (i + 1 >= What.size())
            {
                cur += c;
                out.push_back(cur);
                cur = "";
            }
            else if (What[i + 1] != '=')
            {
                out.push_back(cur);

                cur = c;
                out.push_back(cur);
                cur = "";
            }
            else
            {
                if (cur != "")
                {
                    out.push_back(cur);
                }

                i++;
                cur = c;
                cur += What[i];
                out.push_back(cur);
                cur = "";
            }
            continue;
        }

        // Boolean not (ignores the trailing ! on macros!)
        else if (c == '!' && (i == 0 || What[i - 1] == ' '))
        {
            if (i + 1 >= What.size())
            {
                cur += c;
                out.push_back(cur);
                cur = "";
            }
            else if (What[i + 1] != '=')
            {
                out.push_back(cur);

                cur = c;
                out.push_back(cur);
                cur = "";
            }
            else
            {
                if (cur != "")
                {
                    out.push_back(cur);
                }

                i++;
                cur = c;
                cur += What[i];
                out.push_back(cur);
                cur = "";
            }
            continue;
        }

        else if (c == '&')
        {
            if (i + 1 >= What.size())
            {
                cur += c;
                out.push_back(cur);
                cur = "";
            }
            else if (What[i + 1] != '&' && What[i + 1] != '=')
            {
                out.push_back(cur);

                cur = c;
                out.push_back(cur);
                cur = "";
            }
            else
            {
                if (cur != "")
                {
                    out.push_back(cur);
                }

                i++;
                cur = c;
                cur += What[i];
                out.push_back(cur);
                cur = "";
            }
            continue;
        }

        else if (c == '|')
        {
            if (i + 1 >= What.size())
            {
                cur += c;
                out.push_back(cur);
                cur = "";
            }
            else if (What[i + 1] != '|' && What[i + 1] != '=')
            {
                out.push_back(cur);

                cur = c;
                out.push_back(cur);
                cur = "";
            }
            else
            {
                if (cur != "")
                {
                    out.push_back(cur);
                }

                i++;
                cur = c;
                cur += What[i];
                out.push_back(cur);
                cur = "";
            }
            continue;
        }

        else if (c == ':')
        {
            if (cur != "")
            {
                out.push_back(cur);
            }

            cur = ":";

            if (i + 1 < What.size() && What[i + 1] == ':')
            {
                i++;
                out.push_back("::");
                cur = "";
            }
            else
            {
                out.push_back(":");
                cur = "";
            }
            continue;
        }

        else if (c == '+')
        {
            if (What.size() > i + 1 && What[i + 1] == '=')
            {
                cur += c;
                cur += What[i + 1];
                out.push_back(cur);
                i++;
                cur = "";
            }
            else if (What.size() > i + 1 && What[i + 1] == '+')
            {
                // Increment operator ++
                cur += c;
                cur += What[i + 1];
                out.push_back(cur);
                i++;
                cur = "";
            }
            else
            {
                out.push_back(cur);
                cur = c;
                out.push_back(cur);
                cur = "";
            }
            continue;
        }

        else if (c == '-')
        {
            if (What.size() > i + 1 && (What[i + 1] == '=' || What[i + 1] == '-'))
            {
                cur += c;
                cur += What[i + 1];
                out.push_back(cur);
                i++;
                cur = "";
            }
            else
            {
                out.push_back(cur);
                cur = c;
                out.push_back(cur);
                cur = "";
            }
            continue;
        }

        else if (c == '*')
        {
            if (What.size() > i + 1 && What[i + 1] == '=')
            {
                cur += c;
                cur += What[i + 1];
                out.push_back(cur);
                i++;
                cur = "";
            }
            else
            {
                out.push_back(cur);
                cur = c;
                out.push_back(cur);
                cur = "";
            }
            continue;
        }

        else if (c == '/')
        {
            if (What.size() > i + 1 && What[i + 1] == '=')
            {
                cur += c;
                cur += What[i + 1];
                out.push_back(cur);
                i++;
                cur = "";
            }
            else
            {
                out.push_back(cur);

                cur = c;
                out.push_back(cur);
                cur = "";
            }
            continue;
        }

        else if (c == '%')
        {
            if (What.size() > i + 1 && What[i + 1] == '=')
            {
                cur += c;
                cur += What[i + 1];
                out.push_back(cur);
                i++;
                cur = "";
            }
            else
            {
                out.push_back(cur);

                cur = c;
                out.push_back(cur);
                cur = "";
            }
            continue;
        }

        else if (c == '^')
        {
            if (What.size() > i + 1 && What[i + 1] == '=')
            {
                cur += c;
                cur += What[i + 1];
                out.push_back(cur);
                i++;
                cur = "";
            }
            else
            {
                out.push_back(cur);
                cur = c;
                out.push_back(cur);
                cur = "";
            }
            continue;
        }

        // Universal deliminators (non-ignored)
        else if (c == ';' || c == '(' || c == ')' || c == '[' ||
                 c == ']' || c == '{' || c == '}' || c == '.' ||
                 c == ',' || c == '^' || c == '@')
        {
            if (cur != "")
            {
                out.push_back(cur);
            }

            cur = c;
            out.push_back(cur);
            cur = "";
            continue;
        }

        // String literals
        else if ((i == 0 || What[i - 1] != '\\') && (c == '"' || c == '\''))
        {
            if (cur != "")
            {
                out.push_back(cur);
                cur = "";
            }

            i++;
            while (i < What.size() && What[i] != c)
            {
                if (What[i] == '\\')
                {
                    cur += What[i];
                    cur += What[i + 1];
                    i += 2;
                }
                else
                {
                    cur += What[i];
                    i++;
                }
            }

            out.push_back("\"" + cur + "\"");
            cur = "";

            continue;
        }

        // Character set change deliminator (number to letter only)
        else if (i + 1 < What.size() && numbers.find(c) != string::npos && alphabet.find(What[i + 1]) != string::npos)
        {
            cur += c;
            out.push_back(cur);
            cur = "";
            continue;
        }

        // Non-deliminators
        else
        {
            cur += c;
        }
    }

    if (cur != "")
    {
        out.push_back(cur);
        cur = "";
    }

    for (int i = 0; i < out.size(); i++)
    {
        if (out[i] == "")
        {
            out.erase(out.begin() + i);
            i--;
        }
    }

    // Coagulate number literals
    string prev, next;
    for (int i = 0; i < out.size(); i++)
    {
        prev = i == 0 ? "" : out[i - 1];
        cur = out[i];
        next = i + 1 == out.size() ? "" : out[i + 1];

        if (cur == ".")
        {
            if (prev != "" && (('0' <= prev[0] && prev[0] <= '9') || (prev != "-" && '0' <= prev[1] && prev[1] <= '9')))
            {
                out[i] = prev + out[i];
                out.erase(out.begin() + (i - 1));
                i--;

                cur = out[i];
                next = out[i + 1];
            }

            if (next != "" && ('0' <= next[0] && next[0] <= '9'))
            {
                out[i] += next;
                out.erase(out.begin() + (i + 1));

                cur = out[i];
                next = out[i + 1];
            }
        }
        else if (cur == "-")
        {
            if (next != "" && ('0' <= next[0] && next[0] <= '9'))
            {
                out[i] += next;
                out.erase(out.begin() + (i + 1));

                cur = out[i];
                next = out[i + 1];
            }
        }

        prev = cur;
    }

    return out;
}

void smartSystem(const string &What)
{
    int result = system(What.c_str());

    if (result != 0)
    {
        throw runtime_error("System call `" + What + "` failed with code " + to_string(result));
    }

    return;
}
