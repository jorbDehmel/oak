/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "sequence.hpp"

unsigned long long int curLine = 1;
string curFile = "";

// Any string not in this set is a symbol name
const set<string> specials = {
    "+", "-", "*", "/", "%", "&", "|", "<", ">", "<=", ">=", "==",
    "!=", "!", "^", "&&", "(", ")", "{", "}", "[", "]", "||", "&&",
    "~", ":", ";", "->", "&", "<<", ">>", "+=", "-=", "*=", "/=",
    "&=", "|=", "=", "let", "for", "struct", "if", "else", "while",
    "const", "mut", "catch", "try", "switch", "case"};

// Compare two types until they have a join (,) or end
bool compareTypesUntilJoin(Type *A, Type *B)
{
    Type *left = A;
    Type *right = B;

    while (left != nullptr && right != nullptr)
    {
        if (left->info == join && right->info == join)
        {
            return true;
        }
        else if (left->info != right->info)
        {
            return false;
        }
        else if (left->info == atomic)
        {
            if (left->name != right->name)
            {
                return false;
            }
        }

        left = left->next;
        right = right->next;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////

void debugPrint(sequence &What, int spaces)
{
    for (int i = 0; i < spaces; i++)
    {
        cout << "| ";
    }

    switch (What.info)
    {
    case list:
        cout << "list";
        break;
    case function_call:
        cout << toStr(&What.type) << " function_call";
        break;
    case parenthesis:
        cout << "parenthesis";
        break;
    case code_scope:
        cout << "code_scope";
        break;
    case code_line:
        cout << "code_line";
        break;
    case for_triple:
        cout << "for_triple";
        break;
    case access:
        cout << "access";
        break;
    case atom:
        cout << "atom";
        break;
    case keyword:
        cout << "keyword";
        break;
    }

    cout << " w/ raw " << What.raw << ":\n";
    for (auto s : What.items)
    {
        debugPrint(s, spaces + 1);
    }

    return;
}

// Internal consumptive version: Erases from vector, so not safe for frontend
sequence __createSequence(vector<string> &From, bool templated = false)
{
    sequence out;
    out.info = code_line;

    if (From.size() == 0)
    {
        return out;
    }

    if (From[0].size() > 11 && From[0].substr(0, 11) == "//__LINE__=")
    {
        // Line update special symbol
        string newLineNum = From[0].substr(11);

        curLine = stoull(newLineNum);

        pop_front(From);
        return __createSequence(From);
    }
    if (From[0] == ";")
    {
        pop_front(From);
        return __createSequence(From);
    }
    else if (From[0] == ".")
    {
        out.info = keyword;
        out.raw = From[0];
        return out;
    }
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

        pop_front(From);

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

        pop_front(From);

        out.items.push_back(__createSequence(From));

        return out;
    }
    else if (From[0] == "try")
    {
        // Takes a code scope / code line
        out.info = keyword;
        out.raw = From[0];
        out.type = nullType;

        pop_front(From);

        out.items[0] = __createSequence(From);

        sm_assert(!From.empty() && From[0] == "catch", "Try block must be followed by catch block");

        return out;
    }
    else if (From[0] == "catch")
    {
        // Takes a code scope / code line

        // I'll figure out a graceful solution to error capture later
        // Maybe a global string? IDK

        out.info = keyword;
        out.raw = From[0];
        out.type = nullType;

        pop_front(From);

        out.items[0] = __createSequence(From);

        return out;
    }
    else if (From[0] == "let")
    {
        // Get name
        pop_front(From); // let
        seq_assert(!From.empty());
        string name = From[0];
        pop_front(From);

        // Get type
        if (From[0] == ":")
        {
            pop_front(From);
            if (!From.empty() && (From.front() == "struct" || From.front() == "live"))
            {
                // addStruct takes in the full struct definition, from
                // let to end curly bracket. So we must first parse this.

                vector<string> toAdd = {"let", name, ":", From.front()};

                int count = 0;
                while (count != 0 || (From[0] != "}" && From[0] != ";"))
                {
                    toAdd.push_back(From[0]);
                    pop_front(From);
                }
                toAdd.push_back(From[0]);
                pop_front(From);

                // addStruct
                addStruct(toAdd);

                // This should be left out of toC, as it should only be used
                // in the header file during reconstruction.
            }
            else
            {
                // Non-struct definition; Local var, not function

                // Scrape entire definition for this
                vector<string> toAdd = {
                    "let",
                    name,
                    ":",
                };

                while (!From.empty() && From.front() != ";" && From.front() != "=")
                {
                    toAdd.push_back(From.front());
                    pop_front(From);
                }

                addSymb(name, toAdd, __createSequence(From));

                // This SHOULD appear during toC.
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
                pop_front(From);
            } while (From[0] != "{" && From[0] != ";");

            auto type = toType(toAdd);

            map<string, Type> argsWithType = getArgs(type);
            for (pair<string, Type> p : argsWithType)
            {
                table[p.first].push_back(__multiTableSymbol{sequence(), p.second});
            }

            // Insert symbol
            if (From.front() == "{")
            {
                addSymb(name, toAdd, __createSequence(From));
            }
            else
            {
                addSymb(name, toAdd, sequence{});
            }

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
    else if (From[0] == "(")
    {
        // Parenthesis
        // At this stage, is NEVER a function call.

        vector<string> internals;
        int count = 0;

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

            internals.push_back(From.front());
            pop_front(From);
        } while (count != 0 && !From.empty());

        if (internals.front() == "(")
        {
            pop_front(internals);
        }
        if (internals.back() == ")")
        {
            internals.pop_back();
        }

        out.info = parenthesis;
        out.items.push_back(__createSequence(internals));

        out.type = out.items[0].type;

        return out;
    }
    else if (From[0] == "{")
    {
        pop_front(From);

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
                    pop_front(From);
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

                pop_front(From);
            }
            else
            {
                if (!From.empty())
                {
                    curVec.push_back(From[0]);
                    pop_front(From);
                }
                else
                {
                    break;
                }
            }
        }

        // Restore symbol table
        // table = oldTable;

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
    else if (From[0] == "[")
    {
        sm_assert(false, "UNIMPLEMENTED");

        // List

        return out;
    }

    // Non-special case; code line.
    // Function calls may occur within.

    Type curType;
    int i = 0;

    for (i = 0; i < From.size(); i++)
    {
        string cur = From[i];

        if (cur.size() > 11 && cur.substr(0, 11) == "//__LINE__=")
        {
            // Line update special symbol
            string newLineNum = cur.substr(12);
            curLine = stoull(newLineNum);
            continue;
        }
        else if (cur == ".")
        {
            sm_assert(i > 0 && specials.count(From[i - 1]) == 0, "'" + From[i - 1] + "' is not a struct.");
        }
        else if (cur == ",")
        {
            continue;
        }
        else if (cur == "<")
        {
            // Templated function call
            sm_assert(i > 0, "Missing function name on function call parenthesis.");
            string name = From[i - 1];

            // Part 1: Instantiate templated call
            {
                vector<string> templContents;
                vector<Type> splitTypes;

                // Get template contents here
                int count = 1;
                do
                {
                    i++;

                    if (From[i] == "<")
                    {
                        count++;
                    }
                    else if (From[i] == ">")
                    {
                        count--;
                    }

                    if ((count == 1 && From[i] == ",") || (count == 0 && From[i] == ">"))
                    {
                        splitTypes.push_back(toType(templContents));
                        templContents.clear();
                    }
                    else
                    {
                        templContents.push_back(From[i]);
                    }
                } while (count != 0 && i < From.size());

                instantiateTemplate(name, splitTypes);
            }

            // Part 2: Standard function call
            {
                i++;

                vector<string> contents;
                int count = 0, j = i;

                int eraseStart = i - 1, eraseNum = 1;

                do
                {
                    contents.push_back(From[j]);

                    if (From[j] == "(")
                    {
                        count++;
                    }
                    else if (From[j] == ")")
                    {
                        count--;
                    }

                    eraseNum++;
                    j++;
                } while (j < From.size() && count != 0);

                contents.pop_back();
                pop_front(contents);

                // Function
                sequence s = __createSequence(contents, true);

                out.info = function_call;
                out.items.clear();

                out.items.push_back(sequence{
                    nullType,
                    vector<sequence>(),
                    atom,
                    name});

                out.items.push_back(s);

                // Search for candidate function
                out.type = getReturnType(name, s.type);

                for (int k = 0; k < eraseNum; k++)
                {
                    From.erase(From.begin() + eraseStart);
                }

                out.type = out.items[0].type;

                return out;
            }
        }
        else if (cur == "(")
        {
            vector<string> contents;
            int count = 0, j = i;

            int eraseStart = i - 1, eraseNum = 1;

            do
            {
                contents.push_back(From[j]);

                if (From[j] == "(")
                {
                    count++;
                }
                else if (From[j] == ")")
                {
                    count--;
                }

                eraseNum++;
                j++;
            } while (j < From.size() && count != 0);

            if (i > 0 && specials.count(From[i - 1]) == 0)
            {
                // Calls; Resolve argument list, then check candidates
                contents.pop_back();
                pop_front(contents);

                if (i < 3 || From[i - 2] != ".")
                {
                    // Function
                    sm_assert(i > 0, "Missing function name on function call parenthesis.");

                    sequence s = __createSequence(contents);

                    out.info = function_call;
                    out.items.clear();

                    out.items.push_back(sequence{
                        nullType,
                        vector<sequence>(),
                        atom,
                        From[i - 1]});

                    out.items.push_back(s);

                    // Search for candidate function
                    for (int k = 0; k < eraseNum; k++)
                    {
                        From.erase(From.begin() + eraseStart);
                    }
                }
                else
                {
                    // Method; Fix
                    sm_assert(false, "UNIMPLEMENTED");
                }
            }
            else
            {
                // Parenthesis
                // sm_assert(false, "UNIMPLEMENTED");

                pop_front(From);

                out.info = parenthesis;

                // Code scope.
                int count = 1;
                vector<string> curVec;
                while (!From.empty())
                {
                    if (From[0] == "(")
                    {
                        count++;
                    }
                    else if (From[0] == ")")
                    {
                        count--;

                        if (count == 0)
                        {
                            out.items.push_back(__createSequence(curVec));
                            curVec.clear();
                            pop_front(From);
                            break;
                        }
                    }

                    curVec.push_back(From[0]);
                    pop_front(From);
                }
            }

            out.type = out.items[0].type;

            return out;
        }
        else if (cur == "[")
        {
            if (i > 0 && specials.count(From[i - 1]) == 0)
            {
                // Access operator

                // a[b.c.d()] <=> Get(a, b.c.d()) -> Get(&a, d(&b.c))

                sm_assert(false, "UNIMPLEMENTED");
            }
            else
            {
                // List
                sm_assert(false, "UNIMPLEMENTED");
            }
        }
        else if (specials.count(cur) == 0)
        {
            // Symbol name
            out.items.push_back(sequence{Type(), vector<sequence>(), atom, cur});

            if ((cur[0] == '"' && cur.back() == '"') || (cur[0] == '\'' && cur[0] == '\''))
            {
                // String literal
                out.items.back().type = Type(atomic, "str");
            }
            else if (('0' <= cur[0] && cur[0] <= '9') || cur[0] == '-')
            {
                // Number literal
                if (cur.find(".") != string::npos)
                {
                    out.items.back().type = Type(atomic, "f32");
                }
                else
                {
                    out.items.back().type = Type(atomic, "i32");
                }
            }
            else if (cur == "true" || cur == "false")
            {
                out.items.back().type = Type(atomic, "bool");
            }
            else if (cur == "void")
            {
                out.items.back().type = nullType;
            }
            else if (i + 1 < From.size() && From[i + 1] == "<")
            {
                continue;
            }
            else
            {
                try
                {
                    out.items.back().type = getType(cur);
                }
                catch (runtime_error &e)
                {
                    try
                    {
                        if (table.count(cur) == 0 || table[cur].size() == 0)
                        {
                            throw runtime_error("No candidates were found for symbol '" + cur + "'");
                        }

                        auto candidates = table[cur];

                        // If all candidates have the same return type, we can deduce type
                        Type retType = getReturnType(candidates[0].type);

                        for (int k = 1; k < candidates.size(); k++)
                        {
                            if (getReturnType(candidates[k].type) != retType)
                            {
                                throw runtime_error("Ambiguous return type; Cannot deduce symbol type.");
                            }
                        }
                        out.items.back().type = retType;
                    }
                    catch (runtime_error &e)
                    {
                        cout << tags::yellow_bold
                             << "Warning: A type could not be found for symbol '" << cur << "'\n"
                             << tags::reset;

                        throw parse_error(e.what());
                    }
                }
            }
        }
        else
        {
            out.items.push_back(sequence{nullType, vector<sequence>(), atom, cur});
        }
    }

    From.clear();

    if (out.items.size() == 1)
    {
        return out.items[0];
    }

    return out;
}

// Type enforced version
sequence createSequence(const vector<string> &From, const Type type)
{
    sequence out = createSequence(From);
    seq_assert(out.type == type);
    return out;
}

sequence createSequence(const string &From)
{
    vector<string> lexed = lex(From);
    return createSequence(lexed);
}

// Turn a .oak sequence into a .cpp one
string toC(const sequence &What)
{
    string out = "";

    switch (What.info)
    {
    case code_line:
        // Does NOT append a semicolon!

        /*
        a, b, c;
        let a = 5, let b = 6, let c = (a == b);
        */

        for (int i = 0; i < What.items.size(); i++)
        {
            out += toC(What.items[i]);

            if (i + 1 != What.items.size() && out.back() != '.')
            {
                out += " ";
            }
        }

        break;

    case code_scope:
        /*
        {
            a;
            b;
            c;
            d
        }
        */

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

    case list:
        // Typing cannot be enforced here!

        out += "{";

        for (int i = 0; i < What.items.size(); i++)
        {
            out += toC(What.items[i]);

            if (i + 1 < What.items.size())
            {
                out += ", ";
            }
        }

        out += "}";

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
        for (auto i : out)
        {
            outSeq.items.push_back(i);
        }
        return outSeq;
    }
}

// Get the return type of a function which exists in the symbol table
Type getReturnType(const string &Name, const Type &ArgType, bool templated)
{
    auto functions = table[Name];
    sm_assert(functions.size() > 0, "No candidate definitions exists for '" + Name + "'");

    for (auto f : functions)
    {
        // Function type anatomy in symbol table:
        // function -> ARGS -> maps -> RETURN_TYPE

        Type curType;
        Type *cursor = &f.type;
        seq_assert(cursor != nullptr);

        // Find last function keyword in f.type
        if (f.type.info != function)
        {
            continue;
        }

        seq_assert(cursor->next != nullptr);

        // Skip past function
        cursor = cursor->next;

        // Scrape until maps keyword is reached, then stop
        while (cursor != nullptr && cursor->info != maps)
        {
            if (cursor->info != var_name)
            {
                curType.append(cursor->info, cursor->name);
            }
            cursor = cursor->next;
        }

        // Compare against goal argument type
        if (curType == ArgType)
        {
            // Get return type
            return getReturnType(f.type);
        }
    }

    Type mustMatch = getReturnType(functions[0].type);
    for (int i = 1; i < functions.size(); i++)
    {
        if (getReturnType(functions[i].type) != mustMatch)
        {
            // Failure case; No suitable function definition exist
            for (auto c : functions)
            {
                cout << "Candidate '" << Name << "' has type " << toStr(&c.type) << '\n';
            }
            cout << "No candidate had the required argument type " << toStr(&ArgType) << '\n';

            throw sequencing_error("No candidate functions matched required call for '" + Name + "'");
        }
    }

    return mustMatch;
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

    throw sequencing_error("Can not get return value from non-function type.");
    return Type();
}

// Enforce member existence
Type getMemberType(const Type &Base, const string &Name)
{
    // Get struct name
    string structName = "";

    Type temp(Base);
    int count = 0;
    for (Type *cur = &temp; cur != nullptr; cur = cur->next)
    {
        if (count == 0 && cur->info == atomic)
        {
            structName = cur->name;
            break;
        }
    }
    sm_assert(structName != "", "Could not get struct name from malformed type.");

    sm_assert(structData[structName].members.count(Name) != 0, "struct " + structName + " has no member " + Name + ".");

    return structData[structName].members[Name];
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

vector<string> fixMethodCall(const string &What)
{
    vector<string> lexed = lex(What);
    return fixMethodCall(lexed);
}

Type getType(const string &Name)
{
    auto candidates = table[Name];

    // Safety checks
    sm_assert(candidates.size() != 0, "Symbol '" + Name + "' has no type.");
    sm_assert(candidates.size() < 2, "Symbol '" + Name + "' is ambiguous; No type could be discerned.");

    return candidates[0].type;
}

map<string, Type> getArgs(Type &type)
{
    // Get everything between final function and maps
    map<string, Type> out;

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
                out[symbName] = symbType;
            }

            break;
        }
        else if (cur->info == join)
        {
            if (symbName != "")
            {
                out[symbName] = symbType;

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
