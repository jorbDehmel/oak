/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "sequence.hpp"
#include "sequence_resources.hpp"
#include "symbol_table.hpp"
#include "tags.hpp"

// Activates "dirty" mode, where mem alloc and free are allowed
bool insideMethod = false;

vector<string> curLineSymbols;

unsigned long long int curLine = 1;
string curFile = "";

// The current depth of createSequence
unsigned long long int depth = 0;

// Internal consumptive version: Erases from vector, so not safe for frontend
sequence __createSequence(list<string> &From);

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
            cout << curFile << ":" << curLine << ":c_print! ";

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
                    if (!(count == 1 && From.front() == "(") && !(count == 0 && From.front() == ")") &&
                        From.front() != ",")
                    {
                        string cur = From.front();
                        cout << cleanMacroArgument(cur) << " ";
                    }
                }

                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.pop_front();

            } while (!From.empty() && count != 0);

            cout << "\n";

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
                    if (!(count == 1 && From.front() == "(") && !(count == 0 && From.front() == ")") &&
                        From.front() != ",")
                    {
                        message += cleanMacroArgument(From.front()) + " ";
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
                    if (!(count == 1 && From.front() == "(") && !(count == 0 && From.front() == ")") &&
                        From.front() != ",")
                    {
                        if (command != "")
                        {
                            command += " && ";
                        }

                        command += cleanMacroArgument(From.front());
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

            sm_assert(tempType.size() > 0, "'alloc!' received a malformed first argument.");

            temp.type.pop_front();

            if (numType == nullType)
            {
                num = "1";
                numType = typeNode{atomic, "u128"};
            }

            sm_assert(tempType.size() > 0 && (tempType[0].info == arr || (tempType[0].info == pointer && num == "1")),
                      "'alloc!' returns an unsized array, or pointer if no number was provided.");

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
            sm_assert(tempType[0].info == arr || tempType[0].info == pointer,
                      "'free!' takes an unsized array or pointer.");
            temp.type.pop_front();

            out = getFreeSequence(name, false);

            return out;
        }
        else if (From.front() == "ptrcpy!")
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

            sm_assert(lhsSeq.type[0].info == pointer || lhsSeq.type[0].info == arr,
                      "First argument of ptrcpy! must be pointer or unsized array.");
            sm_assert(rhsSeq.type[0].info == pointer || rhsSeq.raw == "0",
                      "Second argument of ptrcpy! must be a pointer or 0.");

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

            sm_assert(lhsSeq.type[0].info == pointer || lhsSeq.type[0].info == arr || lhsSeq.type[0].info == sarr,
                      "First argument of ptrarr! must be pointer or arr.");
            sm_assert(rhsSeq.type.size() == 1 && rhsSeq.type[0].info == atomic,
                      "second argument of ptrarr! must be an atomic.");

            string t = rhsSeq.type[0].name;
            if (t != "i8" && t != "u8" && t != "i16" && t != "u16" && t != "i32" && t != "u32" && t != "i64" &&
                t != "u64" && t != "i128" && t != "u128")
            {
                throw sequencing_error("Second argument of ptrarr! must be an integer, not " + toStr(&rhsSeq.type));
            }

            out.info = atom;
            out.type = Type(lhsSeq.type, 1);
            out.raw = "(" + lhs + "[" + rhs + "])";

            return out;
        }
    } // Macros

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

    // Enums- VERY C dependant!
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

        if (out.items.back().info == atom)
        {
            out.items.back().raw.push_back(';');
        }
        else if (out.items.back().info == code_line)
        {
            out.items.back().items.push_back(sequence{nullType, vector<sequence>(), atom, ";"});
        }

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
    else if (From.front() == "return")
    {
        // Takes a code scope / code line
        out.info = code_line;
        out.type = nullType;
        out.items.push_back(sequence{nullType, vector<sequence>(), atom, "return"});

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

                // Call constructor (pointers do not get constructors)
                if (type[0].info != pointer && type[0].info != arr && type[0].info != sarr)
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
                else if (type[0].info != sarr)
                {
                    // Syntactically necessary
                    out.items.push_back(sequence{nullType, vector<sequence>(), atom, ";"});

                    sequence toAppend;
                    toAppend.info = atom;
                    toAppend.type = nullType;
                    toAppend.raw = name + " = 0";

                    out.items.push_back(toAppend);
                }
                else
                {
                    // Initialize sized array
                    // Set every byte inside to zero

                    // Syntactically necessary
                    out.items.push_back(sequence{nullType, vector<sequence>(), atom, ";"});

                    sequence toAppend;
                    toAppend.info = atom;
                    toAppend.type = nullType;

                    toAppend.raw = "for (i32 _i = 0; _i < sizeof(" + name + "); _i++) ((u8 *)(&" + name + "))[_i] = 0;";

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

                Type returnType = getReturnType(type);

                // Restrictions upon some types of methods
                if (name == "New" || name == "Del")
                {
                    if (returnType != Type(atomic, "void"))
                    {
                        throw runtime_error("Illegal method definition! Method '" + name + "' must return void.");
                    }

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
                        sm_assert(returnType[0].info == atomic &&
                                      (returnType[0].name == "i32" || returnType[0].name == "void"),
                                  "Function 'main' must return i32.");

                        if (argsWithType.size() != 0)
                        {
                            sm_assert(argsWithType.size() == 2, "'main' must have either 0 or 2 arguments.");
                            sm_assert(argsWithType[0].second == Type(atomic, "i32"),
                                      "First argument of 'main' must be i32.");
                        }
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
                            // Add semicolon
                            table["New"].back().seq.items.push_back(sequence{nullType, vector<sequence>(), atom, ";"});

                            sequence toAppend;
                            toAppend.info = atom;
                            toAppend.type = nullType;
                            toAppend.raw = getMemberNew("(*" + argsWithType[0].first + ")", var.first, var.second);

                            table["New"].back().seq.items.push_back(toAppend);
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
                            // Add semicolon
                            table["Del"].back().seq.items.push_back(sequence{nullType, vector<sequence>(), atom, ";"});

                            sequence toAppend;
                            toAppend.info = atom;
                            toAppend.type = nullType;
                            toAppend.raw = getMemberDel("(*" + argsWithType[0].first + ")", var.first, var.second);

                            table["Del"].back().seq.items.push_back(toAppend);
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
    } // let

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
    } // scope

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

// This should only be called after method replacement
// I know I wrote this, but it still feels like black magic and I don't really understand it
Type resolveFunctionInternal(const vector<string> &What, int &start, vector<string> &c)
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
        c.push_back("&");

        string name = What[start];

        string followingC = "";
        Type type = resolveFunction(What, start, followingC);

        if (type[0].info == function)
        {
            followingC = mangleSymb(name, mangleType(type));
        }

        type.prepend(pointer);
        c.push_back(followingC);

        return type;
    }
    else if (What[start] == "^")
    {
        start++;

        c.push_back("*");

        Type type = resolveFunctionInternal(What, start, c);

        sm_assert(type[0].info == pointer, "Cannot dereference non-pointer.");
        type.pop_front();

        return type;
    }

    // Semicolon check
    else if (What[start] == ";")
    {
        c.push_back(";");
        return nullType;
    }

    // Special symbol check
    else if (What[start].size() >= 2 && What[start].substr(0, 2) == "//")
    {
        start++;
        return resolveFunctionInternal(What, start, c);
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
        Type toReturn = resolveFunctionInternal(toUse, trash, c);

        // c = "(" + c + ")";
        // Costly:
        // c.insert(c.begin(), "(");

        // Less costly:
        c.front().insert(c.front().begin(), '(');
        c.push_back(")");

        return toReturn;
    }

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

    // Append literal C code, without interpretation from Oak
    if (What[start] == "raw_c!")
    {
        // Scrape call to a vector
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
                // Append raw C code
                c.push_back(cleanMacroArgument(What[start]));
                c.push_back(" ");
            }

            start++;
        } while (count != 0 && start < What.size());

        return Type(atomic, "void");
    }

    else if (What[start] == "size!")
    {
        // Case for size!() macro

        // Scrape entire size!(what) call to a vector
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

        int multiplier = 1;
        while (type[0].info == sarr)
        {
            multiplier *= stoi(type[0].name);
            type.pop_front();
        }

        // Append size
        if (type[0].info == atomic)
        {
            if (atomics.count(type[0].name) != 0)
            {
                c.push_back(to_string(atomics[type[0].name] * multiplier));
            }
            else
            {
                c.push_back(to_string(structData[type[0].name].size * multiplier));
            }
        }
        else
        {
            c.push_back(to_string(sizeof(void *) * multiplier));
        }

        return Type(atomic, "u128");
    }

    else if (What[start].back() == '!')
    {
        // Otherwise unspecified macro

        // Scrape entire call to a vector
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

        c.push_back(toC(s));

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

        // Special case: Array access
        if (name == "Get" && (argTypes[0][0].info == sarr || argTypes[0][0].info == arr) && argStrs.size() == 2 &&
            argTypes[1][0].info == atomic &&
            (argTypes[1][0].name == "u8" || argTypes[1][0].name == "i8" || argTypes[1][0].name == "u16" ||
             argTypes[1][0].name == "i16" || argTypes[1][0].name == "u32" || argTypes[1][0].name == "i32" ||
             argTypes[1][0].name == "u64" || argTypes[1][0].name == "i64" || argTypes[1][0].name == "u128" ||
             argTypes[1][0].name == "i128"))
        {
            // Return type is pointer to the thing the array is of
            type = argTypes[0];
            type[0].info = pointer;
            c.push_back("(" + argStrs[0] + "+" + argStrs[1] + ")");
        }
        else
        {
            // Search for candidates
            sm_assert(table.count(name) != 0, "Function call '" + name + "' has no registered symbols.");
            sm_assert(table[name].size() != 0, "Function call '" + name + "' has no registered symbols.");

            vector<__multiTableSymbol> candidates = table[name];

            // Construct candArgs
            vector<vector<Type>> candArgs;
            for (auto item : candidates)
            {
                Type curType = item.type;
                while (curType != nullType && curType[0].info == pointer)
                {
                    curType.pop_front();
                }

                auto args = getArgs(item.type);
                candArgs.push_back(vector<Type>());

                for (auto arg : args)
                {
                    if (arg.second == nullType)
                    {
                        continue;
                    }

                    candArgs.back().push_back(arg.second);
                }
            }

            vector<int> validCandidates;

            // Do stages of candidacy
            validCandidates = getStageOneCandidates(candArgs, argTypes);

            if (validCandidates.size() == 0)
            {
                validCandidates = getStageThreeCandidates(candArgs, argTypes);

                if (validCandidates.size() != 1)
                {
                    validCandidates = getStageTwoCandidates(candArgs, argTypes);
                }
            }

            // Remove any erased symbols
            for (auto item = validCandidates.begin(); item != validCandidates.end(); item++)
            {
                if (candidates[*item].erased)
                {
                    item = validCandidates.erase(item);
                }
            }

            // Error checking
            if (validCandidates.size() == 0)
            {
                // No viable candidates

                cout << tags::red_bold << "Error: No viable candidates for function call '" << name << "'.\n"
                     << tags::reset;

                printCandidateErrors(candidates, argTypes, name);

                throw sequencing_error("No viable candidates for function call '" + name + "'");
            }
            else if (validCandidates.size() > 1)
            {
                // Ambiguous candidates

                cout << tags::red_bold << "Error: Cannot override function call '" << name
                     << "' on return type alone.\n"
                     << tags::reset;

                printCandidateErrors(candidates, argTypes, name);

                throw sequencing_error("Cannot override function call '" + name + "' on return type alone");
            }

            // Use candidate at front; This is highest-priority one
            type = getReturnType(candidates[validCandidates[0]].type);

            if (candidates[validCandidates[0]].type[0].info == pointer)
            {
                c.push_back(name);
                c.push_back("(");
            }
            else
            {
                c.push_back(mangleSymb(name, mangleType(candidates[validCandidates[0]].type)));
                c.push_back("(");
            }

            for (int j = 0; j < argStrs.size(); j++)
            {
                if (j != 0)
                {
                    c.push_back(", ");
                }

                // do referencing stuff here for each argument

                int numDeref = 0;
                // determine actual number here

                auto candArgs = getArgs(candidates[validCandidates[0]].type);

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
                        c.push_back("*");
                    }
                }
                else if (numDeref == -1)
                {
                    for (int k = 0; k < -numDeref; k++)
                    {
                        c.push_back("&");
                    }
                }
                else if (numDeref != 0)
                {
                    cout << "With candidates:\n";
                    for (auto item : candidates)
                    {
                        cout << left << name << setw(40) << toStr(&item.type) << "from " << item.sourceFilePath << '\n';
                    }
                    cout << '\n';

                    throw sequencing_error("Illegal multiple automatic referencing in function call '" + name +
                                           "' (only auto-deref or single auto-ref is allowed).");
                }

                c.push_back("(");
                c.push_back(argStrs[j]);
                c.push_back(")");
            }
            c.push_back(")");
        }

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

        c.push_back(What[start]);
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

                c.front() = "*" + c.front();
            }

            c.front() = "(" + c.front();
            c.push_back(")");
        }

        string structName;
        try
        {
            sm_assert(type[0].info == atomic,
                      "Error during type trimming for member access; Could not find struct name.");
            structName = type[0].name;

            sm_assert(structData.count(structName) != 0, "Struct type '" + structName + "' does not exist.");
            sm_assert(!structData[structName].erased, "Struct '" + structName + "' exists, but is erased (private).");
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

        c.push_back(".");
        c.push_back(What[start + 2]);

        start += 2;
    }

    start++;

    // Return function return type
    return type;
}

Type resolveFunction(const vector<string> &What, int &start, string &c)
{
    vector<string> cVec;

    if (c != "")
    {
        cVec.push_back(c);
    }

    Type out = resolveFunctionInternal(What, start, cVec);

    int size = 0;
    for (const auto &item : cVec)
    {
        size += item.size();
    }
    c.clear();
    c.reserve(size);

    for (const auto &item : cVec)
    {
        if (item.size() == 0)
        {
            continue;
        }

        c += item;
    }

    return out;
}
