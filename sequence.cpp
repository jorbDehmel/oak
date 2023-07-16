/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "sequence.hpp"

// Also New() for constructors
const map<string, string> operatorAliases = {
    make_pair("Get", "[]"),
    make_pair("Less", "<"),
    make_pair("Great", ">"),
    make_pair("Leq", "<="),
    make_pair("Greq", ">="),
    make_pair("Eq", "=="),
    make_pair("Neq", "!="),
    make_pair("And", "&"),
    make_pair("Andd", "&&"),
    make_pair("Or", "|"),
    make_pair("Orr", "||"),
    make_pair("Not", "!"),

    make_pair("Call", "()"),
    make_pair("Copy", "="),
    make_pair("Del", "~"),

    make_pair("Add", "+"),
    make_pair("Sub", "-"),
    make_pair("Mult", "*"),
    make_pair("Div", "/"),
    make_pair("Mod", "%"),
    make_pair("Xor", "^"),

    make_pair("AddEq", "+="),
    make_pair("SubEq", "-="),
    make_pair("MultEq", "*="),
    make_pair("DivEq", "/="),
    make_pair("ModEq", "%="),
    make_pair("XorEq", "^="),

    make_pair("Incr", "++"),
    make_pair("Decr", "--"),

    make_pair("AndEq", "&="),
    make_pair("OrEq", "|="),

    make_pair("Lbs", "<<"),
    make_pair("Rbs", ">>"),

    make_pair("Ref", "&"),
    make_pair("Deref", "*"),

    make_pair("New", "<INVALID SUBSTITUTION>"),
};

map<string, string> inverseOperatorAliases;

void setUpInverseOperatorAliases()
{
    if (inverseOperatorAliases.size() == 0)
    {
        for (pair<string, string> p : operatorAliases)
        {
            inverseOperatorAliases[p.second] = p.first;
        }
    }

    return;
}

/*
// Method substitution
a().b().c().d().e();
e(d(c(b(a()))));

// Operator substitution
a[0].b[1][2]
Get(Get(Get(a, 0).b, 1), 2)

[] <=> Get()
<  <=> Less()
>  <=> Great()
<= <=> Leq()
>= <=> Greq()
== <=> Eq()
!= <=> Neq()
&  <=> And()
&& <=> Andd()
|  <=> Or()
|| <=> Orr()
!  <=> Not()

() <=> Call()
   <=> New()
=  <=> Copy()
~  <=> Del()

+  <=> Add()
-  <=> Sub()
/  <=> Div()
*  <=> Mult()
%  <=> Mod()
^  <=> Xor()

+= <=> AddEq()
-= <=> SubEq()
/= <=> DivEq()
*= <=> MultEq()
%= <=> ModEq()
^= <=> XorEq()

++ <=> Incr()
-- <=> Decr()

&= <=> AndEq()
|= <=> OrEq()

<< <=> Lbs()
>> <=> Rbs()

&  <=> Ref()
*  <=> Deref()

*/

// Any string not in this set is a symbol name
const set<string>
    specials = {
        "+",
        "-",
        "*",
        "/",
        "%",
        "&",
        "|",
        "<",
        ">",
        "<=",
        ">=",
        "==",
        "!=",
        "!",
        "^",
        "&&",
        "(",
        ")",
        "{",
        "}",
        "[",
        "]",
        "||",
        "&&",
        "~",
        ":",
        ";",
        "->",
        "&",
        "<<",
        ">>",
        "+=",
        "-=",
        "*=",
        "/=",
        "&=",
        "|=",
        "^=",
        "=",

        "let",
        "for",
        "struct",
        "live",
        "if",
        "else",
        "while",
        "const",
        "mut",
        "catch",
        "try",
        "switch",
        "case",
};

template <class T>
void highlightIssue(const vector<T> &From, const unsigned int &Index, const unsigned int &Radius)
{
    cout << tags::red_bold
         << "In recent: "
         << tags::reset
         << "'";

    for (int j = -Radius; j < Radius; j++)
    {
        if (Index + j > 0 && Index + j < From.size())
        {
            cout << (j == -1 ? tags::yellow_bold : "")
                 << From[Index + j]
                 << (j == 1 ? tags::reset : "")
                 << ' ';
        }
    }

    cout << "'\n";

    return;
}

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
        else if (left->info == generic || left->info == atomic)
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
        cout << "function_call";
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

/////////////////////////////////////////////////////////////////////////

// Internal consumptive version: Erases from vector, so not safe for frontend
sequence __createSequence(vector<string> &From)
{
    // printvec(From);

    sequence out;
    out.info = code_line;

    if (From.size() == 0)
    {
        return out;
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
    else if (From[0] == "if")
    {
        // Takes a bool code line and a code scope / code line
        out.info = keyword;
        out.raw = From[0];
        out.type = nullType;

        pop_front(From);

        // Pops the first full sequence from the remaining front
        // This is for the conditional
        out.items.push_back(__createSequence(From));

        debugPrint(out.items[0]);

        sm_assert(out.items[0].type == Type(atomic, "bool"), "If statement argument must be boolean.");
        sm_assert(!From.empty(), "Missing statement after if");

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
        // call addSymbol or addStruct

        // let NAME: TYPE;
        // let NAME: TYPE = {};
        // let NAME: TYPE = OTHER_NAME;

        // let NAME : TYPE TYPE TYPE ( ; = )
        // if (=), take another chunk.

        // let NAME() -> hi;
        // let NAME() -> hi {}
        // let NAME(ARG, ARG) -> hi;

        // let NAME<generics>() -> hi;
        // let NAME: hi<types>;

        /*
        cout << "Local contents: '";
        for (int i = 0; i < 5; i++)
            if (i < From.size())
                cout << From[i] << ' ';
        cout << "'\n";
        */

        // Get name
        pop_front(From); // let
        seq_assert(!From.empty());
        string name = From[0];
        pop_front(From);

        // This should handle method calls for now
        while (From[0] == ".")
        {
            sm_assert(name.find(".") == string::npos, "Method definition must occur on typename.");

            name += ".";
            pop_front(From);

            if ('A' <= From[0][0] && From[0][0] <= 'Z')
            {
                sm_assert(operatorAliases.count(From[0]) != 0, From[0] + " is not a valid operator alias.");
            }
            name += From[0];
            pop_front(From);
        }

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

                auto s = addSymb(name, toAdd);
                s->seq = __createSequence(From);

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
            // pop_front(From);

            // Insert symbol
            auto s = addSymb(name, toAdd);

            map<string, Type> argsWithType = getArgs(s->ts.type);
            for (pair<string, Type> p : argsWithType)
            {
                table[p.first].push_back(__multiTableSymbol{tableSymbol(p.second), sequence{}});
            }

            if (From.front() == "{")
            {
                s->seq = __createSequence(From);
            }

            for (pair<string, Type> p : argsWithType)
            {
                for (int k = 0; k < table[p.first].size(); k++)
                {
                    if (table[p.first][k].ts.type == p.second)
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
        // sm_assert(false, "UNIMPLEMENTED");

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
        cout << "Parenthesis wound up with type " << toStr(&out.type) << '\n';

        return out;
    }
    else if (From[0] == "{")
    {
        pop_front(From);

        out.info = code_scope;

        // Back up symbol table
        // multiSymbolTable oldTable = table;

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

                debugPrint(out);

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
    int i;

    try
    {
        for (i = 0; i < From.size(); i++)
        {
            string cur = From[i];

            if (cur == ".")
            {
                sm_assert(i > 0 && specials.count(From[i - 1]) == 0, "'" + From[i - 1] + "' is not a struct.");
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
                        out.type = getReturnType(From[i - 1], s.type);

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

                try
                {
                    out.items.back().type = getType(cur);
                }
                catch (...)
                {
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
                    else
                    {
                        cout << tags::yellow_bold
                             << "Warning: A type could not be found for symbol '" << cur << "'\n"
                             << tags::reset;
                    }
                }
            }
            else
            {
                out.items.push_back(sequence{nullType, vector<sequence>(), atom, cur});
            }
        }

        From.clear();
    }
    catch (sequencing_error &e)
    {
        highlightIssue(From, i, 5);

        throw e;
    }

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
        sm_assert(What.items.size() == 3, "For loop triple must contain 3 elements.");
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

////////////////////// Utilities //////////////////////

// Get the return type of a function which exists in the symbol table
Type getReturnType(const string &Name, const Type &ArgType)
{
    auto functions = table[Name];
    sm_assert(functions.size() > 0, "No candidate definitions exists for '" + Name + "'");

    for (auto f : functions)
    {
        // Function type anatomy in symbol table:
        // function -> (TEMPLATING -> function) -> ARGS -> maps -> RETURN_TYPE
        //
        // So we want to check everything between the LAST function
        // and maps

        Type curType;
        Type *cursor = &f.ts.type;
        seq_assert(cursor != nullptr);

        // Find last function keyword in f.type
        if (f.ts.type.info != function)
        {
            continue;
        }

        seq_assert(cursor->next != nullptr);
        if (cursor->next->info == templ)
        {
            while (cursor->info != templ_end)
            {
                cursor = cursor->next;
                seq_assert(cursor != nullptr);
            }
            cursor = cursor->next;

            seq_assert(cursor != nullptr);
            sm_assert(cursor->info == function, "Failed to parse return type of malformed templated function.");
            cursor = cursor->next;
            seq_assert(cursor != nullptr);
        }

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

        // DONT FORGET GENERICS HERE

        if (curType == ArgType)
        {
            // Get return type
            return getReturnType(f.ts.type);
        }
        else
        {
            // Need a non-const version
            Type argTypeClone(ArgType);

            // Generics test
            bool isValid = true;
            for (Type *a = &curType, *b = &argTypeClone; isValid && a != nullptr && b != nullptr;)
            {
                if (a->info == b->info)
                {
                    a = a->next;
                    b = b->next;

                    continue;
                }
                else if (a->info == generic)
                {
                    while (b != nullptr && b->info != join)
                    {
                        b = b->next;
                    }

                    if (a != nullptr && b != nullptr)
                    {
                        a = a->next;
                        b = b->next;
                    }

                    continue;
                }
                else if (b->info == generic)
                {
                    while (a != nullptr && a->info != join)
                    {
                        a = a->next;
                    }

                    if (a != nullptr && b != nullptr)
                    {
                        a = a->next;
                        b = b->next;
                    }

                    continue;
                }
                else
                {
                    isValid = false;
                }
            }

            if (isValid)
            {
                return getReturnType(f.ts.type);
            }
        }
    }

    // Failure case; No suitable function definition exists.
    for (auto c : functions)
    {
        cout << "Candidate '" << Name << "' has type " << toStr(&c.ts.type) << '\n';
    }
    cout << "No candidate had the required argument type " << toStr(&ArgType) << '\n';

    throw sequencing_error("No candidate functions matched required call for '" + Name + "'");

    // Unreachable but principled
    return Type();
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
        if (cur->info == templ)
        {
            count++;
        }
        else if (cur->info == templ_end)
        {
            count--;
        }
        else if (count == 0 && cur->info == atomic)
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
// a.b.c.d().e
// NEVER let a: squimbo = a.b.c.d().e + 5;
// NEVER let a.b() -> a;
vector<string> fixMethodCall(const vector<string> &What)
{
    /*
    Outline of algorithm:
    1) Parse last "dot" chunk
        1a) If member access, append this to end
        1b) If method, append this to beginning
            Also parse args and append them
    2) Recurse on rest of chunks
    */

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

    return candidates[0].ts.type;
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

    if (cur->next->info == templ)
    {
        while (cur->info != templ_end)
        {
            cur = cur->next;
        }
        cur = cur->next;

        if (cur->info != function)
        {
            return out;
        }
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
