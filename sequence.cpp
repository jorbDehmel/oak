/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "sequence.hpp"
#include "type-builder.hpp"

// Activates "dirty" mode, where mem alloc and free are allowed
bool insideMethod = false;

vector<string> curLineSymbols;

unsigned long long int curLine = 1;
string curFile = "";

// The current depth of createSequence
unsigned long long int depth = 0;

// Destroy all unit, temp, or autogen definitions matching a given type.
// Can throw errors if doThrow is true.
// Mostly used for New and Del, Oak ~0.0.14
void destroyUnits(const string &name, const Type &type, const bool &doThrow)
{
    // Safety check
    if (table.count(name) != 0 && table[name].size() != 0)
    {
        // Iterate over items
        for (int i = 0; i < table[name].size(); i++)
        {
            if (table[name][i].type == type)
            {
                // If is unit, erase
                if (table[name][i].seq.items.size() == 0 ||
                    (table[name][i].seq.items.size() >= 1 && table[name][i].seq.items[0].raw.size() > 8 &&
                     table[name][i].seq.items[0].raw.substr(0, 9) == "//AUTOGEN"))
                {
                    table.at(name).erase(table[name].begin() + i);
                    i--;
                }

                // Else if doThrow, throw redef error
                else if (doThrow)
                {
                    throw sequencing_error("Function " + name + toStr(&type) +
                                           "' cannot have multiple explicit definitions.");
                }
            }
        }
    }

    return;
}

// returns true if a should be before b
bool sortCandidates(__multiTableSymbol &a, __multiTableSymbol &b)
{
    auto a_args = getArgs(a.type);
    auto b_args = getArgs(b.type);

    int a_count = 0;
    int b_count = 0;

    for (int i = 0; i < a_args[0].second.size() && a_args[0].second[i].info == pointer; i++)
    {
        a_count++;
    }

    for (int i = 0; i < b_args[0].second.size() && b_args[0].second[i].info == pointer; i++)
    {
        b_count++;
    }

    return a_count < b_count;
}

// Actually used in dump files, not just for debugging.
void debugPrint(const sequence &What, int spaces, ostream &to)
{
    for (int i = 0; i < spaces; i++)
    {
        to << "| ";
    }

    switch (What.info)
    {
    case code_scope:
        to << "code_scope";
        break;
    case code_line:
        to << "code_line";
        break;
    case atom:
        to << "atom";
        break;
    case keyword:
        to << "keyword";
        break;
    case enum_keyword:
        to << "enum_keyword";
        break;
    }

    to << " w/ raw " << What.raw << ", type " << toStr(&What.type) << ":\n";
    for (auto s : What.items)
    {
        debugPrint(s, spaces + 1, to);
    }

    return;
}

sequence createSequence(const vector<string> &From)
{
    // Clone to feed into the consumptive version
    // vector<string> temp(From);
    list<string> temp;
    temp.assign(From.begin(), From.end());

    vector<sequence> out;

    // Feed to consumptive version
    while (!temp.empty())
    {
        out.push_back(__createSequence(temp));
    }

    if (out.size() == 0)
    {
        return sequence{};
    }
    else if (out.size() == 1)
    {
        return out[0];
    }
    else
    {
        sequence outSeq;
        outSeq.info = code_scope;
        outSeq.type = nullType;

        for (auto i : out)
        {
            outSeq.items.push_back(i);
        }
        return outSeq;
    }
}

// Internal consumptive version: Erases from vector, so not safe for frontend
sequence __createSequence(list<string> &From)
{
    static string prevMatchTypeStr = "NULL";

    sequence out;
    out.info = code_line;

    // Misc. Invalid cases
    if (From.empty())
    {
        return out;
    }
    else if (strncmp(From.front().c_str(), "//__LINE__=", 11) == 0)
    {
        // Line update special symbol
        string newLineNum = From.front().substr(11);

        curLine = stoull(newLineNum);

        curLineSymbols.clear();
        if (From.size() != 0)
        {
            for (auto it = From.begin();
                 it != From.end() && !(it != From.begin() && it->size() > 11 && it->substr(0, 11) == "//__LINE__=");
                 it++)
            {
                curLineSymbols.push_back(*it);
            }
        }

        if (curLineSymbols.size() != 0)
        {
            curLineSymbols.erase(curLineSymbols.begin());
        }

        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();
        return __createSequence(From);
    }
    else if (From.front() == ",")
    {
        return out;
    }

    else if (From.front().size() > 1 && From.front().back() == '!')
    {
        // Erasure macro; Erases types or struct members from existence
        // Technically just marks them erased
        if (From.front() == "erase!")
        {
            int count = 0;
            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();
            list<string> contents;

            do
            {
                if (From.front() == "(")
                {
                    count++;
                }
                else if (From.front() == ")")
                {
                    count--;
                }

                if (!(count == 1 && From.front() == "(") && !(count == 0 && From.front() == ")"))
                {
                    if (From.front() != ",")
                    {
                        contents.push_back(From.front());
                    }
                }

                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.pop_front();
            } while (!From.empty() && count != 0);

            for (auto s : contents)
            {
                if (!((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\'')))
                {
                    throw parse_error("All arguments to 'erase!' must be strings.");
                }

                // Trim quotes
                string symb = s.substr(1, s.size() - 2);

                if (structData.count(symb) != 0)
                {
                    structData[symb].erased = true;
                }
                else if (table.count(symb) != 0)
                {
                    for (int l = 0; l < table[symb].size(); l++)
                    {
                        table[symb][l].erased = true;
                    }
                }
                else
                {
                    cout << tags::yellow_bold << "Warning! No symbols were erase!-ed via the symbol '" << symb << "'\n"
                         << tags::reset;
                }
            }

            return out;
        }

        // Misc macros
        else if (From.front() == "c_print!")
        {
            string message = curFile + ":" + to_string(curLine) + ":c_print! ";

            int count = 0;
            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();

            do
            {
                if (From.front() == "(")
                {
                    count++;
                }
                else if (From.front() == ")")
                {
                    count--;
                }

                if (strncmp(From.front().c_str(), "//", 2) != 0)
                {
                    if (!(count == 1 && From.front() == "(") && !(count == 0 && From.front() == ")"))
                    {
                        message += From.front() + " ";
                    }
                }

                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.pop_front();

            } while (!From.empty() && count != 0);

            message += "\n";

            cout << message;

            return out;
        }
        else if (From.front() == "c_panic!")
        {
            string message = curFile + ":" + to_string(curLine) + ":c_panic! ";

            int count = 0;
            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();

            do
            {
                if (From.front() == "(")
                {
                    count++;
                }
                else if (From.front() == ")")
                {
                    count--;
                }

                if (strncmp(From.front().c_str(), "//", 2) != 0)
                {
                    if (!(count == 1 && From.front() == "(") && !(count == 0 && From.front() == ")"))
                    {
                        message += From.front() + " ";
                    }
                }

                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.pop_front();
            } while (!From.empty() && count != 0);

            message += "\n";

            throw sequencing_error(message);
        }
        else if (From.front() == "c_sys!")
        {
            cout << curFile << ":" << to_string(curLine) << ":c_sys! ";

            string command;
            int count = 0;
            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();

            do
            {
                if (From.front() == "(")
                {
                    count++;
                }
                else if (From.front() == ")")
                {
                    count--;
                }

                if (From.front().size() > 2 && strncmp(From.front().c_str(), "//", 2) != 0)
                {
                    if (!(count == 1 && From.front() == "(") && !(count == 0 && From.front() == ")"))
                    {
                        if (command != "")
                        {
                            command += " && ";
                        }

                        command += From.front().substr(1, From.front().size() - 2);
                    }
                }

                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.pop_front();
            } while (!From.empty() && count != 0);

            cout << '`' << command << "`\n";
            int res = system(command.c_str()) != 0;

            if (res != 0)
            {
                cout << tags::red_bold << "Compile-time command failed w/ error code " << res << ".\n" << tags::reset;
            }

            return out;
        }

        // else

        // Memory Keywords
        else if (From.front() == "alloc!")
        {
            sm_assert(insideMethod, "Memory cannot be allocated outside of an operator-alias method.");

            int count = 0;
            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();
            list<string> contents;

            do
            {
                if (From.front() == "(")
                {
                    count++;
                }
                else if (From.front() == ")")
                {
                    count--;
                }

                if (!(count == 1 && From.front() == "(") && !(count == 0 && From.front() == ")"))
                {
                    contents.push_back(From.front());
                }

                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.pop_front();
            } while (!From.empty() && count != 0);

            string num = "1";

            sequence temp = __createSequence(contents);
            string name = toC(temp);

            while (contents.front() == ",")
            {
                contents.pop_front();
            }

            sequence numSeq = __createSequence(contents);
            num = toC(numSeq);

            Type tempType = temp.type;
            Type numType = numSeq.type;

            sm_assert(tempType.size() > 0 && tempType[0].info == pointer, "'alloc!' returns a pointer.");
            sm_assert(tempType.size() > 0, "'alloc!' received a malformed first argument.");

            temp.type.pop_front();

            if (numType == nullType)
            {
                num = "1";
                numType = typeNode{atomic, "u128"};
            }

            sm_assert(numType[0].info == atomic && numType[0].name == "u128",
                      "'alloc!' takes 'u128', not '" + toStr(&numType) + "'.");

            out = getAllocSequence(temp.type, name, num);

            return out;
        }
        else if (From.front() == "free!")
        {
            sm_assert(insideMethod, "Memory cannot be deleted outside of an operator-alias method.");

            int count = 0;
            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();
            list<string> contents;

            do
            {
                if (From.front() == "(")
                {
                    count++;
                }
                else if (From.front() == ")")
                {
                    count--;
                }

                if (!(count == 1 && From.front() == "(") && !(count == 0 && From.front() == ")"))
                {
                    contents.push_back(From.front());
                }

                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.pop_front();
            } while (!From.empty() && count != 0);

            sequence temp = __createSequence(contents);
            string name = toC(temp);

            Type tempType = temp.type;
            sm_assert(tempType[0].info == pointer, "'free!' takes a pointer.");
            temp.type.pop_front();

            out = getFreeSequence(name, false);

            return out;
        }
        else if (From.front() == "ptrcpy!")
        {
            sm_assert(insideMethod, "Pointers cannot be copied outside of an operator-alias method.");

            int count = 0;
            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();
            list<string> contents;

            do
            {
                if (From.front() == "(")
                {
                    count++;
                }
                else if (From.front() == ")")
                {
                    count--;
                }

                if (!(count == 1 && From.front() == "(") && !(count == 0 && From.front() == ")"))
                {
                    contents.push_back(From.front());
                }

                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.pop_front();
            } while (!From.empty() && count != 0);

            sequence lhsSeq = __createSequence(contents);
            string lhs = toC(lhsSeq);

            while (contents.front() == ",")
            {
                contents.pop_front();
            }

            sequence rhsSeq = __createSequence(contents);
            string rhs = toC(rhsSeq);

            sm_assert(lhsSeq.type[0].info == pointer, "First argument of cpyptr! must be pointer.");
            sm_assert(rhsSeq.type[0].info == pointer || rhsSeq.raw == "0",
                      "Second argument of cpyptr! must be a pointer or 0.");

            out.info = code_line;
            out.type = nullType;
            out.items.clear();

            out.items.push_back(sequence{nullType, vector<sequence>(), atom, lhs + " = (void*)(" + rhs + ")"});

            return out;
        }
        else if (From.front() == "ptrarr!")
        {
            int count = 0;
            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();
            list<string> contents;

            do
            {
                if (From.front() == "(")
                {
                    count++;
                }
                else if (From.front() == ")")
                {
                    count--;
                }

                if (!(count == 1 && From.front() == "(") && !(count == 0 && From.front() == ")"))
                {
                    contents.push_back(From.front());
                }

                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.pop_front();
            } while (!From.empty() && count != 0);

            sequence lhsSeq = __createSequence(contents);
            string lhs = toC(lhsSeq);

            while (contents.front() == ",")
            {
                contents.pop_front();
            }

            sequence rhsSeq = __createSequence(contents);
            string rhs = toC(rhsSeq);

            sm_assert(lhsSeq.type[0].info == pointer, "First argument of ptrarr! must be pointer.");
            sm_assert(rhsSeq.type == Type(atomic, "u128"),
                      "Second argument of ptrarr! must be a u128, not " + toStr(&rhsSeq.type));

            out.info = atom;
            out.type = Type(lhsSeq.type, 1);
            out.raw = "(" + lhs + "[" + rhs + "])";

            return out;
        }
    }

    // Misc key-characters
    else if (From.front() == ";")
    {
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();
        auto toReturn = __createSequence(From);
        toReturn.type = nullType;

        return toReturn;
    }

    // Keywords below

    // Enums- VERY C++ dependant!
    else if (From.front() == "match")
    {
        // Takes an enum code line and a code scope / code line
        out.info = enum_keyword;
        out.raw = From.front();
        out.type = nullType;

        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        // Pops the first full sequence from the remaining front
        // This is for the enum
        out.items.push_back(__createSequence(From));

        sm_assert(out.items[0].type[0].info == atomic && enumData.count(out.items[0].type[0].name) != 0,
                  out.raw + " statement argument must be enum. Instead, '" + toStr(&out.items[0].type) + "'");
        sm_assert(!From.empty(), "Missing statement after " + out.raw + "");

        string old = prevMatchTypeStr;
        prevMatchTypeStr = out.items[0].type[0].name;

        // This is for the code chunk
        out.items.push_back(__createSequence(From));

        prevMatchTypeStr = old;

        // Ensure internal safety
        for (int j = 1; j < out.items.size(); j++)
        {
            if (out.items[j].raw == "")
            {
                continue;
            }

            if (out.items[j].raw != "case" && out.items[j].raw != "default")
            {
                throw sequencing_error("Match statement must contain only 'case' and 'default' statements, not '" +
                                       out.items[j].raw + "'.");
            }

            if (out.items[j].raw == "default" && j != out.items.size() - 1)
            {
                throw sequencing_error("Default statement must be the final branch of a match statement.");
            }
        }

        return out;
    }
    else if (From.front() == "case")
    {
        // case NAME() {}
        sm_assert(prevMatchTypeStr != "NULL", "'case' statement must occur within a 'match' statement.");

        // Takes a code scope / code line
        out.info = enum_keyword;
        out.raw = From.front();
        out.type = nullType;

        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        sm_assert(!From.empty(), "'case' must be followed by enumeration option name.");
        out.items.push_back(sequence{nullType, vector<sequence>{}, atom, From.front()});
        string optionName = From.front();

        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        sm_assert(!From.empty() && From.front() == "(",
                  "Enumeration option must be followed by capture parenthesis (IE name(capture_here)).");
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        sm_assert(!From.empty(), "Capture group is missing name or closing parenthesis.");
        string captureName = "NULL";

        if (From.front() != ")")
        {
            captureName = From.front();
            out.items.push_back(sequence{nullType, vector<sequence>{}, atom, From.front()});
            From.pop_front();

            table[captureName].push_back(__multiTableSymbol{sequence{}, pointer, false, curFile});
            table[captureName].back().type.append(enumData[prevMatchTypeStr].options[optionName]);
        }
        else
        {
            // Padding so that this spot will always refer to the capture variable
            out.items.push_back(sequence{nullType, vector<sequence>{}, atom, "NULL"});
        }

        sm_assert(!From.empty() && From.front() == ")", "Capture parenthesis must contain at most one symbol.");

        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        // Get actual code scope
        out.items.push_back(__createSequence(From));

        // Remove capture group from Oak table if needed
        if (captureName != "NULL")
        {
            table[captureName].pop_back();
        }

        return out;
    }
    else if (From.front() == "default")
    {
        // default {}

        // Takes a code scope / code line
        out.info = enum_keyword;
        out.raw = From.front();
        out.type = nullType;

        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        out.items.push_back(__createSequence(From));

        return out;
    }

    // Conditionals and junk
    else if (From.front() == "if" || From.front() == "while")
    {
        // Takes a bool code line and a code scope / code line
        out.info = keyword;
        out.raw = From.front();
        out.type = nullType;

        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        // Pops the first full sequence from the remaining front
        // This is for the conditional
        out.items.push_back(__createSequence(From));

        sm_assert(out.items[0].type == Type(atomic, "bool"),
                  out.raw + " statement argument must be boolean. Instead, '" + toStr(&out.items[0].type) + "'");
        sm_assert(!From.empty(), "Missing statement after " + out.raw + "");

        // This is for the code chunk
        out.items.push_back(__createSequence(From));

        return out;
    }
    else if (From.front() == "else")
    {
        // Takes a code scope / code line
        out.info = keyword;
        out.raw = From.front();
        out.type = nullType;

        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        out.items.push_back(__createSequence(From));

        return out;
    }
    else if (From.front() == "let")
    {
        // Get name
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front(); // let
        sm_assert(!From.empty(), "'let' must be followed by something.");

        string name = From.front();
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        // Gather templating if there is any
        vector<string> generics;
        if (From.front() == "<")
        {
            int count = 0;

            do
            {
                if (From.front() == "<")
                {
                    count++;
                }
                else if (From.front() == ">")
                {
                    count--;
                }

                if (From.front() != "<" || count != 1)
                {
                    generics.push_back(From.front());
                }

                From.pop_front();
            } while (!From.empty() && !(count == 1 && From.front() == ">"));

            // Trim trailing angle bracket
            if (!From.empty())
            {
                From.pop_front();
            }
        }

        // Get type
        if (From.front() == ":")
        {
            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();

            if (!From.empty() && (From.front() == "struct" || From.front() == "enum"))
            {
                // addStruct takes in the full struct definition, from
                // let to end curly bracket. So we must first parse this.

                vector<string> toAdd = {"let", name};

                // Add generics back in here
                if (generics.size() != 0)
                {
                    toAdd.push_back("<");
                    for (int i = 0; i < generics.size(); i++)
                    {
                        toAdd.push_back(generics[i]);
                    }
                    toAdd.push_back(">");
                }

                toAdd.push_back(":");
                string front = From.front();

                int count = 0;
                while (count != 0 || (!From.empty() && From.front() != "}" && From.front() != ";"))
                {
                    toAdd.push_back(From.front());

                    sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                    From.pop_front();
                }

                if (From.front().size() < 2 || From.front().substr(0, 2) != "//")
                {
                    toAdd.push_back(From.front());
                }

                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.pop_front();

                bool exempt = false;
                for (auto s : generics)
                {
                    if (structData.count(s) != 0 || s == "u8" || s == "i8" || s == "u16" || s == "i16" || s == "u32" ||
                        s == "i32" || s == "u64" || s == "i64" || s == "u128" || s == "i128" || s == "str" ||
                        s == "bool")
                    {
                        exempt = true;
                        break;
                    }
                }

                if (generics.size() == 0 || exempt)
                {
                    if (front == "struct")
                    {
                        // Non-templated struct
                        addStruct(toAdd);
                    }
                    else if (front == "enum")
                    {
                        // Non-templated enum
                        addEnum(toAdd);
                    }
                }
                else
                {
                    // Check for needs / inst block here
                    vector<string> instBlock;

                    while (!From.empty() && From.front().size() > 1 && From.front().substr(0, 2) == "//")
                    {
                        From.pop_front();
                    }

                    if (!From.empty() && From.front() == "needs")
                    {
                        // pop needs
                        From.pop_front();

                        while (!From.empty() && From.front().size() > 1 && From.front().substr(0, 2) == "//")
                        {
                            From.pop_front();
                        }

                        // pop {
                        sm_assert(!From.empty() && From.front() == "{", "'needs' block must be followed by scope.");
                        From.pop_front();

                        int count = 1;
                        while (!From.empty())
                        {
                            if (From.front() == "{")
                            {
                                count++;
                            }
                            else if (From.front() == "}")
                            {
                                count--;
                            }

                            if (count == 0)
                            {
                                From.pop_front();
                                break;
                            }
                            else
                            {
                                instBlock.push_back(From.front());
                                From.pop_front();
                            }
                        }
                    }

                    addGeneric(toAdd, name, generics, instBlock, {front});
                }

                // This should be left out of toC, as it should only be used
                // in the header file during reconstruction.
            }
            else
            {
                // Non-struct definition; Local var, not function

                sm_assert(depth != 0, "Global variables are not allowed.");
                sm_assert(generics.empty(), "Variable declaration must not be templated.");

                // Scrape entire definition for this
                vector<string> toAdd = {
                    "let",
                    name,
                    ":",
                };

                while (!From.empty() && From.front() != ";")
                {
                    if (From.front() == "type!")
                    {
                        // Case for type!() macro

                        // Scrape entire type!(what) call to a vector
                        vector<string> toAnalyze;
                        int count = 0;

                        From.pop_front();
                        do
                        {
                            if (From.front() == "(")
                            {
                                count++;
                            }
                            else if (From.front() == ")")
                            {
                                count--;
                            }

                            if (!((From.front() == "(" && count == 1) || (From.front() == ")" && count == 0)))
                            {
                                toAnalyze.push_back(From.front());
                            }

                            From.pop_front();
                        } while (count != 0);

                        // Garbage to feed to resolveFunction
                        string junk = "";
                        int pos = 0;

                        // Analyze type of vector
                        Type type = resolveFunction(toAnalyze, pos, junk);

                        // Convert type to lexed string vec
                        vector<string> lexedType = lex(toStr(&type));

                        // Push lexed vec to front of From
                        for (auto iter = lexedType.rbegin(); iter != lexedType.rend(); iter++)
                        {
                            From.push_front(*iter);
                        }

                        continue;
                    }

                    toAdd.push_back(From.front());
                    sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                    From.pop_front();

                    if (!From.empty() && From.front() == "=")
                    {
                        throw sequencing_error("Instantiate / assignment combo is not supported.");
                    }
                }

                // This SHOULD appear during toC.
                out.info = code_line;
                out.raw = "";
                out.type = nullType;
                out.items.clear();

                Type type = toType(toAdd);

                out.items.push_back(sequence{nullType, vector<sequence>(), atom, toStrC(&type, name)});

                // Insert into table
                table[name].push_back(
                    __multiTableSymbol{sequence{type, vector<sequence>(), atom, ""}, type, false, curFile});

                // Call constructor (pointers, atomics and enums do not get constructors)
                if (type[0].info != pointer && enumData.count(type[0].name) == 0 && atomics.count(type[0].name) == 0)
                {
                    // Syntactically necessary
                    out.items.push_back(sequence{nullType, vector<sequence>(), atom, ";"});

                    vector<string> newCall = {"New", "(", "@", name, ")"};
                    int garbage = 0;

                    sequence toAppend;
                    toAppend.info = atom;
                    toAppend.type = resolveFunction(newCall, garbage, toAppend.raw);

                    out.items.push_back(toAppend);
                }
                else if (type[0].info == pointer)
                {
                    // Syntactically necessary
                    out.items.push_back(sequence{nullType, vector<sequence>(), atom, ";"});

                    sequence toAppend;
                    toAppend.info = atom;
                    toAppend.type = nullType;
                    toAppend.raw = name + " = 0";

                    out.items.push_back(toAppend);
                }

                return out;
            }
        }
        else if (From.front() == "(")
        {
            // Function definition
            if (generics.size() == 0)
            {
                // Arguments
                vector<string> toAdd;
                do
                {
                    toAdd.push_back(From.front());
                    sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                    From.pop_front();
                } while (From.front() != "{" && From.front() != ";");

                auto type = toType(toAdd);

                auto oldTable = table;

                auto argsWithType = getArgs(type);
                for (pair<string, Type> p : argsWithType)
                {
                    table[p.first].push_back(__multiTableSymbol{sequence(), p.second, false, curFile});
                }

                bool isMethod = false;
                for (char c : name)
                {
                    if ('A' <= c && c <= 'Z')
                    {
                        isMethod = true;
                        break;
                    }
                }

                bool isSingleArg = true;
                for (int ptr = 0; ptr < type.size(); ptr++)
                {
                    if (type[ptr].info == join)
                    {
                        isSingleArg = false;
                        break;
                    }
                }

                // Restrictions upon some types of methods
                if (name == "New" || name == "Del")
                {
                    if (!isSingleArg)
                    {
                        throw runtime_error("Illegal method definition! Method '" + name +
                                            "' must have exactly one argument.");
                    }
                }

                bool oldSafeness = insideMethod;
                insideMethod = isMethod;

                // Insert explicit symbol
                if (From.size() > 0 && From.front() == "{")
                {
                    if (name == "main")
                    {
                        sm_assert(table[name].size() == 0, "Function 'main' cannot be overloaded.");
                    }

                    destroyUnits(name, type, true);

                    table[name].push_back(__multiTableSymbol{sequence{}, type, false, curFile});

                    if (name == "New")
                    {
                        // prepend New for all members
                        string structName;

                        // Type *cur = &argsWithType[0].second;
                        int cur = 0;

                        while (cur < argsWithType[0].second.size() && argsWithType[0].second[cur].info == pointer)
                        {
                            cur++;
                        }

                        structName = argsWithType[0].second[cur].name;

                        for (auto var : structData[structName].members)
                        {
                            string varName = var.first;

                            // Special pointer case
                            if (var.second[0].info == pointer)
                            {
                                sequence seq;
                                seq.info = atom;
                                seq.raw = argsWithType[0].first + "->" + varName + " = 0";
                                seq.type = nullType;

                                table["New"].back().seq.items.push_back(seq);
                            }

                            // Avoid performing on atomics
                            else if (structData.count(var.second[0].name) != 0)
                            {
                                sequence seq;
                                seq.info = atom;
                                seq.type = nullType;

                                seq.raw = "New(&" + argsWithType[0].first + "->" + varName + ")";
                                table["New"].back().seq.items.push_back(seq);
                            }
                        }
                    }

                    depth++;
                    table[name].back().seq = __createSequence(From);
                    depth--;

                    if (name == "Del")
                    {
                        // append Del for all members
                        string structName;

                        // Type *cur = &argsWithType[0].second;
                        int cur = 0;

                        while (cur < argsWithType[0].second.size() && argsWithType[0].second[cur].info == pointer)
                        {
                            cur++;
                        }

                        structName = argsWithType[0].second[cur].name;

                        for (auto var : structData[structName].members)
                        {
                            string varName = var.first;

                            // Special pointer case
                            if (var.second[0].info == pointer)
                            {
                                sequence seq;
                                seq.info = atom;
                                seq.raw = argsWithType[0].first + "->" + varName + " = 0";
                                seq.type = nullType;

                                table["Del"].back().seq.items.push_back(seq);
                            }

                            // Avoid performing on atomics
                            else if (structData.count(var.second[0].name) != 0)
                            {
                                sequence seq;
                                seq.info = atom;
                                seq.type = nullType;

                                seq.raw = "Del_FN_PTR_" + var.second[0].name + "_MAPS_void(&" + argsWithType[0].first +
                                          "->" + varName + ")";
                                table["Del"].back().seq.items.push_back(seq);
                            }
                        }
                    }
                }
                else
                {
                    // Casual reference
                    // IE let to_i32(what: bool) -> i32;
                    // Destroys all autogen, requires
                    // actual definition.

                    destroyUnits(name, type, false);

                    // Ensure exactly one unit declaration
                    table[name].push_back(__multiTableSymbol{sequence{}, type, false, curFile});
                }

                insideMethod = oldSafeness;

                restoreSymbolTable(oldTable);
            }
            else
            {
                // Templated function definition

                vector<string> toAdd = {"let", name}, returnType;
                vector<string> typeVec;

                while (From.front() != "->")
                {
                    if (From.front().size() < 2 || From.front().substr(0, 2) != "//")
                    {
                        typeVec.push_back(From.front());
                        toAdd.push_back(From.front());
                    }

                    From.pop_front();
                }

                toAdd.push_back(From.front());

                From.pop_front();

                while (From.front() != "{" && From.front() != ";")
                {
                    if (From.front().size() < 2 || From.front().substr(0, 2) != "//")
                    {
                        toAdd.push_back(From.front());

                        returnType.push_back(From.front());
                    }

                    From.pop_front();
                }

                sm_assert(From.front() == "{", "Generic functions must be defined at declaration.");

                // Scrape contents
                int count = 0;
                do
                {
                    if (From.front() == "{")
                    {
                        count++;
                    }
                    else if (From.front() == "}")
                    {
                        count--;
                    }

                    if (From.front().size() < 2 || From.front().substr(0, 2) != "//")
                    {
                        toAdd.push_back(From.front());
                    }

                    From.pop_front();
                } while (count != 0);

                // Insert templated function
                // For now, cannot have needs block here
                addGeneric(toAdd, name, generics, vector<string>(), typeVec);
            }
        }

        return out;
    }
    else if (From.front() == "{")
    {
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        // Save symbol table for later restoration
        auto oldTable = table;

        out.info = code_scope;

        // Code scope.
        int count = 1;
        list<string> curVec;
        while (true)
        {
            if (From.empty())
            {
                break;
            }

            if (From.front() == "{")
            {
                count++;
            }
            else if (From.front() == "}")
            {
                count--;

                if (count == 0)
                {
                    out.items.push_back(__createSequence(curVec));
                    curVec.clear();
                    sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                    From.pop_front();
                    break;
                }
            }

            if (count == 1 && (From.front() == ";" || From.front() == "}"))
            {
                if (!curVec.empty())
                {
                    out.items.push_back(__createSequence(curVec));
                    out.items.back().type = nullType;
                    curVec.clear();
                }

                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.pop_front();
            }
            else
            {
                if (!From.empty())
                {
                    curVec.push_back(From.front());
                    sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                    From.pop_front();
                }
                else
                {
                    break;
                }
            }
        }

        // Restore symbol table

        // This copies only newly instantiated functions; No other symbols.
        string output = restoreSymbolTable(oldTable);

        // Call destructors
        out.items.push_back(sequence{nullType, vector<sequence>(), atom, output});

        // Check if/else validity
        for (int i = 1; i < out.items.size(); i++)
        {
            if (out.items[i].info == keyword && out.items[i].raw == "else")
            {
                if (out.items[i - 1].items.size() != 0 && out.items[i - 1].items[0].info == keyword &&
                    out.items[i - 1].items[0].raw == "if")
                {
                    continue;
                }

                sm_assert(out.items[i - 1].info == keyword && out.items[i - 1].raw == "if",
                          "Else statement must be prefixed by if statement.");
            }
        }

        return out;
    }

    // Non-special case; code line.
    // Function calls and template instantiation may occur within.

    while (From.front().size() > 11 && From.front().substr(0, 11) == "//__LINE__=")
    {
        // Line update special symbol
        string newLineNum = From.front().substr(12);
        curLine = stoull(newLineNum);

        // Pop
        From.pop_front();
    }

    sequence temp;
    temp.info = atom;

    int i = 0;

    vector<string> tempVec;
    for (auto i : From)
    {
        tempVec.push_back(i);

        if (i == ";")
        {
            break;
        }
    }
    temp.type = resolveFunction(tempVec, i, temp.raw);

    out.items.push_back(temp);

    // Erase old
    for (int k = 0; !From.empty() && k < i; k++)
    {
        From.pop_front();
    }

    if (out.items.size() == 1)
    {
        return out.items[0];
    }

    return out;
} // __createSequence

// Turn a .oak sequence into a .cpp one
string toC(const sequence &What)
{
    string out = "";
    string temp;
    int scopeReturnCount;

    switch (What.info)
    {
    case code_line:
        for (int i = 0; i < What.items.size(); i++)
        {
            out += toC(What.items[i]);

            if (i + 1 != What.items.size())
            {
                out += " ";
            }
        }

        break;

    case code_scope:
        out += "{\n";

        scopeReturnCount = 0;
        for (int i = 0; i < What.items.size(); i++)
        {
            if (What.items[i].info == keyword)
            {
                out += toC(What.items[i]);
            }
            else if (What.items[i].type == nullType)
            {
                temp = toC(What.items[i]);

                if (temp != "" && temp != ";")
                {
                    out += temp;

                    if (!(temp.size() > 1 && temp.substr(0, 2) == "//"))
                    {
                        out += ";";
                    }

                    out += "\n";
                }
            }
            else
            {
                if (scopeReturnCount == 0)
                {
                    out += "return (" + toC(What.items[i]) + ");\n";
                    scopeReturnCount++;
                }
                else
                {
                    throw sequencing_error("A function definition must have exactly one return point.");
                }
            }
        }

        out += "}\n";

        break;

    case atom:
        out += What.raw;
        break;

    case keyword:
        out += What.raw + " ";

        for (auto child : What.items)
        {
            out += toC(child) + " ";
        }

        break;

    case enum_keyword:
        // These get special treatment because they are heavily C++-dependant.
        if (What.raw == "match")
        {
            // VERY different!
            // First item is object to capture.
            // Remaining items are case or defaults.

            string itemStr = toC(What.items[0]);

            Type type = What.items[0].type;
            string typeStr = toStrC(&type);

            if (typeStr.substr(0, 7) == "struct ")
            {
                typeStr = typeStr.substr(7);
            }

            sm_assert(enumData.count(typeStr) != 0, "'match' may only be used on enums, not '" + typeStr + "'");
            vector<string> options = enumData[typeStr].order;

            map<string, bool> usedOptions;
            for (auto opt : enumData[typeStr].order)
            {
                usedOptions[opt] = false;
            }

            // Skips any vestigial "Del" statements
            int max = What.items[1].items.size();
            while (max >= 0 && What.items[1].items[max - 1].raw.substr(0, 3) == "Del")
            {
                max--;
            }

            for (int ind = 0; ind < max; ind++)
            {
                if (What.items[1].items[ind].raw == "case")
                {
                    auto &cur = What.items[1].items[ind];

                    string optionName = cur.items[0].raw;

                    sm_assert(enumData[typeStr].options.count(optionName) != 0,
                              "'" + optionName + "' is not an option for enum '" + typeStr + "'");
                    sm_assert(usedOptions.count(optionName) != 0,
                              "Option '" + optionName + "' cannot be used multiple times in a match statement.");
                    usedOptions.erase(optionName);

                    string captureName = cur.items[1].raw;

                    auto backupTable = table;

                    if (ind != 0)
                    {
                        out += "else ";
                    }

                    out += "if (" + itemStr + ".__info == " + typeStr + "_OPT_" + optionName + ")\n{";

                    if (captureName != "NULL")
                    {
                        int numPtrs = 0;

                        Type clone = enumData[typeStr].options[optionName];
                        while (clone.size() > 0 && clone[0].info == pointer)
                        {
                            numPtrs++;
                            clone.pop_front();
                        }

                        string captureType = mangleType(clone);

                        if (atomics.count(captureType) == 0)
                        {
                            out += "struct ";
                        }

                        for (int l = 0; l < numPtrs; l++)
                        {
                            captureType += "*";
                        }

                        out +=
                            captureType + " *" + captureName + " = &" + itemStr + ".__data." + optionName + "_data;\n";
                    }

                    // Add capture group to Oak table if needed

                    out += toC(cur.items[2]);
                    out += "\n}\n";
                }
                else if (What.items[1].items[ind].raw == "default")
                {
                    // Causes errors with destructor calls
                    // debugPrint(What.items[1]);

                    sm_assert(ind + 2 >= What.items[1].items.size(),
                              "Default statement must be final branch of match statement.");
                    usedOptions.clear();

                    auto &cur = What.items[1].items[ind];

                    string contents = toC(cur.items[0]);

                    if (ind == 0)
                    {
                        out += "{\n" + contents + "\n}\n";
                    }
                    else
                    {
                        out += "else\n{\n" + contents + "\n}\n";
                    }
                }
                else if (What.items[1].items[ind].raw != "")
                {
                    sm_assert(false, "Invalid option '" + What.items[1].items[ind].raw + "' in match statement.");
                }
            }

            if (usedOptions.size() != 0)
            {
                cout << tags::yellow_bold << "Warning! Match statement does not handle option(s) of enum '" << typeStr
                     << "':\n";

                for (auto opt : usedOptions)
                {
                    cout << '\t' << opt.first << '\n';
                }

                cout << tags::reset;
            }
        }
        else
        {
            cout << tags::yellow_bold << "Warning! Unknown enum keyword '" << What.raw
                 << "'. Treating as regular keyword.\n"
                 << tags::reset;

            out += What.raw + " ";
            for (auto child : What.items)
            {
                out += toC(child) + " ";
            }
        }

        break;
    }

    return out;
}

// Get the return type from a Type (of a function signature)
Type getReturnType(const Type &T)
{
    static map<unsigned long long, Type> getReturnTypeCache;

    if (getReturnTypeCache.count(T.ID) != 0)
    {
        return getReturnTypeCache[T.ID];
    }

    Type temp(T);

    int count = 0;
    for (int cur = 0; cur < temp.size(); cur++)
    {
        if (temp[cur].info == function)
        {
            count++;
        }

        if (temp[cur].info == maps)
        {
            count--;

            if (count == 0)
            {
                Type out(temp, cur + 1);

                getReturnTypeCache[T.ID] = out;

                return out;
            }
        }
    }

    if (getReturnTypeCache.size() > 1000)
    {
        getReturnTypeCache.clear();
    }

    getReturnTypeCache[T.ID] = T;

    return T;
}

vector<pair<string, Type>> getArgs(Type &type)
{
    static map<unsigned long long, vector<pair<string, Type>>> cache;

    // Check cache for existing value
    if (cache.count(type.ID) != 0)
    {
        return cache[type.ID];
    }

    // Get everything between final function and maps
    // For the case of function pointers, all variable names will be the unit string
    vector<pair<string, Type>> out;
    string curName = "";
    Type curType = nullType;
    int count = 0;

    int cur = 0;

    // Dereference
    while (cur < type.size() && type[cur].info == pointer)
    {
        cur++;
    }

    // Should now point to first 'function'

    // Do iteration
    while (cur < type.size())
    {
        if (type[cur].info == function)
        {
            count++;
        }
        else if (type[cur].info == maps)
        {
            count--;

            if (count == 0)
            {
                // Final append
                out.push_back(make_pair(curName, curType));
                break;
            }

            // Skip to the end of the return type
            int subCount = 1;
            curType.append(type[cur].info);

            cur++;

            while (cur < type.size())
            {
                if (type[cur].info == function)
                {
                    subCount++;
                }
                else if (type[cur].info == maps)
                {
                    subCount--;
                }

                if (subCount == 0)
                {
                    if (type[cur].info == maps || type[cur].info == join)
                    {
                        break;
                    }
                }

                curType.append(type[cur].info, type[cur].name);

                cur++;
            }

            continue;
        }

        if (count == 1 && type[cur].info == var_name)
        {
            curName = type[cur].name;
        }
        else if (count == 1 && type[cur].info == join)
        {
            out.push_back(make_pair(curName, curType));
            curName = "";
            curType = nullType;
        }
        else if (count != 1 || type[cur].info != function)
        {
            // Append to curType
            curType.append(type[cur].info, type[cur].name);
        }

        // Increment at end
        cur++;
    }

    // Copy into cache
    if (cache.size() > 1000)
    {
        cache.clear();
    }

    cache[type.ID] = out;

    // Return
    return out;
}

// This should only be called after method replacement
// I know I wrote this, but it still feels like black magic and I don't really understand it
Type resolveFunction(const vector<string> &What, int &start, string &c)
{
    while (start < What.size() && What[start].size() > 1 && What[start].substr(0, 2) == "//")
    {
        start++;
    }

    if (What.empty() || start >= What.size())
    {
        return nullType;
    }

    // Pointer check
    if (What[start] == "@")
    {
        start++;

        string name = What[start];

        string followingC = "";
        Type type = resolveFunction(What, start, followingC);

        if (type[0].info == function)
        {
            followingC = mangleSymb(name, mangleType(type));
        }

        type.prepend(pointer);

        c += "&" + followingC;

        return type;
    }
    else if (What[start] == "^")
    {
        start++;

        string followingC = "";
        Type type = resolveFunction(What, start, followingC);

        sm_assert(type[0].info == pointer, "Cannot dereference non-pointer.");
        type.pop_front();

        c += "*" + followingC;

        return type;
    }

    // Semicolon check
    else if (What[start] == ";")
    {
        c += ";";
        return nullType;
    }

    // Special symbol check
    else if (What[start].size() >= 2 && What[start].substr(0, 2) == "//")
    {
        start++;
        return resolveFunction(What, start, c);
    }

    // Standard case

    // get name (first item)
    string name = What[start];
    Type type = nullType;

    // Parenthesis
    if (What[start] == "(")
    {
        vector<string> toUse;
        int count = 0;
        do
        {
            if (What[start] == "(")
            {
                count++;
            }
            else if (What[start] == ")")
            {
                count--;
            }

            toUse.push_back(What[start]);

            start++;
        } while (start < What.size() && count != 0);

        if (!toUse.empty())
        {
            toUse.erase(toUse.begin());
            toUse.pop_back();
        }

        int trash = 0;
        Type toReturn = resolveFunction(toUse, trash, c);
        c = "(" + c + ")";

        return toReturn;
    }
    else
    {
        // Template instantiation
        bool didTemplate = false;
        if (What.size() > start + 1 && What[start + 1] == "<")
        {
            start++;

            vector<string> curGen;
            vector<vector<string>> generics;
            int count = 0;
            do
            {
                if (What[start] == "<")
                {
                    count++;

                    if (count > 1)
                    {
                        curGen.push_back(What[start]);
                    }
                }
                else if (What[start] == ">")
                {
                    count--;

                    if (count == 0)
                    {
                        if (curGen.size() > 0)
                        {
                            generics.push_back(curGen);
                        }

                        curGen.clear();
                    }
                    else
                    {
                        curGen.push_back(What[start]);
                    }
                }
                else if (What[start] == "," && count == 1)
                {
                    if (curGen.size() > 0)
                    {
                        generics.push_back(curGen);
                    }

                    curGen.clear();
                }
                else
                {
                    curGen.push_back(What[start]);
                }

                start++;
            } while (start < What.size() && count != 0);
            // start--;

            // should leave w/ what[start] == ';'
            vector<string> typeVec;

            while (start < What.size() && What[start] != ";")
            {
                typeVec.push_back(What[start]);
                start++;
            }

            if (typeVec.empty())
            {
                typeVec.push_back("struct");
            }

            string result = instantiateGeneric(name, generics, typeVec);

            start--;

            didTemplate = true;
        }

        if (What[start] == "size!")
        {
            // Case for type!() macro

            // Scrape entire type!(what) call to a vector
            vector<string> toAnalyze;
            int count = 0;

            start++;
            do
            {
                if (What[start] == "(")
                {
                    count++;
                }
                else if (What[start] == ")")
                {
                    count--;
                }

                if (!((What[start] == "(" && count == 1) || (What[start] == ")" && count == 0)))
                {
                    toAnalyze.push_back(What[start]);
                }

                start++;
            } while (count != 0 && start < What.size());

            // Garbage to feed to resolveFunction
            string junk = "";
            int pos = 0;

            // Analyze type of collected
            Type type = resolveFunction(toAnalyze, pos, junk);

            // Append size
            if (type[0].info == atomic)
            {
                if (atomics.count(type[0].name) != 0)
                {
                    c += to_string(atomics[type[0].name]);
                }
                else
                {
                    c += to_string(structData[type[0].name].size);
                }
            }
            else
            {
                c += to_string(sizeof(void *));
            }

            return Type(atomic, "u128");
        }
        else if (What[start].back() == '!')
        {
            // Otherwise unspecified macro: Void

            // Scrape entire type!(what) call to a vector
            list<string> toAnalyze = {What[start], "("};
            int count = 0;

            start++;
            do
            {
                if (What[start] == "(")
                {
                    count++;
                }
                else if (What[start] == ")")
                {
                    count--;
                }

                if (!((What[start] == "(" && count == 1) || (What[start] == ")" && count == 0)))
                {
                    toAnalyze.push_back(What[start]);
                }

                start++;
            } while (count != 0 && start < What.size());
            toAnalyze.push_back(")");

            // Analyze type of collected
            sequence s = __createSequence(toAnalyze);

            c += toC(s);

            return s.type;
        }

        // Function call
        if (What.size() > start + 1 && What[start + 1] == "(")
        {
            // get args within parenthesis
            vector<string> curArg;
            vector<vector<string>> args;

            int count = 0, templCount = 0;
            start++;
            do
            {
                if (What[start] == "(")
                {
                    count++;
                }
                else if (What[start] == ")")
                {
                    count--;
                }
                else if (What[start] == "<")
                {
                    templCount++;
                }
                else if (What[start] == ">")
                {
                    templCount--;
                }

                if (What[start] == "," && count == 1 && templCount == 0)
                {
                    args.push_back(curArg);
                    curArg.clear();
                }

                curArg.push_back(What[start]);

                start++;
            } while (start < What.size() && count != 0);

            if (!curArg.empty())
            {
                args.push_back(curArg);
            }

            // Erase commas, open parenthesis on first one
            for (int i = 0; i < args.size(); i++)
            {
                if (!args[i].empty())
                {
                    args[i].erase(args[i].begin());
                }
            }

            // Erase end parenthesis
            if (!args.empty() && !args.back().empty())
            {
                args.back().pop_back();
            }

            for (int i = 0; i < args.size(); i++)
            {
                if (args[i].size() == 0)
                {
                    args.erase(args.begin() + i);
                    i--;
                }
            }

            vector<Type> argTypes;
            vector<string> argStrs;
            for (vector<string> arg : args)
            {
                int trash = 0;
                string cur;

                argTypes.push_back(resolveFunction(arg, trash, cur));
                argStrs.push_back(cur);
            }

            // Special case; Pointer array access
            if (name == "Get" && argTypes.size() == 2 && argTypes[0][0].info == pointer && argTypes[0].size() > 1 &&
                argTypes[0][1].info == pointer)
            {
                c += argStrs[0] + "[" + argStrs[1] + "]";

                return Type(argTypes[0], 1);
            }

            // Special case; Pointer copy
            else if (name == "Copy" && argTypes.size() == 2 && argTypes[0].size() > 2 &&
                     argTypes[0][0].info == pointer && argTypes[0][1].info == pointer &&
                     argTypes[1][0].info == pointer && argTypes[1].size() > 1)
            {
                // Ensure equality; If not, throw error

                int left, right;
                left = 2;
                right = 1;

                // left = argTypes[0][0].next->next;
                // right = argTypes[1][0].next;

                bool isValid = true;
                while (left < argTypes[0].size() && right < argTypes[1].size())
                {
                    while (argTypes[0][left].info == var_name)
                    {
                        left++;
                    }

                    while (argTypes[1][right].info == var_name)
                    {
                        right++;
                    }

                    if (argTypes[0][left].info != argTypes[1][right].info ||
                        argTypes[0][left].name != argTypes[1][right].name)
                    {
                        isValid = false;
                        break;
                    }

                    left++;
                    right++;
                }

                sm_assert(isValid, "Cannot ptr copy '" + toStr(&argTypes[1]) + "' to '" + toStr(&argTypes[0]) + "'.");

                // Turn to C
                c += "(*" + argStrs[0] + ") = " + argStrs[1];

                // Return nullType (Copy yields void)
                return nullType;
            }

            // Search for candidates
            sm_assert(table.count(name) != 0, "Function call '" + name + "' has no registered symbols.");
            sm_assert(table[name].size() != 0, "Function call '" + name + "' has no registered symbols.");

            auto candidates = table[name];
            int indexOfExactMatch = -1;

            // Weed out any candidates which do not have the correct argument types
            for (int j = 0; j < candidates.size(); j++)
            {
                Type curType = candidates[j].type;
                while (curType != nullType && curType[0].info == pointer)
                {
                    curType.pop_front();
                }

                auto candArgs = getArgs(curType);
                for (int k = 0; k < candArgs.size(); k++)
                {
                    if (candArgs[k].second == nullType)
                    {
                        candArgs.erase(candArgs.begin() + k);
                        k--;
                    }
                }

                if (candArgs.size() != argTypes.size() || candidates[j].erased)
                {
                    candidates.erase(candidates.begin() + j);
                    j--;
                    continue;
                }

                bool isExactMatch = (indexOfExactMatch == -1);
                for (int k = 0; k < candArgs.size(); k++)
                {
                    // Check exactly
                    if (isExactMatch)
                    {
                        isExactMatch &= typesAreSameExact(&candArgs[k].second, &argTypes[k]);
                    }

                    // check inexactly
                    if (!typesAreSame(&candArgs[k].second, &argTypes[k]))
                    {
                        candidates.erase(candidates.begin() + j);
                        j--;
                        break;
                    }
                }

                if (isExactMatch)
                {
                    indexOfExactMatch = j;
                }
            }

            // If no candidates, throw error
            if (candidates.size() == 0)
            {
                // Get all theoretical candidates again
                candidates = table[name];

                cout << "\nCandidates:\n";
                for (auto c : candidates)
                {
                    cout << name << " has type " << toStr(&c.type) << '\n' << "Candidate arguments (";
                    auto candArgs = getArgs(c.type);
                    cout << candArgs.size() << "):\n";

                    for (auto arg : candArgs)
                    {
                        cout << "\t" << arg.first << "\t" << toStr(&arg.second) << '\n';
                    }

                    // Provide reason for elimination
                    if (candArgs.size() != argTypes.size())
                    {
                        cout << "\t(Too " << ((candArgs.size() < argTypes.size()) ? "few" : "many") << " arguments)\n";
                    }
                    else if (c.erased)
                    {
                        cout << "\t(Erased / private symbol)\n";
                    }
                    else
                    {
                        cout << "\t(Incompatible argument type(s))\n";
                    }
                }

                cout << "\nProvided arguments (" << argTypes.size() << "):\n(";
                for (auto a : argTypes)
                {
                    cout << toStr(&a) << ", ";
                }
                cout << ")\n\n";

                throw sequencing_error("Function call '" + name + "' has no viable candidates.");
            }

            // If multiple candidates, ensure all return types are the same
            else if (candidates.size() > 1)
            {
                type = getReturnType(candidates[0].type);

                for (int j = 1; j < candidates.size(); j++)
                {
                    if (getReturnType(candidates[j].type) != type)
                    {
                        throw sequencing_error("In function call '" + name +
                                               "': Cannot overload by return type alone.");
                    }
                }

                // Check for exact match

                if (indexOfExactMatch != -1)
                {
                    auto copy = candidates[indexOfExactMatch];

                    candidates.clear();
                    candidates.push_back(copy);
                }
                else
                {
                    sort(candidates.begin(), candidates.end(), sortCandidates);
                }

                // If has exact match, use it
                // Otherwise, use match with fewest differences in reference status
            }

            // Single candidate
            type = getReturnType(candidates[0].type);

            // do stuff here
            if (candidates[0].type[0].info == pointer)
            {
                c += name + "(";
            }
            else
            {
                c += mangleSymb(name, mangleType(candidates[0].type)) + "(";
            }

            for (int j = 0; j < argStrs.size(); j++)
            {
                if (j != 0)
                {
                    c += ", ";
                }

                // do referencing stuff here for each argument

                int numDeref = 0;
                // determine actual number here

                auto candArgs = getArgs(candidates[0].type);

                // Type *candCursor = &candArgs[j].second;
                // Type *argCursor = &argTypes[j];

                int candCursor = 0, argCursor = 0;

                while (argCursor < argTypes[j].size() && argTypes[j][argCursor].info == pointer)
                {
                    argCursor++;
                    numDeref++;
                }

                while (candCursor < candArgs[j].second.size() && candArgs[j].second[candCursor].info == pointer)
                {
                    candCursor++;
                    numDeref--;
                }

                if (numDeref > 0)
                {
                    for (int k = 0; k < numDeref; k++)
                    {
                        c.append("*");
                    }
                }
                else if (numDeref == -1)
                {
                    for (int k = 0; k < -numDeref; k++)
                    {
                        c.append("&");
                    }
                }
                else if (numDeref != 0)
                {
                    throw sequencing_error("Illegal multiple automatic referencing in function call '" + name +
                                           "' (only auto-deref or single auto-ref is allowed).");
                }

                c += "(" + argStrs[j] + ")";
            }
            c += ")";

            start--;
        }

        else if (!didTemplate)
        {
            // Non-function-call

            // Literal check
            Type litType = checkLiteral(What[start]);
            if (litType == nullType)
            {
                // Is not a literal
                sm_assert(table.count(What[start]) != 0, "No definitions exist for symbol '" + What[start] + "'.");
                auto candidates = table[What[start]];
                sm_assert(candidates.size() != 0, "No definitions exist for symbol '" + What[start] + "'.");

                type = candidates.back().type;
            }
            else
            {
                // Is a literal
                type = litType;
            }

            c += What[start];
        }

        // if member access, resolve that
        while (start + 1 < What.size() && What[start + 1] == ".")
        {
            // Auto-dereference pointers (. to -> automatically)
            if (type[0].info == pointer)
            {
                while (type[0].info == pointer)
                {
                    type.pop_front();

                    c = "*" + c;
                }

                c = "(" + c + ")";
            }

            string structName;
            try
            {
                sm_assert(type[0].info == atomic,
                          "Error during type trimming for member access; Could not find struct name.");
                structName = type[0].name;

                sm_assert(structData.count(structName) != 0, "Struct type '" + structName + "' does not exist.");
                sm_assert(!structData[structName].erased,
                          "Struct '" + structName + "' exists, but is erased (private).");
                sm_assert(structData[structName].members.count(What[start + 2]) != 0,
                          "Struct '" + structName + "' has no member '" + What[start + 2] + "'.");
            }
            catch (sequencing_error &e)
            {
                cout << tags::yellow_bold << "In context: '";

                int pos = start - 8;
                if (pos < 0)
                {
                    pos = 0;
                }

                for (; pos < What.size() && pos < start + 8; pos++)
                {
                    cout << What[pos] << ' ';
                }

                cout << "'\n" << tags::reset;

                throw e;
            }

            type = structData[structName].members[What[start + 2]];

            c += "." + What[start + 2];

            start += 2;
        }

        start++;

        // Return function return type
        return type;
    }

    // unreachable
}

/////////////////////////////////////////

// Can throw errors (IE malformed definitions)
void addEnum(const vector<string> &FromIn)
{
    vector<string> From;

    // Clean input of any special symbols
    for (auto f : FromIn)
    {
        if (f.size() < 2 || f.substr(0, 2) != "//")
        {
            From.push_back(f);
        }
    }

    // Assert the expression can be properly-formed
    parse_assert(From.size() >= 4);

    // Get name and check against malformations
    int i = 0;

    parse_assert(From[i] == "let");
    i++;

    string name = From[i];
    i++;

    parse_assert(i < From.size());

    // Scrape generics here (and mangle)
    vector<string> curGen;
    vector<vector<string>> generics;

    int count = 0;
    while (i < From.size() && From[i] != ":" && From[i] != "{")
    {
        if (From[i] == "<")
        {
            count++;

            if (count == 1)
            {
                i++;
                continue;
            }
        }

        else if (From[i] == ">")
        {
            count--;

            if (count == 0)
            {
                generics.push_back(curGen);
            }
        }

        else if (From[i] == "<")
        {
            count++;
        }

        curGen.push_back(From[i]);
        i++;
    }

    if (generics.size() != 0)
    {
        name = mangleStruct(name, generics);
    }

    if (enumData.count(name) != 0 || structData.count(name) != 0)
    {
        cout << tags::yellow_bold << "Warning! Definition of enum '" << name << "' erases struct of the same name.\n"
             << tags::reset;
    }

    // Ensure for unit enums
    structOrder.push_back(name);

    // Auto-create unit New and Del
    Type t;
    t.append(function);
    t.append(var_name, "what");
    t.append(pointer);
    t.append(atomic, name);
    t.append(maps);
    t.append(atomic, "void");

    sequence s;
    s.info = code_scope;
    s.type = Type(atomic, "void");
    s.items.push_back(sequence{});
    s.items.back().info = atom;
    s.items.back().raw = "//AUTOGEN";

    // Ensure these keys exist
    table["New"];
    table["Del"];

    table["New"].push_back(__multiTableSymbol{s, t, false, curFile});
    table["Del"].push_back(__multiTableSymbol{s, t, false, curFile});

    parse_assert(From[i] == ":");
    i++;
    parse_assert(From[i] == "enum");
    i++;

    if (From[i] == "{")
    {
        i++;
        for (; i < From.size() && From[i] != "}"; i++)
        {
            // name : type ,
            // name , name2 , name3 : type < string , hi > , name4 : type2 ,
            vector<string> names, lexedType;

            while (i + 1 < From.size() && From[i + 1] == ",")
            {
                names.push_back(From[i]);

                i += 2;
            }

            names.push_back(From[i]);

            parse_assert(i + 1 < From.size());
            parse_assert(From[i + 1] == ":");

            i += 2;

            // Get lexed type (can be multiple symbols due to templating)
            int templCount = 0;
            vector<string> genericHolder;

            while (i < From.size() && !(templCount == 0 && From[i] == ","))
            {
                if (templCount == 0 && From[i] != "<")
                {
                    lexedType.push_back(From[i]);
                }
                else
                {
                    genericHolder.push_back(From[i]);
                }

                if (From[i] == "<")
                {
                    templCount++;
                }
                else if (From[i] == ">")
                {
                    templCount--;

                    if (templCount == 0)
                    {
                        string toAdd = mangle(genericHolder);
                        genericHolder.clear();

                        if (toAdd != "")
                        {
                            lexedType.back().append("_" + toAdd);
                        }
                    }
                }

                i++;
            }

            Type toAdd = toType(lexedType);
            for (string varName : names)
            {
                enumData[name].options[varName] = toAdd;
                enumData[name].order.push_back(varName);
            }
        }
    }
    else if (From[4] != ";")
    {
        throw parse_error("Malformed enum definition; Expected ';' or '{'.");
    }

    enumData[name].size = 0;
    __enumLookupData &cur = enumData[name];
    string enumTypeStr = name;
    unsigned long long curSize;
    for (auto optionName : cur.order)
    {
        // Sizing
        if (cur.options[optionName][0].info == atomic)
        {
            if (structData.count(cur.options[optionName][0].name) != 0)
            {
                curSize = structData[cur.options[optionName][0].name].size;
            }
            else
            {
                curSize = 1;
            }
        }
        else
        {
            curSize = sizeof(void *);
        }

        if (curSize > cur.size)
        {
            cur.size = curSize;
        }

        if (cur.options[optionName][0].info == atomic && cur.options[optionName][0].name == "unit")
        {
            // Unit struct; Single argument constructor

            // Insert Oak version
            Type constructorType = nullType;
            constructorType.append(function);
            constructorType.append(var_name, "self");
            constructorType.append(pointer);
            constructorType.append(atomic, enumTypeStr);
            constructorType.append(maps);
            constructorType.append(atomic, "void");

            table["wrap_" + optionName].push_back(__multiTableSymbol{sequence{}, constructorType, false, curFile});
        }
        else
        {
            // Double argument constructor

            // Insert Oak version
            Type constructorType = nullType;
            constructorType.append(function);
            constructorType.append(var_name, "self");
            constructorType.append(pointer);
            constructorType.append(atomic, enumTypeStr);
            constructorType.append(join);
            constructorType.append(var_name, "data");
            constructorType.append(cur.options[optionName]);
            constructorType.append(maps);
            constructorType.append(atomic, "void");

            table["wrap_" + optionName].push_back(__multiTableSymbol{sequence{}, constructorType, false, curFile});
        }
    }

    cur.size += enumSize;

    return;
}

// Dump data to file
void dump(const vector<string> &Lexed, const string &Where, const string &FileName, const int &Line,
          const sequence &FileSeq, const vector<string> LexedBackup)
{
    string sep = "";
    for (int i = 0; i < 50; i++)
    {
        sep += '-';
    }
    sep += '\n';

    ofstream file(Where);
    if (!file.is_open())
    {
        throw runtime_error("Failed to open dump file '" + Where + "'; At this point, you should give up.");
    }

    auto curTime = time(NULL);

    file << sep << "Dump file for " << FileName << " generated by Acorn at " << ctime(&curTime);

    file << sep << "// Pre-everything lexed:\n";

    for (auto s : LexedBackup)
    {
        if (s.size() >= 2 && s.substr(0, 2) == "//")
        {
            file << "\n" << s << "\t|\t";
        }
        else
        {
            file << s << ' ';
        }
    }
    file << '\n';

    file << sep << "// Post-substitution lexed:\n";

    for (auto s : Lexed)
    {
        if (s.size() >= 2 && s.substr(0, 2) == "//")
        {
            file << "\n" << s << "\t|\t";
        }
        else
        {
            file << s << ' ';
        }
    }
    file << '\n';

    file << sep << "// Symbols and their types:\n";

    for (auto p : table)
    {
        file << p.first << ":\n";

        for (auto item : p.second)
        {
            file << '\t' << toStr(&item.type) << '\n';
        }
    }

    file << sep << "// Structs:\n";

    for (auto s : structData)
    {
        file << s.first << "\n";

        for (auto m : s.second.members)
        {
            file << '\t' << m.first << '\t' << toStr(&m.second) << '\n';
        }
    }

    file << sep << "// Enums:\n";

    for (auto e : enumData)
    {
        file << e.first << "\n";

        for (auto m : e.second.options)
        {
            file << '\t' << m.first << '\t' << toStr(&m.second) << '\n';
        }
    }

    file << sep << "// Generics:\n";

    printGenericDumpInfo(file);

    file << sep << "// Full anatomy:\n";

    debugPrint(FileSeq, 0, file);

    file << sep << "// All rules:\n";

    for (auto s : rules)
    {
        file << s.first << '\n' << '\t';

        for (auto t : s.second.inputPattern)
        {
            file << t << ' ';
        }

        file << "\n\t";

        for (auto t : s.second.outputPattern)
        {
            file << t << ' ';
        }

        file << "\n";
    }

    file << sep << "// All bundles:\n";

    for (auto p : bundles)
    {
        file << p.first << "\n\t";

        for (auto r : p.second)
        {
            file << r << ' ';
        }

        file << '\n';
    }

    file << sep << "// Active rules:\n";

    for (auto s : activeRules)
    {
        file << s << '\n';
    }

    file.close();
    return;
}
