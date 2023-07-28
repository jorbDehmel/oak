/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "sequence.hpp"

// Activates "dirty" mode, where mem alloc and free are allowed
bool insideMethod = false;

unsigned long long int curLine = 1;
string curFile = "";

// Any string not in this set is a symbol name
const set<string> specials = {
    "+", "-", "*", "/", "%", "&", "|", "<", ">", "<=", ">=", "==",
    "!=", "!", "^", "&&", "(", ")", "{", "}", "[", "]", "||", "&&",
    "~", ":", ";", "->", "&", "<<", ">>", "+=", "-=", "*=", "/=",
    "&=", "|=", "=", "let", "for", "struct", "if", "else", "while",
    "catch", "try", "switch", "case"};

// Actually used in dump files, not just for debugging.
void debugPrint(const sequence &What, int spaces, ostream &to)
{
    for (int i = 0; i < spaces; i++)
    {
        to << "| ";
    }

    switch (What.info)
    {
    case comma_sep:
        to << "comma_sep:";
        break;
    case function_call:
        to << toStr(&What.type) << " function_call";
        break;
    case parenthesis:
        to << "parenthesis";
        break;
    case code_scope:
        to << "code_scope";
        break;
    case code_line:
        to << "code_line";
        break;
    case for_triple:
        to << "for_triple";
        break;
    case access:
        to << "access";
        break;
    case atom:
        to << "atom";
        break;
    case keyword:
        to << "keyword";
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
    vector<string> temp(From);
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
sequence __createSequence(vector<string> &From)
{
    sequence out;
    out.info = code_line;

    // Misc. Invalid cases (common)
    if (From.size() == 0)
    {
        return out;
    }
    else if (From[0].size() > 11 && From[0].substr(0, 11) == "//__LINE__=")
    {
        // Line update special symbol
        string newLineNum = From[0].substr(11);

        curLine = stoull(newLineNum);

        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.erase(From.begin());
        return __createSequence(From);
    }

    // Memory Keywords
    else if (From[0] == "alloc!")
    {
        sm_assert(insideMethod, "Memory cannot be allocated outside of an operator-alias method.");

        int count = 0;
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.erase(From.begin());
        vector<string> contents;

        do
        {
            if (From[0] == "(")
            {
                count++;
            }
            else if (From[0] == ")")
            {
                count--;
            }

            if (!(count == 1 && From[0] == "(") && !(count == 0 && From[0] == ")"))
            {
                contents.push_back(From[0]);
            }

            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.erase(From.begin());
        } while (!From.empty() && count != 0);

        int num = 1;
        for (int j = 0; j < contents.size(); j++)
        {
            if (contents[j] == ",")
            {
                num = stoi(contents[j + 1]);

                while (j + 1 < contents.size())
                {
                    contents.erase(contents.begin() + (j + 1));
                }

                break;
            }
        }

        sequence temp = __createSequence(contents);
        string name = toC(temp);

        Type tempType = temp.type;
        sm_assert(tempType.info == pointer, "'alloc!' returns a pointer.");
        temp.type = *tempType.next;

        out = getAllocSequence(temp.type, name, num);

        return out;
    }
    else if (From[0] == "free!")
    {
        sm_assert(insideMethod, "Memory cannot be deleted outside of an operator-alias method.");

        int count = 0;
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.erase(From.begin());
        vector<string> contents;

        do
        {
            if (From[0] == "(")
            {
                count++;
            }
            else if (From[0] == ")")
            {
                count--;
            }

            if (!(count == 1 && From[0] == "(") && !(count == 0 && From[0] == ")"))
            {
                contents.push_back(From[0]);
            }

            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.erase(From.begin());
        } while (!From.empty() && count != 0);

        sequence temp = __createSequence(contents);
        string name = toC(temp);

        Type tempType = temp.type;
        sm_assert(tempType.info == pointer, "'alloc!' returns a pointer.");
        temp.type = *tempType.next;

        out = getFreeSequence(name, false);

        return out;
    }
    else if (From[0] == "free_arr!")
    {
        sm_assert(insideMethod, "Memory cannot be deleted outside of an operator-alias method.");

        int count = 0;
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.erase(From.begin());
        vector<string> contents;

        do
        {
            if (From[0] == "(")
            {
                count++;
            }
            else if (From[0] == ")")
            {
                count--;
            }

            if (!(count == 1 && From[0] == "(") && !(count == 0 && From[0] == ")"))
            {
                contents.push_back(From[0]);
            }

            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.erase(From.begin());
        } while (!From.empty() && count != 0);

        sequence temp = __createSequence(contents);
        string name = toC(temp);

        Type tempType = temp.type;
        sm_assert(tempType.info == pointer, "'alloc!' returns a pointer.");
        temp.type = *tempType.next;

        out = getFreeSequence(name, true);

        return out;
    }

    // Misc key-characters
    else if (From[0] == ";")
    {
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.erase(From.begin());
        auto toReturn = __createSequence(From);
        toReturn.type = nullType;

        return toReturn;
    }

    // Referencing and dereferencing operators
    else if (From[0] == "^")
    {
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.erase(From.begin());
        auto toReturn = __createSequence(From);
        sm_assert(toReturn.type.info == pointer, "Cannot dereference non-pointer.");

        Type temp = *toReturn.type.next;
        toReturn.type = temp;

        return toReturn;
    }
    else if (From[0] == "@")
    {
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.erase(From.begin());
        auto toReturn = __createSequence(From);

        toReturn.type.prepend(pointer);

        return toReturn;
    }

    // Keywords
    else if (From[0] == "for")
    {
        // Takes a for_triple and a code scope / code line
        sm_assert(false, "UNIMPLEMENTED");

        return out;
    }
    else if (From[0] == "if" || From[0] == "while")
    {
        // Takes a bool code line and a code scope / code line
        out.info = keyword;
        out.raw = From[0];
        out.type = nullType;

        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.erase(From.begin());

        // Pops the first full sequence from the remaining front
        // This is for the conditional
        out.items.push_back(__createSequence(From));

        sm_assert(out.items[0].type == Type(atomic, "bool"), out.raw + " statement argument must be boolean. Instead, '" + toStr(&out.items[0].type) + "'");
        sm_assert(!From.empty(), "Missing statement after " + out.raw + "");

        // This is for the code chunk
        out.items.push_back(__createSequence(From));

        return out;
    }
    else if (From[0] == "else")
    {
        // Takes a code scope / code line
        out.info = keyword;
        out.raw = From[0];
        out.type = nullType;

        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.erase(From.begin());

        out.items.push_back(__createSequence(From));

        return out;
    }
    else if (From[0] == "let")
    {
        // Get name
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.erase(From.begin()); // let
        sm_assert(!From.empty(), "'let' must be followed by something.");
        string name = From[0];
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.erase(From.begin());

        // Get type
        if (From[0] == ":")
        {
            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.erase(From.begin());
            if (!From.empty() && From.front() == "struct")
            {
                // addStruct takes in the full struct definition, from
                // let to end curly bracket. So we must first parse this.

                vector<string> toAdd = {"let", name, ":"};

                int count = 0;
                while (count != 0 || (From[0] != "}" && From[0] != ";"))
                {
                    toAdd.push_back(From[0]);
                    sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                    From.erase(From.begin());
                }
                toAdd.push_back(From[0]);
                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.erase(From.begin());

                // addStruct
                addStruct(toAdd);

                // This should be left out of toC, as it should only be used
                // in the header file during reconstruction.
            }
            else
            {
                // Non-struct definition; Local var, not function
                // NOTE: let a: i32 = 5; is NO LONGER supported!

                // Scrape entire definition for this
                vector<string> toAdd = {
                    "let",
                    name,
                    ":",
                };

                while (!From.empty() && From.front() != ";")
                {
                    toAdd.push_back(From.front());
                    sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                    From.erase(From.begin());

                    if (!From.empty() && From.front() == "=")
                    {
                        throw sequencing_error("Instantiate / assignment combo is not supported.");
                    }
                }

                // This SHOULD appear during toC.
                out.info = comma_sep;
                out.raw = "";
                out.type = nullType;
                out.items.clear();

                Type type = toType(toAdd);

                out.items.push_back(sequence{nullType, vector<sequence>(), atom, toStrC(&type)});
                out.items.push_back(sequence{nullType, vector<sequence>(), atom, name});

                // Insert into table
                table[name].push_back(__multiTableSymbol{sequence{type, vector<sequence>(), atom, ""}, type});

                // Call constructor (pointers do not get constructors)
                if (type.info != pointer)
                {
                    // Syntactically necessary
                    out.items.push_back(sequence{nullType, vector<sequence>(), atom, ";"});

                    vector<string> newCall = {"New", "(", "@", name, ")"};
                    int garbage = 0;

                    sequence toAppend;
                    toAppend.info = atom;
                    toAppend.type = resolveFunction(newCall, garbage, toAppend.raw);

                    out.items.push_back(toAppend);

                    // out.items.push_back(sequence{nullType, vector<sequence>(), atom, "; New(&" + name + ")"});
                }

                return out;
            }
        }
        else if (From[0] == "<" || From[0] == "(")
        {
            // Function definition

            // This should NOT appear during toC, as it is only
            // present during reconstruction.

            // Templating and arguments
            vector<string> toAdd;
            do
            {
                toAdd.push_back(From.front());
                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.erase(From.begin());
            } while (From[0] != "{" && From[0] != ";");

            auto type = toType(toAdd);

            auto argsWithType = getArgs(type);
            for (pair<string, Type> p : argsWithType)
            {
                table[p.first].push_back(__multiTableSymbol{sequence(), p.second});
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

            bool oldSafeness = insideMethod;
            insideMethod = isMethod;

            // Insert symbol
            if (From.front() == "{")
            {
                addSymb(name, toAdd, __createSequence(From));
            }
            else
            {
                addSymb(name, toAdd, sequence{});
            }

            insideMethod = oldSafeness;

            for (pair<string, Type> p : argsWithType)
            {
                for (int k = 0; k < table[p.first].size(); k++)
                {
                    if (table[p.first][k].type == p.second)
                    {
                        table[p.first].erase(table[p.first].begin() + k);
                        break;
                    }
                }
            }
        }

        return out;
    }
    else if (From[0] == "{")
    {
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.erase(From.begin());

        // Save symbol table for later restoration
        auto oldTable = table;

        out.info = code_scope;

        // Code scope.
        int count = 1;
        vector<string> curVec;
        while (true)
        {
            if (From[0] == "{")
            {
                count++;
            }
            else if (From[0] == "}")
            {
                count--;

                if (count == 0)
                {
                    out.items.push_back(__createSequence(curVec));
                    curVec.clear();
                    sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                    From.erase(From.begin());
                    break;
                }
            }

            if (count == 1 && (From[0] == ";" || From[0] == "}"))
            {
                if (!curVec.empty())
                {
                    out.items.push_back(__createSequence(curVec));
                    out.items.back().type = nullType;
                    curVec.clear();
                }

                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.erase(From.begin());
            }
            else
            {
                if (!From.empty())
                {
                    curVec.push_back(From[0]);
                    sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                    From.erase(From.begin());
                }
                else
                {
                    break;
                }
            }
        }

        // Restore symbol table
        // This copies only newly instantiated functions; No other symbols.
        restoreSymbolTable(oldTable);

        // Check if/else validity
        for (int i = 1; i < out.items.size(); i++)
        {
            if (out.items[i].info == keyword && out.items[i].raw == "else")
            {
                if (out.items[i - 1].items.size() != 0 && out.items[i - 1].items[0].info == keyword && out.items[i - 1].items[0].raw == "if")
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
    // Function calls may occur within.

    for (int i = 0; i < From.size(); i++)
    {
        string cur = From[i];

        if (cur.size() > 11 && cur.substr(0, 11) == "//__LINE__=")
        {
            // Line update special symbol
            string newLineNum = cur.substr(12);
            curLine = stoull(newLineNum);
            continue;
        }
        else
        {
            sequence temp;
            temp.info = atom;
            temp.type = resolveFunction(From, i, temp.raw);

            out.items.push_back(temp);
        }
    }

    From.clear();

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

    switch (What.info)
    {
    case comma_sep:
        for (int i = 0; i < What.items.size(); i++)
        {
            out += toC(What.items[i]);

            if (i + 1 != What.items.size())
            {
                out += " ";
            }
        }

        break;

    case code_line:
        for (int i = 0; i < What.items.size(); i++)
        {
            out += toC(What.items[i]);

            if (i + 1 != What.items.size())
            {
                out += ", ";
            }
        }

        break;

    case code_scope:
        out += "{\n";

        for (int i = 0; i < What.items.size(); i++)
        {
            if (What.items[i].info == keyword)
            {
                out += toC(What.items[i]);
            }
            else if (What.items[i].type == nullType)
            {
                out += toC(What.items[i]) + ";\n";
            }
            else
            {
                out += "return (" + toC(What.items[i]) + ");\n";
            }
        }

        out += "}\n";

        break;

    case for_triple:
        sm_assert(What.items.size() == 3, "For loop triple must contain 3 elements");
        sm_assert(What.items[1].type == Type(atomic, "bool"), "Second argument of for loop triple must be convertible to bool.");

        out += "(" + toC(What.items[0]) + "; " + toC(What.items[1]) + "; " + toC(What.items[2]) + ")";

        break;

    case function_call:
        // Typing cannot be enforced here!

        out += toC(What.items[0]);
        out += "(";

        for (int i = 1; i < What.items.size(); i++)
        {
            out += toC(What.items[i]);

            if (i + 1 < What.items.size())
            {
                out += ", ";
            }
        }

        out += ")";

        break;

    case parenthesis:
        // No join characters

        out += "(";

        for (auto child : What.items)
        {
            out += toC(child) + " ";
        }

        out += ")";

        break;

    case access:
        out += "[" + toC(What.items[0]) + "]";

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
    }

    return out;
}

// Get the return type from a Type (of a function signature)
Type getReturnType(const Type &T)
{
    Type temp(T);

    for (Type *cur = &temp; cur != nullptr; cur = cur->next)
    {
        if (cur->info == maps)
        {
            Type out(*cur->next);
            return out;
        }
    }

    return T;
}

// Assume the input has no extraneous symbols
vector<string> fixMethodCall(const vector<string> &What)
{
    if (What.size() < 2)
    {
        vector<string> out = What;
        return out;
    }

    // Parse last dot chunk
    vector<string> finalChunk;
    vector<string> rest = What;

    vector<string> out;

    int count = 0;
    while (!rest.empty() && !(rest.back() == "." && count == 0))
    {
        if (rest.back() == "(")
        {
            count++;
        }
        else if (rest.back() == ")")
        {
            count--;
        }

        finalChunk.insert(finalChunk.begin(), rest.back());
        rest.pop_back();
    }

    if (!rest.empty())
    {
        rest.pop_back();
    }

    // Determine if method or member
    if (finalChunk.back() == ")")
    {
        bool needsComma = false;

        // Method call; Prepend
        out.push_back(finalChunk.front());
        out.push_back("(");

        if (!rest.empty())
        {
            needsComma = true;

            out.push_back("&");

            // RECURSE for previous
            vector<string> fixedRest = fixMethodCall(rest);
            for (string s : fixedRest)
            {
                out.push_back(s);
            }
        }

        // ARGUMENTS
        vector<string> curArg;
        int i = 2;
        int count = 1;

        do
        {
            if (finalChunk[i] == "(")
            {
                count++;
            }
            else if (finalChunk[i] == ")")
            {
                count--;
            }

            if (finalChunk[i] == ")" && count == 0)
            {
                vector<string> fixedArg = fixMethodCall(curArg);

                if (needsComma && fixedArg.size() != 0)
                {
                    out.push_back(",");
                }

                for (string s : fixedArg)
                {
                    out.push_back(s);
                }

                needsComma = true;

                break;
            }
            else if (finalChunk[i] == "," && count == 1)
            {
                vector<string> fixedArg = fixMethodCall(curArg);

                if (needsComma && fixedArg.size() != 0)
                {
                    out.push_back(",");
                }

                for (string s : fixedArg)
                {
                    out.push_back(s);
                }

                curArg.clear();

                needsComma = true;
            }
            else
            {
                curArg.push_back(finalChunk[i]);
            }

            i++;
        } while (i < finalChunk.size());

        out.push_back(")");
    }
    else
    {
        // Member access; Append
        sm_assert(!rest.empty(), "Cannot access member of NULL");

        // RECURSE
        vector<string> fixedRest = fixMethodCall(rest);
        for (string s : fixedRest)
        {
            out.push_back(s);
        }

        out.push_back(".");
        out.push_back(finalChunk.front());
    }

    return out;
}

vector<pair<string, Type>> getArgs(Type &type)
{
    // Get everything between final function and maps
    vector<pair<string, Type>> out;

    Type *cur = &type;
    if (cur->info != function)
    {
        return out;
    }

    cur = cur->next;

    string symbName;
    Type symbType;
    for (; cur != nullptr; cur = cur->next)
    {
        if (cur->info == maps)
        {
            if (symbName != "")
            {
                out.push_back(make_pair(symbName, symbType));
            }

            break;
        }
        else if (cur->info == join)
        {
            if (symbName != "")
            {
                out.push_back(make_pair(symbName, symbType));

                symbName = "";
                symbType = nullType;
            }
        }
        else if (cur->info == var_name)
        {
            symbName = cur->name;
        }
        else
        {
            symbType.append(cur->info, cur->name);
        }
    }

    return out;
}

// This should only be called after method replacement
Type resolveFunction(const vector<string> &What, int &start, string &c)
{
    if (What.empty() || start >= What.size())
    {
        return nullType;
    }

    // Pointer check
    if (What[start] == "@")
    {
        start++;

        string followingC = "";
        Type type = resolveFunction(What, start, followingC);
        type.prepend(pointer);

        c += "&" + followingC;

        return type;
    }
    else if (What[start] == "^")
    {
        start++;

        string followingC = "";
        Type type = resolveFunction(What, start, followingC);

        sm_assert(type.info == pointer, "Cannot dereference non-pointer.");
        Type temp = *type.next;
        type = temp;

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

    // Function call
    else if (What.size() > start + 1 && What[start + 1] == "(")
    {
        // get args within parenthesis
        vector<string> curArg;
        vector<vector<string>> args;

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

            if (What[start] == "," && count == 1)
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

        c += name + "(";

        int i = 0;
        string argC = "";
        vector<Type> argTypes;
        for (vector<string> arg : args)
        {
            int trash = 0;
            argTypes.push_back(resolveFunction(arg, trash, argC));

            if (i + 1 < args.size())
            {
                argC += ", ";
            }

            i++;
        }

        c += argC + ")";

        // Search for candidates
        sm_assert(table.count(name) != 0, "Function call '" + name + "' has no registered symbols.");
        sm_assert(table[name].size() != 0, "Function call '" + name + "' has no registered symbols.");

        auto candidates = table[name];

        // Weed out any candidates which do not have the correct argument types
        for (int j = 0; j < candidates.size(); j++)
        {
            auto candArgs = getArgs(candidates[j].type);

            if (candArgs.size() != argTypes.size())
            {
                candidates.erase(candidates.begin() + j);
                j--;
                continue;
            }

            for (int k = 0; k < candArgs.size(); k++)
            {
                if (candArgs[k].second != argTypes[k])
                {
                    candidates.erase(candidates.begin() + j);
                    j--;
                    break;
                }
            }
        }

        // If no candidates, throw error
        if (candidates.size() == 0)
        {
            candidates = table[name];

            cout << "Candidates:\n";
            for (auto c : candidates)
            {
                cout << name << " has type " << toStr(&c.type) << '\n';
            }

            cout << "Arguments:\n(";
            for (auto a : argTypes)
            {
                cout << toStr(&a) << ", ";
            }
            cout << ")\n";

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
                    throw sequencing_error("Function call '" + name + "' has two or more viable candidates.");
                }
            }

            // Now safe; Has been verified that all candidates have identical types
        }
        else
        {
            // Single candidate
            type = getReturnType(candidates[0].type);
        }

        start--;
    }
    else
    {
        // Non-function

        // Literal check
        Type litType = checkLiteral(What[start]);
        if (litType == nullType)
        {
            // Is not a literal
            sm_assert(table.count(What[start]) != 0, "No definitions exist for symbol '" + What[start] + "'.");
            auto candidates = table[What[start]];
            sm_assert(candidates.size() != 0, "No definitions exist for symbol '" + What[start] + "'.");

            type = getReturnType(candidates.back().type);
        }
        else
        {
            // Is a literal
            type = litType;
        }

        c += What[start];
    }

    // if member access, resolve that
    while (start < What.size() && What[start + 1] == ".")
    {
        // Auto-dereference pointers (. to -> automatically)
        if (type.info == pointer)
        {
            while (type.info == pointer)
            {
                Type temp = *type.next;
                type = temp;

                c = "*" + c;
            }

            c = "(" + c + ")";
        }

        sm_assert(type.info == atomic, "Error during type trimming for member access; Could not find struct name.");
        string structName = type.name;

        sm_assert(structData.count(structName) != 0, "Struct type '" + structName + "' does not exist.");
        sm_assert(structData[structName].members.count(What[start + 2]) != 0, "Struct '" + structName + "' has no member '" + What[start + 2] + "'.");

        type = structData[structName].members[What[start + 2]];

        c += "." + What[start + 2];

        start += 2;
    }

    start++;

    // Return function return type
    return type;
}

Type checkLiteral(const string &From)
{
    // Check as bool
    if (From == "true" || From == "false")
    {
        return Type(atomic, "bool");
    }

    // Check as string
    else if ((From.front() == '"' && From.back() == '"') || (From.front() == '\'' && From.back() == '\''))
    {
        return Type(atomic, "str");
    }

    // Check as numbers
    bool canBeDouble = true;
    for (char c : From)
    {
        if (!('0' <= c && c <= '9') && c != '-' && c != '.')
        {
            canBeDouble = false;
            break;
        }
    }

    if (canBeDouble)
    {
        if (From.find(".") == string::npos)
        {
            // Int

            // Check size
            long long val = stoll(From);

            if (abs(val) <= __INT_MAX__)
            {
                return Type(atomic, "i32");
            }
            else if (abs(val) <= __LONG_MAX__)
            {
                return Type(atomic, "i64");
            }
            else
            {
                return Type(atomic, "i128");
            }
        }
        else
        {
            // Float
            return Type(atomic, "f64");
        }
    }

    // Default: Not a literal
    return nullType;
}

void restoreSymbolTable(multiSymbolTable &backup)
{
    multiSymbolTable newTable = backup;
    for (auto p : table)
    {
        if (p.second.size() == backup[p.first].size())
        {
            continue;
        }
        else
        {
            for (auto s : p.second)
            {
                if (s.type.info == function)
                {
                    bool present = false;
                    for (auto ss : backup[p.first])
                    {
                        if (ss.type == s.type)
                        {
                            present = true;
                            break;
                        }
                    }

                    if (!present)
                    {
                        newTable[p.first].push_back(s);
                    }
                }
            }
        }
    }

    table = newTable;

    return;
}
