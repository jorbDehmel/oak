/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "oakc_fns.hpp"
#include "oakc_structs.hpp"

// The current depth of createSequence
unsigned long long int depth = 0;

// Internal consumptive version: Erases from std::vector, so not safe for frontend
ASTNode __createSequence(std::list<Token> &From, AcornSettings &settings);

ASTNode createSequence(const std::vector<Token> &From, AcornSettings &settings)
{
    // Clone to feed into the consumptive version
    std::list<Token> temp;
    temp.assign(From.begin(), From.end());

    std::vector<ASTNode> out;

    // Feed to consumptive version
    while (!temp.empty())
    {
        while (!temp.empty() && temp.front() == ";")
        {
            temp.pop_front();
        }

        out.push_back(__createSequence(temp, settings));
    }

    if (out.size() == 0)
    {
        return ASTNode{};
    }
    else if (out.size() == 1)
    {
        return out[0];
    }
    else
    {
        ASTNode outSeq;
        outSeq.info = code_scope;
        outSeq.type = nullType;

        for (auto i : out)
        {
            outSeq.items.push_back(i);
        }
        return outSeq;
    }
}

// Internal consumptive version: Erases from std::vector, so not safe for frontend
ASTNode __createSequence(std::list<Token> &From, AcornSettings &settings)
{
    ASTNode out;
    out.info = code_line;

    // If empty, break
    if (From.empty())
    {
        return out;
    }

    // Update line if needed
    if (From.front().line != settings.curLine)
    {
        // Line update special symbol
        settings.curLine = From.front().line;
        settings.curLineSymbols.clear();
        if (From.size() != 0)
        {
            for (auto it = From.begin(); it != From.end() && it->line == settings.curLine; it++)
            {
                settings.curLineSymbols.push_back(*it);
            }
        }

        if (settings.curLineSymbols.size() != 0)
        {
            settings.curLineSymbols.erase(settings.curLineSymbols.begin());
        }
    }

    // Real cases
    if (From.front() == ",")
    {
        return out;
    }

    else if (From.front() == "let")
    {
        // Get name
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front(); // let
        sm_assert(!From.empty(), "'let' must be followed by something.");

        std::vector<std::string> names = {From.front()};

        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        // Handle plural definitions
        while (!From.empty() && From.front() == ",")
        {
            From.pop_front();

            sm_assert(!From.empty(), "Ill-formed 'let' statement: Name must follow ','");
            names.push_back(From.front());

            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();
        }

        // Gather templating if there is any
        std::vector<std::string> generics;
        while (From.front() == "<")
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
                {
                    // addStruct takes in the full struct definition, from
                    // let to end curly bracket. So we must first parse this.

                    // sm_assert(depth == 0, "Structs and enums may only be declared in the global scope.");

                    std::vector<Token> toAdd = {Token("let"), Token("NAME_HERE")};

                    // Add generics back in here
                    if (generics.size() != 0)
                    {
                        toAdd.push_back(Token("<"));
                        for (int i = 0; i < generics.size(); i++)
                        {
                            toAdd.push_back(generics[i]);
                        }
                        toAdd.push_back(Token(">"));
                    }

                    toAdd.push_back(Token(":"));
                    std::string front = From.front();

                    int count = 0;
                    while (count != 0 || (!From.empty() && From.front() != "}" && From.front() != ";"))
                    {
                        toAdd.push_back(From.front());

                        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                        From.pop_front();
                    }

                    toAdd.push_back(From.front());

                    sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                    From.pop_front();

                    bool exempt = false;
                    for (auto s : generics)
                    {
                        if (settings.structData.count(s) != 0 || checkLiteral(s) != nullType || s == "u8" ||
                            s == "i8" || s == "u16" || s == "i16" || s == "u32" || s == "i32" || s == "u64" ||
                            s == "i64" || s == "u128" || s == "i128" || s == "str" || s == "bool")
                        {
                            exempt = true;
                            break;
                        }
                    }

                    if (generics.size() == 0 || exempt)
                    {
                        for (auto name : names)
                        {
                            toAdd[1].text = name;
                            if (front == "struct")
                            {
                                // Non-templated struct
                                addStruct(toAdd, settings);
                            }
                            else if (front == "enum")
                            {
                                // Non-templated enum
                                addEnum(toAdd, settings);
                            }
                        }
                    }
                    else
                    {
                        // Check for needs / inst block here
                        std::vector<Token> preBlock, postBlock;

                        while (!From.empty() && (From.front() == "pre" || From.front() == "post"))
                        {
                            if (!From.empty() && From.front() == "pre")
                            {
                                // pop needs
                                From.pop_front();

                                // pop {
                                sm_assert(!From.empty() && From.front() == "{",
                                          "'pre' block must be followed by scope.");
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
                                        preBlock.push_back(From.front());
                                        From.pop_front();
                                    }
                                }
                            }
                            else if (!From.empty() && From.front() == "post")
                            {
                                // pop needs
                                From.pop_front();

                                // pop {
                                sm_assert(!From.empty() && From.front() == "{",
                                          "'post' block must be followed by scope.");
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
                                        postBlock.push_back(From.front());
                                        From.pop_front();
                                    }
                                }
                            }
                        }

                        for (auto name : names)
                        {
                            toAdd[1].text = name;
                            addGeneric(toAdd, name, generics, {front}, preBlock, postBlock, settings);
                        }
                    }

                    // This should be left out of toC, as it should only be used
                    // in the header file during reconstruction.
                }
            }
            else
            {
                // Non-struct definition; Local var, not function

                sm_assert(depth != 0, "Global variables are not allowed.");
                sm_assert(generics.empty(), "Variable declaration must not be templated.");

                // Scrape entire definition for this
                std::vector<Token> toAdd = {
                    Token("let"),
                    Token("NAME_HERE"),
                    Token(":"),
                };

                while (!From.empty() && From.front() != ";")
                {
                    if (From.front() == "type!")
                    {
                        // Case for type!() macro

                        // Scrape entire type!(what) call to a std::vector
                        std::vector<Token> toAnalyze;
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
                        std::string junk = "";
                        int pos = 0;

                        // Analyze type of std::vector
                        Type type = resolveFunction(toAnalyze, pos, junk, settings);

                        // Convert type to lexed std::string vec
                        Lexer dfa_lexer;
                        std::vector<Token> lexedType = dfa_lexer.lex(toStr(&type));

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

                Type type = toType(toAdd, settings);

                for (auto name : names)
                {
                    // Ensure this is a safe name
                    sm_assert(oakKeywords.count(name) == 0,
                              "Variable name conflicts w/ canonical Oak keyword '" + name + "'");
                    sm_assert(settings.structData.count(name) == 0 && atomics.count(name) == 0,
                              "Variable name conflicts w/ existing struct name '" + name + "'");
                    sm_assert(settings.enumData.count(name) == 0,
                              "Variable name conflicts w/ existing enum name '" + name + "'");
                    sm_assert(checkLiteral(name) == nullType, "Variable name cannot be a valid literal.");

                    out.items.push_back(ASTNode{nullType, std::vector<ASTNode>(), atom, toStrC(&type, settings, name)});
                    out.items.push_back(ASTNode{nullType, std::vector<ASTNode>(), atom, ";"});

                    // Insert into table
                    settings.table[name].push_back(MultiTableSymbol{ASTNode{type, std::vector<ASTNode>(), atom, ""},
                                                                    type, false, settings.curFile});

                    // Call constructor (pointers do not get constructors)
                    if (type[0].info != pointer && type[0].info != arr && type[0].info != sarr)
                    {
                        // Syntactically necessary
                        out.items.push_back(ASTNode{nullType, std::vector<ASTNode>(), atom, ";"});

                        std::vector<Token> newCall = {Token("New"), Token("("), Token("@"), Token(name), Token(")")};
                        int garbage = 0;

                        ASTNode toAppend;
                        toAppend.info = atom;
                        toAppend.type = resolveFunction(newCall, garbage, toAppend.raw, settings);

                        out.items.push_back(toAppend);

                        // Syntactically necessary
                        out.items.push_back(ASTNode{nullType, std::vector<ASTNode>(), atom, ";"});
                    }
                    else if (type[0].info != sarr)
                    {
                        ASTNode toAppend;
                        toAppend.info = atom;
                        toAppend.type = nullType;
                        toAppend.raw = name + "=0;";

                        out.items.push_back(toAppend);
                    }
                    else
                    {
                        // Initialize sized array
                        // Set every byte inside to zero

                        ASTNode toAppend;
                        toAppend.info = atom;
                        toAppend.type = nullType;

                        toAppend.raw =
                            "for (i32 _i = 0; _i < sizeof(" + name + "); _i++) ((u8 *)(&" + name + "))[_i] = 0;";

                        out.items.push_back(toAppend);
                    }
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
                std::vector<Token> argList;
                do
                {
                    argList.push_back(From.front());
                    sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                    From.pop_front();
                } while (From.front() != "{" && From.front() != ";");

                auto type = toType(argList, settings);

                auto oldTable = settings.table;

                auto argsWithType = getArgs(type, settings);
                for (std::pair<std::string, Type> p : argsWithType)
                {
                    settings.table[p.first].push_back(MultiTableSymbol{ASTNode(), p.second, false, settings.curFile});
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

                sm_assert(settings.currentReturnType == nullType, "Cannot nest function definitions.");
                settings.currentReturnType = getReturnType(type, settings);

                // Scrape contents
                int count = 0;
                std::list<Token> toAdd;

                if (From.front() != ";")
                {
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

                        toAdd.push_back(From.front());

                        From.pop_front();
                    } while (count != 0);
                }

                for (auto name : names)
                {
                    // Restrictions upon some types of methods
                    if (name == "New" || name == "Del")
                    {
                        if (settings.currentReturnType != Type(atomic, "void"))
                        {
                            throw std::runtime_error("Illegal method definition! Method '" + name +
                                                     "' must return void.");
                        }

                        if (!isSingleArg)
                        {
                            throw std::runtime_error("Illegal method definition! Method '" + name +
                                                     "' must have exactly one argument. Instead, type '" +
                                                     toStr(&type) + "'.");
                        }
                    }

                    // Restrictions upon main functions
                    if (name == "main" && !From.empty() && !toAdd.empty())
                    {
                        sm_assert(settings.table[name].size() == 0, "Function 'main' cannot be overloaded.");
                        sm_assert(settings.currentReturnType[0].info == atomic &&
                                      (settings.currentReturnType[0].name == "i32" ||
                                       settings.currentReturnType[0].name == "void"),
                                  "Function 'main' must return i32.");

                        if (argsWithType.size() != 0)
                        {
                            sm_assert(argsWithType.size() == 2, "'main' must have either 0 or 2 arguments.");
                            sm_assert(argsWithType[0].second == Type(atomic, "i32"),
                                      "First argument of 'main' must be i32, not " + toStr(&argsWithType[0].second));

                            if (!((argsWithType[1].second.size() == 2 && argsWithType[1].second[0].info == arr &&
                                   argsWithType[1].second[1].info == atomic &&
                                   argsWithType[1].second[1].name == "str") ||
                                  (argsWithType[1].second.size() == 3 && argsWithType[1].second[0].info == arr &&
                                   argsWithType[1].second[1].info == arr && argsWithType[1].second[2].info == atomic &&
                                   argsWithType[1].second[2].name == "i8")))
                            {
                                throw sequencing_error("Second argument of 'main' must be []str, or [][]i8, not " +
                                                       toStr(&argsWithType[1].second));
                            }
                        }
                    }

                    // Restrictions upon plural definitions
                    if (names.size() > 1 && (name == "New" || name == "Del" || name == "main"))
                    {
                        throw sequencing_error("Name '" + name + "' is not allowed in plural definitions.");
                    }

                    // Insert explicit symbol
                    if (!toAdd.empty())
                    {
                        destroyUnits(name, type, true, settings);

                        settings.table[name].push_back(MultiTableSymbol{ASTNode{}, type, false, settings.curFile});

                        if (name == "New")
                        {
                            // prepend New for all members
                            std::string structName;

                            // Type *cur = &argsWithType[0].second;
                            int cur = 0;

                            while (cur < argsWithType[0].second.size() && argsWithType[0].second[cur].info == pointer)
                            {
                                cur++;
                            }

                            structName = argsWithType[0].second[cur].name;

                            for (auto var : settings.structData[structName].members)
                            {
                                // Add semicolon
                                settings.table["New"].back().seq.items.push_back(
                                    ASTNode{nullType, std::vector<ASTNode>(), atom, ";"});

                                ASTNode toAppend;
                                toAppend.info = atom;
                                toAppend.type = nullType;
                                toAppend.raw =
                                    getMemberNew("(*" + argsWithType[0].first + ")", var.first, var.second, settings);

                                settings.table["New"].back().seq.items.push_back(toAppend);
                            }
                        }

                        depth++;
                        std::list<Token> temp;
                        temp.assign(toAdd.begin(), toAdd.end());
                        settings.table[name].back().seq = __createSequence(temp, settings);
                        depth--;

                        if (name == "Del")
                        {
                            // append Del for all members
                            std::string structName;

                            // Type *cur = &argsWithType[0].second;
                            int cur = 0;

                            while (cur < argsWithType[0].second.size() && argsWithType[0].second[cur].info == pointer)
                            {
                                cur++;
                            }

                            structName = argsWithType[0].second[cur].name;

                            for (auto var : settings.structData[structName].members)
                            {
                                // Add semicolon
                                settings.table["Del"].back().seq.items.push_back(
                                    ASTNode{nullType, std::vector<ASTNode>(), atom, ";"});

                                ASTNode toAppend;
                                toAppend.info = atom;
                                toAppend.type = nullType;
                                toAppend.raw =
                                    getMemberDel("(*" + argsWithType[0].first + ")", var.first, var.second, settings);

                                settings.table["Del"].back().seq.items.push_back(toAppend);
                            }
                        }
                    }
                    else
                    {
                        // Casual reference
                        // IE let to_i32(what: bool) -> i32;
                        // Destroys all autogen, requires
                        // actual definition.

                        destroyUnits(name, type, false, settings);

                        // Ensure exactly one unit declaration
                        settings.table[name].push_back(MultiTableSymbol{ASTNode{}, type, false, settings.curFile});
                    }

                    auto destructors = restoreSymbolTable(oldTable, settings.table);
                    // insertDestructors(table[name].back().seq, destructors);

                    settings.currentReturnType = nullType;
                }
            }
            else
            {
                // Templated function definition

                std::vector<Token> returnType, toAdd = {Token("let"), Token("NAME_HERE")};
                std::vector<std::string> typeVec;

                while (From.front() != "->")
                {
                    typeVec.push_back(From.front());
                    toAdd.push_back(From.front());

                    From.pop_front();
                }

                toAdd.push_back(From.front());

                From.pop_front();

                while (From.front() != "{" && From.front() != ";")
                {
                    toAdd.push_back(From.front());
                    returnType.push_back(From.front());

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

                    toAdd.push_back(From.front());

                    From.pop_front();
                } while (count != 0);

                // Check for needs / inst block here
                std::vector<Token> preBlock, postBlock;

                while (!From.empty() && (From.front() == "pre" || From.front() == "post"))
                {
                    if (!From.empty() && From.front() == "pre")
                    {
                        // pop needs
                        From.pop_front();

                        // pop {
                        sm_assert(!From.empty() && From.front() == "{", "'pre' block must be followed by scope.");
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
                                preBlock.push_back(From.front());
                                From.pop_front();
                            }
                        }
                    }
                    else if (!From.empty() && From.front() == "post")
                    {
                        // pop needs
                        From.pop_front();

                        // pop {
                        sm_assert(!From.empty() && From.front() == "{", "'post' block must be followed by scope.");
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
                                postBlock.push_back(From.front());
                                From.pop_front();
                            }
                        }
                    }
                }

                // Insert templated function
                for (auto name : names)
                {
                    toAdd[1].text = name;
                    addGeneric(toAdd, name, generics, typeVec, preBlock, postBlock, settings);
                }
            }
        }
        else
        {
            throw sequencing_error("Ill-formed 'let' statement: Must take the form `let a: type` or `let fn() -> "
                                   "type`. Instead, name is followed by '" +
                                   From.front().text + "'");
        }

        return out;
    } // let

    else if (From.front().size() > 1 && From.front().back() == '!')
    {
        // Erasure macro; Erases types or struct members from existence
        // Technically just marks them erased
        if (From.front() == "erase!")
        {
            int count = 0;
            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();
            std::list<std::string> contents;

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
                std::string symb = s;
                if ((s.front() == '"' && s.back() == '"') || (s.front() == '\'' && s.back() == '\''))
                {
                    // Trim quotes
                    symb = symb.substr(1, s.size() - 2);
                }

                bool didErase = false;

                if (settings.structData.count(symb) != 0)
                {
                    didErase = true;
                    settings.structData[symb].erased = true;
                }

                if (settings.enumData.count(symb) != 0)
                {
                    didErase = true;
                    settings.enumData[symb].erased = true;
                }

                if (settings.table.count(symb) != 0)
                {
                    didErase = true;
                    for (int l = 0; l < settings.table[symb].size(); l++)
                    {
                        settings.table[symb][l].erased = true;
                    }
                }

                if (!didErase)
                {
                    std::cout << tags::yellow_bold << "Warning: No symbols were erase!-ed via the symbol '" << symb
                              << "'\n"
                              << tags::reset;
                }
            }

            return out;
        }

        // Misc macros
        else if (From.front() == "c_print!")
        {
            std::cout << settings.curFile << ":" << settings.curLine << ":c_print! ";

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

                if (!(count == 1 && From.front() == "(") && !(count == 0 && From.front() == ")") && From.front() != ",")
                {
                    std::string cur = From.front();
                    std::cout << cleanMacroArgument(cur) << " ";
                }

                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.pop_front();

            } while (!From.empty() && count != 0);

            std::cout << "\n";

            return out;
        }
        else if (From.front() == "c_panic!")
        {
            std::string message = settings.curFile + ":" + std::to_string(settings.curLine) + ":c_panic! ";

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

                if (!(count == 1 && From.front() == "(") && !(count == 0 && From.front() == ")") && From.front() != ",")
                {
                    message += cleanMacroArgument(From.front()) + " ";
                }

                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.pop_front();
            } while (!From.empty() && count != 0);

            message += "\n";

            throw sequencing_error(message);
        }
        else if (From.front() == "c_warn!")
        {
            std::string message = settings.curFile + ":" + std::to_string(settings.curLine) + ":c_warn! ";

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

                if (!(count == 1 && From.front() == "(") && !(count == 0 && From.front() == ")") && From.front() != ",")
                {
                    message += cleanMacroArgument(From.front()) + " ";
                }

                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.pop_front();
            } while (!From.empty() && count != 0);

            message += "\n";

            std::cout << tags::yellow_bold << message << tags::reset;

            return out;
        }
        else if (From.front() == "c_sys!")
        {
            std::cout << settings.curFile << ":" << std::to_string(settings.curLine) << ":c_sys! ";

            std::string command;
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

                if (!(count == 1 && From.front() == "(") && !(count == 0 && From.front() == ")") && From.front() != ",")
                {
                    if (command != "")
                    {
                        command += " && ";
                    }

                    command += cleanMacroArgument(From.front());
                }

                sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
                From.pop_front();
            } while (!From.empty() && count != 0);

            std::cout << '`' << command << "`\n";
            int res = system(command.c_str()) != 0;

            if (res != 0)
            {
                std::cout << tags::red_bold << "Compile-time command failed w/ error code " << res << ".\n"
                          << tags::reset;
            }

            return out;
        }

        // else

        // Memory Keywords
        else if (From.front() == "alloc!")
        {
            int count = 0;
            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();
            std::list<Token> contents;

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

            std::string num = "1";

            ASTNode temp = __createSequence(contents, settings);
            std::string name = toC(temp, settings);

            while (contents.front() == ",")
            {
                contents.pop_front();
            }

            ASTNode numSeq = __createSequence(contents, settings);
            num = toC(numSeq, settings);

            Type tempType = temp.type;
            Type numType = numSeq.type;

            sm_assert(tempType.size() > 0, "'alloc!' received a malformed first argument.");

            temp.type.pop_front();

            if (numType == nullType)
            {
                num = "1";
                numType = TypeNode{atomic, "u128"};
            }

            sm_assert(tempType.size() > 0 && (tempType[0].info == arr || (tempType[0].info == pointer && num == "1")),
                      "'alloc!' returns an unsized array, or pointer if no number was provided.");

            sm_assert(numType[0].info == atomic && numType[0].name == "u128",
                      "'alloc!' takes 'u128', not '" + toStr(&numType) + "'.");

            out = getAllocSequence(temp.type, name, settings, num);

            return out;
        }
        else if (From.front() == "free!")
        {
            int count = 0;
            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();
            std::list<Token> contents;

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

            ASTNode temp = __createSequence(contents, settings);
            std::string name = toC(temp, settings);

            Type tempType = temp.type;
            sm_assert(tempType[0].info == arr || tempType[0].info == pointer,
                      "'free!' takes an unsized array or pointer.");
            temp.type.pop_front();

            out = getFreeSequence(name, settings);

            return out;
        }
        else if (From.front() == "ptrcpy!")
        {
            int count = 0;
            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();
            std::list<Token> contents;

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

            ASTNode lhsSeq = __createSequence(contents, settings);
            std::string lhs = toC(lhsSeq, settings);

            while (contents.front() == ",")
            {
                contents.pop_front();
            }

            ASTNode rhsSeq = __createSequence(contents, settings);
            std::string rhs = toC(rhsSeq, settings);

            sm_assert(lhsSeq.type[0].info == pointer || lhsSeq.type[0].info == arr,
                      "First argument of ptrcpy! must be pointer or unsized array.");
            sm_assert(rhsSeq.type[0].info == pointer || rhsSeq.type[0].info == arr || rhsSeq.raw == "0",
                      "Second argument of ptrcpy! must be a pointer, unsized array, or 0.");

            out.info = code_line;
            out.type = nullType;
            out.items.clear();

            out.items.push_back(ASTNode{nullType, std::vector<ASTNode>(), atom, lhs + " = (void*)(" + rhs + ")"});

            return out;
        }
        else if (From.front() == "ptrarr!")
        {
            int count = 0;
            sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
            From.pop_front();
            std::list<Token> contents;

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

            ASTNode lhsSeq = __createSequence(contents, settings);
            std::string lhs = toC(lhsSeq, settings);

            while (contents.front() == ",")
            {
                contents.pop_front();
            }

            ASTNode rhsSeq = __createSequence(contents, settings);
            std::string rhs = toC(rhsSeq, settings);

            sm_assert(lhsSeq.type[0].info == pointer || lhsSeq.type[0].info == arr || lhsSeq.type[0].info == sarr,
                      "First argument of ptrarr! must be pointer or arr.");
            sm_assert(rhsSeq.type.size() == 1 && rhsSeq.type[0].info == atomic,
                      "second argument of ptrarr! must be an atomic.");

            std::string t = rhsSeq.type[0].name;
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
        auto toReturn = __createSequence(From, settings);
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
        out.items.push_back(__createSequence(From, settings));

        sm_assert(out.items[0].type[0].info == atomic && settings.enumData.count(out.items[0].type[0].name) != 0,
                  out.raw + " statement argument must be enum. Instead, '" + toStr(&out.items[0].type) + "'");
        sm_assert(!From.empty(), "Missing statement after " + out.raw + "");

        std::string old = settings.prevMatchTypeStr;
        settings.prevMatchTypeStr = out.items[0].type[0].name;

        // This is for the code chunk
        out.items.push_back(__createSequence(From, settings));

        settings.prevMatchTypeStr = old;

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
        sm_assert(settings.prevMatchTypeStr != "NULL", "'case' statement must occur within a 'match' statement.");

        // Takes a code scope / code line
        out.info = enum_keyword;
        out.raw = From.front();
        out.type = nullType;

        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        sm_assert(!From.empty(), "'case' must be followed by enumeration option name.");
        out.items.push_back(ASTNode{nullType, std::vector<ASTNode>{}, atom, From.front()});
        std::string optionName = From.front();

        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        sm_assert(!From.empty() && From.front() == "(",
                  "Enumeration option must be followed by capture parenthesis (IE name(capture_here)).");
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        sm_assert(!From.empty(), "Capture group is missing name or closing parenthesis.");
        std::string captureName = "NULL";

        if (From.front() != ")")
        {
            captureName = From.front();
            out.items.push_back(ASTNode{nullType, std::vector<ASTNode>{}, atom, From.front()});
            From.pop_front();

            settings.table[captureName].push_back(MultiTableSymbol{ASTNode{}, pointer, false, settings.curFile});
            settings.table[captureName].back().type.append(
                settings.enumData[settings.prevMatchTypeStr].options[optionName]);
        }
        else
        {
            // Padding so that this spot will always refer to the capture variable
            out.items.push_back(ASTNode{nullType, std::vector<ASTNode>{}, atom, "NULL"});
        }

        sm_assert(!From.empty() && From.front() == ")", "Capture parenthesis must contain at most one symbol.");

        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        // Get actual code scope
        out.items.push_back(__createSequence(From, settings));

        // Remove capture group from Oak table if needed
        if (captureName != "NULL")
        {
            settings.table[captureName].pop_back();
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

        out.items.push_back(__createSequence(From, settings));

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
        out.items.push_back(__createSequence(From, settings));

        sm_assert(out.items[0].type == Type(atomic, "bool"),
                  out.raw + " statement argument must be boolean. Instead, '" + toStr(&out.items[0].type) + "'");
        sm_assert(!From.empty(), "Missing statement after " + out.raw + "");

        // This is for the code chunk
        out.items.push_back(__createSequence(From, settings));

        if (out.items.back().info == atom)
        {
            out.items.back().raw.push_back(';');
        }
        else if (out.items.back().info == code_line)
        {
            out.items.back().items.push_back(ASTNode{nullType, std::vector<ASTNode>(), atom, ";"});
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

        out.items.push_back(__createSequence(From, settings));

        return out;
    }
    else if (From.front() == "return")
    {
        // Takes a code scope / code line
        out.info = code_line;
        out.type = nullType;
        out.items.push_back(ASTNode{nullType, std::vector<ASTNode>(), keyword, "return"});

        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        sm_assert(!From.empty() && From.front() != ";" && From.front() != "}",
                  "'return' must be followed by the thing to return (void returns are not legal).");
        out.items.push_back(__createSequence(From, settings));

        int garbage = 0;
        sm_assert(typesAreSameCast(&out.items.back().type, &settings.currentReturnType, garbage),
                  "Cannot return '" + toStr(&out.items.back().type) + "' from a function w/ return type '" +
                      toStr(&settings.currentReturnType) + "'");

        return out;
    }

    else if (From.front() == "{")
    {
        sm_assert(!From.empty(), "Cannot pop from front of empty vector.");
        From.pop_front();

        // Save symbol table for later restoration
        auto oldTable = settings.table;

        out.info = code_scope;

        // Code scope.
        int count = 1;
        std::list<Token> curVec;
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
                    out.items.push_back(__createSequence(curVec, settings));
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
                    out.items.push_back(__createSequence(curVec, settings));
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

        // Fetch destructors
        auto destructors = restoreSymbolTable(oldTable, settings.table);
        insertDestructors(out, destructors, settings);

        // If a void function, call all destructors
        // (otherwise, these calls will have been handled elsewhere)
        if (settings.currentReturnType == Type(atomic, "void"))
        {
            for (const auto &p : destructors)
            {
                out.items.push_back(ASTNode{nullType, std::vector<ASTNode>(), atom, p.second + ";"});
            }
        }

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

            if (out.items[i].type != nullType)
            {
                int garbage = 0;
                sm_assert(typesAreSameCast(&out.items[i].type, &settings.currentReturnType, garbage),
                          "Cannot return '" + toStr(&out.items[i].type) + "' from a function w/ return type '" +
                              toStr(&settings.currentReturnType) + "'");
            }
        }

        return out;
    } // scope

    // Non-special case; code line.
    // Function calls and template instantiation may occur within.

    ASTNode temp;
    temp.info = atom;

    int i = 0;

    std::vector<Token> tempVec;
    for (auto i : From)
    {
        tempVec.push_back(i);

        if (i == ";")
        {
            break;
        }
    }
    temp.type = resolveFunction(tempVec, i, temp.raw, settings);

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
Type resolveFunctionInternal(const std::vector<Token> &What, int &start, std::vector<std::string> &c,
                             AcornSettings &settings)
{
    if (What.empty() || start >= What.size())
    {
        return nullType;
    }

    // std::cout << "resolveFunctionInternal called on " << What[start].text << " " << What[start].file << ":"
    //           << What[start].line << '\n';

    // Pointer check
    if (What[start] == "@")
    {
        start++;
        c.push_back("&");

        std::string name = What[start];

        std::string followingC = "";
        Type type = resolveFunction(What, start, followingC, settings);

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

        Type type = resolveFunctionInternal(What, start, c, settings);

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

    // Standard case

    // get name (first item)
    std::string name = What[start];
    Type type = nullType;

    // Parenthesis
    if (What[start] == "(")
    {
        std::vector<Token> toUse;
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
        Type toReturn = resolveFunctionInternal(toUse, trash, c, settings);

        if (!c.empty())
        {
            c.front().insert(c.front().begin(), '(');
        }
        else
        {
            c.push_back("(");
        }
        c.push_back(")");

        return toReturn;
    }

    // Template instantiation
    bool didTemplate = false;
    if (What.size() > start + 1 && What[start + 1] == "<")
    {
        start++;

        std::vector<std::string> curGen;
        std::vector<std::vector<std::string>> generics;
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
        std::vector<std::string> typeVec;

        while (start < What.size() && What[start] != ";")
        {
            typeVec.push_back(What[start]);
            start++;
        }

        if (typeVec.empty())
        {
            typeVec.push_back("struct");
        }

        Type oldType = settings.currentReturnType;
        settings.currentReturnType = nullType;

        auto oldDepth = depth;
        depth = 0;

        std::string result = instantiateGeneric(name, generics, typeVec, settings);

        depth = oldDepth;
        settings.currentReturnType = oldType;

        start--;

        didTemplate = true;
    }

    // Append literal C code, without interpretation from Oak
    if (What[start] == "raw_c!")
    {
        // Scrape call to a std::vector
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

        // Scrape entire size!(what) call to a std::vector
        std::vector<Token> toAnalyze;
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
        std::string junk = "";
        int pos = 0;

        // Analyze type of collected
        Type type = resolveFunction(toAnalyze, pos, junk, settings);

        // Append size
        c.push_back("sizeof(");
        c.push_back(toStrC(&type, settings));
        c.push_back(")");

        return Type(atomic, "u128");
    }

    else if (What[start].back() == '!')
    {
        // Otherwise unspecified macro

        // Scrape entire call to a std::vector
        std::list<Token> toAnalyze = {Token(What[start]), Token("(")};
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
        toAnalyze.push_back(Token(")"));

        // Analyze type of collected
        ASTNode s = __createSequence(toAnalyze, settings);

        c.push_back(toC(s, settings));

        return s.type;
    }

    // Function call
    if (What.size() > start + 1 && What[start + 1] == "(")
    {
        // get args within parenthesis
        std::vector<Token> curArg;
        std::vector<std::vector<Token>> args;

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

        std::vector<Type> argTypes;
        std::vector<std::string> argStrs;
        for (std::vector<Token> arg : args)
        {
            int trash = 0;
            std::string cur;

            argTypes.push_back(resolveFunction(arg, trash, cur, settings));
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
            // Return type is the thing the array is of
            type = Type(argTypes[0], 1);
            c.push_back("(*(" + argStrs[0] + "+" + argStrs[1] + "))");
        }

        // Special case: Pointer nullification
        else if (name == "Copy" && argTypes[0][0].info == pointer && argStrs[1] == "0")
        {
            // Return type is the thing the array is of
            type = argTypes[0];
            c.push_back("(" + argStrs[0] + "=0)");
        }

        else
        {
            // Search for candidates
            sm_assert(settings.table.count(name) != 0, "Function call '" + name + "' has no registered symbols.");
            sm_assert(settings.table[name].size() != 0, "Function call '" + name + "' has no registered symbols.");

            std::vector<MultiTableSymbol> candidates = settings.table[name];

            // Construct candArgs
            std::vector<std::vector<Type>> candArgs;
            for (auto item : candidates)
            {
                Type curType = item.type;
                while (curType != nullType && curType[0].info == pointer)
                {
                    curType.pop_front();
                }

                auto args = getArgs(item.type, settings);
                candArgs.push_back(std::vector<Type>());

                for (auto arg : args)
                {
                    if (arg.second == nullType)
                    {
                        continue;
                    }

                    candArgs.back().push_back(arg.second);
                }
            }

            std::vector<int> validCandidates;

            // Do stages of candidacy
            validCandidates = getExactCandidates(candArgs, argTypes);

            if (validCandidates.size() == 0)
            {
                validCandidates = getReferenceCandidates(candArgs, argTypes);

                if (validCandidates.size() != 1)
                {
                    // This also does references
                    validCandidates = getCastingCandidates(candArgs, argTypes);
                }
            }
            else if (validCandidates.size() > 1)
            {
                // Ambiguous candidates

                std::cout << tags::red_bold << "Error: Cannot override function call '" << name
                          << "' on return type alone.\n"
                          << tags::reset;

                printCandidateErrors(candidates, argTypes, name, settings);

                throw sequencing_error("Cannot override function call '" + name + "' on return type alone");
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

                std::cout << tags::red_bold << "Error: No viable candidates for function call '" << name << "'.\n"
                          << tags::reset;

                printCandidateErrors(candidates, argTypes, name, settings);

                throw sequencing_error("No viable candidates for function call '" + name + "'");
            }

            // Use candidate at front; This is highest-priority one
            type = getReturnType(candidates[validCandidates[0]].type, settings);

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

                auto candArgs = getArgs(candidates[validCandidates[0]].type, settings);

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

                // If arg is str and cand is i8
                if (argCursor < argTypes[j].size() && argTypes[j][argCursor].info == atomic &&
                    argTypes[j][argCursor].name == "str" && candCursor < candArgs[j].second.size() &&
                    candArgs[j].second[candCursor].info == atomic && candArgs[j].second[candCursor].name == "i8")
                {
                    argCursor++;
                    numDeref++;
                }

                // Else if arg is i8 and cand is str
                else if (argCursor < argTypes[j].size() && argTypes[j][argCursor].info == atomic &&
                         argTypes[j][argCursor].name == "i8" && candCursor < candArgs[j].second.size() &&
                         candArgs[j].second[candCursor].info == atomic && candArgs[j].second[candCursor].name == "str")
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
                    c.push_back("&");
                }
                else if (numDeref != 0)
                {
                    std::cout << "With candidates:\n";
                    for (auto item : candidates)
                    {
                        std::cout << std::left << name << std::setw(40) << toStr(&item.type) << "from "
                                  << item.sourceFilePath << '\n';
                    }
                    std::cout << '\n';

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
        std::string litName = What[start];

        if (litType == nullType)
        {
            // Is not a literal

            // Some common issues
            sm_assert(What[start] != "let", "Invalid use of `let` keyword. This is most likely a rule issue.");
            sm_assert(What[start] != "pre", "`pre` may only be used on generic definitions.");
            sm_assert(What[start] != "post", "`post` may only be used on generic definitions.");

            sm_assert(settings.table.count(What[start]) != 0,
                      "No definitions exist for symbol '" + What[start].text + "'.");
            auto candidates = settings.table[What[start]];
            sm_assert(candidates.size() != 0, "No definitions exist for symbol '" + What[start].text + "'.");

            type = candidates.back().type;
        }
        else
        {
            // Is a literal
            type = litType;

            if (litName.size() >= litType[0].name.size() &&
                litName.substr(litName.size() - litType[0].name.size()) == litType[0].name)
            {
                litName.erase(litName.size() - litType[0].name.size());
            }

            if (litType == Type(atomic, "u32"))
            {
                litName += "U";
            }
            else if (litType == Type(atomic, "i64"))
            {
                litName += "UL";
            }
            else if (litType == Type(atomic, "u64"))
            {
                litName += "UL";
            }
            else if (litType == Type(atomic, "i128"))
            {
                litName += "ULL";
            }
            else if (litType == Type(atomic, "u128"))
            {
                litName += "ULL";
            }
        }

        c.push_back(litName);
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

        std::string structName;
        try
        {
            sm_assert(type[0].info == atomic,
                      "Error during type trimming for member access; Could not find struct name.");
            structName = type[0].name;

            sm_assert(settings.structData.count(structName) != 0, "Struct type '" + structName + "' does not exist.");
            sm_assert(!settings.structData[structName].erased,
                      "Struct '" + structName + "' exists, but is erased (private).");
            sm_assert(settings.structData[structName].members.count(What[start + 2]) != 0,
                      "Struct '" + structName + "' has no member '" + What[start + 2].text + "'.");
        }
        catch (sequencing_error &e)
        {
            std::cout << tags::yellow_bold << "In context: '";

            int pos = start - 8;
            if (pos < 0)
            {
                pos = 0;
            }

            for (; pos < What.size() && pos < start + 8; pos++)
            {
                std::cout << What[pos].text << (pos == start + 7 ? "" : " ");
            }

            std::cout << "'\n" << tags::reset;

            throw e;
        }

        type = settings.structData[structName].members[What[start + 2]];

        c.push_back(".");
        c.push_back(What[start + 2]);

        start += 2;
    }

    start++;

    // Return function return type
    return type;
}

Type resolveFunction(const std::vector<Token> &What, int &start, std::string &c, AcornSettings &settings)
{
    std::vector<std::string> cVec;

    if (c != "")
    {
        cVec.push_back(c);
    }

    Type out = resolveFunctionInternal(What, start, cVec, settings);

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
