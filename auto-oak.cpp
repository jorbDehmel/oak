/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "auto-oak.hpp"

// Populates the Oak symbol table from a C file
// TODO: Detect #includes and autoOak those, too
int autoOak(const string &Filename)
{
    bool isValidType = false;
    if (Filename.size() > 3)
    {
        if (Filename.substr(Filename.size() - 2) == ".c" ||
            Filename.substr(Filename.size() - 2) == ".h")
        {
            isValidType = true;
        }
    }
    else if (Filename.size() > 5)
    {
        if (Filename.substr(Filename.size() - 4) == ".cpp" ||
            Filename.substr(Filename.size() - 4) == ".hpp")
        {
            isValidType = true;
        }
    }

    if (!isValidType)
    {
        return 2;
    }

    ifstream file(Filename);
    if (!file.is_open())
    {
        return 1;
    }

    // Load entire file
    string text, line;
    while (getline(file, line))
    {
        text += line + '\n';
    }

    file.close();

    const vector<string> lexed = lex(text);

    // Filter out all struct / class definitions
    // regex oak: 'let ([^:a-zA-Z0-9]+): ([^ {\n\t]+)[ \t\n]*{'
    // regex cpp: '(class)|(struct) ([^ \n\t]*)

    // Add these to the symbol table

    // Fix all method call signatures
    // (same as a function call, but within a class)

    // Add these to the symbol table

    // Filter out all function signatures

    // Add these to the symbol table

    string curStruct = "";
    int scope = 0;
    for (int i = 0; i < lexed.size(); i++)
    {
        string cur = lexed[i];

        if (cur == "{")
        {
            scope++;
        }
        else if (cur == "}")
        {
            scope--;
        }

        // Ensure we are only looking at globals
        else if (scope == 0)
        {
            if (cur == "struct" || cur == "class")
            {
                curStruct = lexed[i + 1];
                structData[curStruct] = __structLookupData{
                    set<string>(),
                    Type(),
                    map<string, Type>()};

                // TODO: Scan member variables and methods
                // With the current code,
                /*
                struct hi
                {
                    int a, b;
                    string hihi;
                };

                <=>

                let hi: struct
                {
                    ,
                }
                */
            }

            // TODO: Scan for functions
        }
    }

    return 0;
}

/*
class hi
{
public:
    hi();
    hi(int &Hi, string &Hello);
    ~hi();

    int getHi() const;
    int publicVar;
private:
    int hi;
    string hello;
};

struct hihi : public hi;

// Becomes

let hi: struct
{
    publicVar: i16,
}

let hi.hi(self: *hi) -> *hi;
let hi.hi(self: *hi, Hi: *i16, Hello: *str) -> *hi;
let hi.Del(self: *hi) -> void;
let hi.getHi(self: *hi) -> i16;

let hihi: hi
{
    ,
}

// Oak does not have private types, so the only cross-language
// access to these types can be via getters and setters.

*/
