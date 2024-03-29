/*
Jordan Dehmel
2023 - present
jdehmel@outlook.com
*/

#include "oakc_fns.hpp"

#define rm_assert(expression, message)                                                                                 \
    ((bool)(expression) ? true : throw rule_error(message " (Failed assertion: '" #expression "')"))

// I is the point in Lexed at which a macro name was found
// CONSUMPTIVE!
std::list<std::string> getMacroArgs(std::list<Token> &lexed, std::list<Token>::iterator &it)
{
    std::list<std::string> out;

    it = lexed.erase(it);

    if (*it != "(")
    {
        throw std::runtime_error("Internal error; Malformed call to getMacroArgs.");
    }

    // Erase opening parenthesis
    it = lexed.erase(it);

    std::string cur = "";
    int count = 1;

    while (!lexed.empty() && *it != ";")
    {
        if (*it == "(")
        {
            count++;
        }
        else if (*it == ")")
        {
            count--;
        }

        if ((*it == "," && count == 1) || (*it == ")" && count == 0))
        {
            out.push_back(cur);
            cur = "";
        }
        else
        {
            if (cur != "")
            {
                cur.push_back(' ');
            }

            cur += *it;
        }

        it = lexed.erase(it);
    }

    return out;
}

void doRules(std::list<Token> &From, AcornSettings &settings)
{
    if (settings.doRuleLogFile)
    {
        settings.ruleLogFile.open(".oak_build/__rule_log.log", std::ios::out | std::ios::app);
        if (!settings.ruleLogFile.is_open())
        {
            throw rule_error("Failed to open rule log file '.oak_build/__rule_log.log'");
        }

        settings.ruleLogFile << "\n\n/////////// In file " << settings.curFile << " ///////////\n\n";
    }

    for (auto it = From.begin(); it != From.end(); it++)
    {
        // Add a new rule to the list of all rules
        if (*it == "new_rule!")
        {
            auto args = getMacroArgs(From, it);
            for (auto &j : args)
            {
                rm_assert(j.size() >= 2, "Rule macro argument is of insufficient size.");
                rm_assert((j.front() == '"' && j.back() == '"') || (j.front() == '\'' && j.back() == '\''),
                          "All rule macro arguments must be strings.");
                j.pop_back();
                j.erase(0, 1);
            }

            rm_assert(args.size() == 3 || args.size() == 4, "new_rule! requires three or four arguments.");

            std::string name = args.front();
            Rule toAdd;
            Lexer dfa_lexer;

            auto argIter = args.begin();
            argIter++; // Now on 1

            auto lexed = dfa_lexer.lex_list(*argIter);
            toAdd.inputPattern = {lexed.begin(), lexed.end()};
            argIter++; // Now on 2

            lexed = dfa_lexer.lex_list(*argIter);
            toAdd.outputPattern = {lexed.begin(), lexed.end()};
            argIter++; // Now on 3

            if (args.size() == 3)
            {
                // Default: Use sapling
                toAdd.engineName = "sapling";
            }
            else if (args.size() == 4)
            {
                // Custom rule engine: Look up in engine table

                rm_assert(settings.engines.count(*argIter) != 0,
                          "Failed to load unknown rule engine '" + *argIter + "'");
                toAdd.engineName = *argIter;
            }

            settings.rules[name] = toAdd;
        }

        // Use a rule that already exists
        else if (*it == "use_rule!")
        {
            // use_rule!("NAME", "NAME", ...);

            auto args = getMacroArgs(From, it);
            for (auto &j : args)
            {
                rm_assert(j.size() >= 2, "Rule macro argument is of insufficient size.");
                rm_assert((j.front() == '"' && j.back() == '"') || (j.front() == '\'' && j.back() == '\''),
                          "All rule macro arguments must be strings.");
                j.pop_back();
                j.erase(0, 1);
            }

            rm_assert(args.size() > 0, "use_rule! requires at least one argument.");

            for (auto arg : args)
            {
                rm_assert(settings.rules.count(arg) != 0 || settings.bundles.count(arg) != 0,
                          "Rule '" + arg + "' does not exist and is not a bundle.");

                // bundles override rules with the same name
                if (settings.bundles.count(arg) != 0)
                {
                    for (auto r : settings.bundles[arg])
                    {
                        settings.activeRules.push_back(r);
                    }
                }
                else
                {
                    settings.activeRules.push_back(arg);
                }
            }
        }

        // Stop using a rule that is in use
        else if (*it == "rem_rule!")
        {
            // rem_rule!("NAME", "NAME", ...);

            auto args = getMacroArgs(From, it);
            for (auto &j : args)
            {
                rm_assert(j.size() >= 2, "Rule macro argument is of insufficient size.");
                rm_assert((j.front() == '"' && j.back() == '"') || (j.front() == '\'' && j.back() == '\''),
                          "All rule macro arguments must be strings.");
                j.pop_back();
                j.erase(0, 1);
            }

            if (args.size() == 0)
            {
                settings.activeRules.clear();
            }

            for (auto arg : args)
            {
                bool found = false;
                for (int j = settings.activeRules.size() - 1; j >= 0; j--)
                {
                    if (settings.activeRules[j] == arg)
                    {
                        found = true;
                        settings.activeRules.erase(settings.activeRules.begin() + j);
                        break;
                    }
                }

                rm_assert(found, "Rule '" + arg + "' is not in use.");
            }
        }

        // Bundle multiple rules into one
        else if (*it == "bundle_rule!")
        {
            // bundle_rule!("BUNDLE_NAME", "NAME", ...);

            auto args = getMacroArgs(From, it);
            for (auto &j : args)
            {
                rm_assert(j.size() >= 2, "Rule macro argument is of insufficient size.");
                rm_assert((j.front() == '"' && j.back() == '"') || (j.front() == '\'' && j.back() == '\''),
                          "All rule macro arguments must be strings.");
                j.pop_back();
                j.erase(0, 1);
            }

            rm_assert(args.size() > 1, "bundle_rule! requires at least two arguments.");

            std::string name = args.front();
            for (auto it = args.begin(); it != args.end(); it++)
            {
                if (it == args.begin())
                {
                    continue;
                }

                settings.bundles[name].push_back(*it);
            }
        }

        // Regular case; Non-rule-macro symbol. Check against active rules.
        else
        {
            for (int ruleIndex = 0; ruleIndex < settings.dialectRules.size() + settings.activeRules.size(); ruleIndex++)
            {
                Rule curRule;
                if (ruleIndex < settings.dialectRules.size())
                {
                    if (settings.rules.count(settings.dialectRules[ruleIndex]) == 0)
                    {
                        continue;
                    }

                    curRule = settings.rules[settings.dialectRules[ruleIndex]];
                }
                else
                {
                    if (settings.rules.count(settings.activeRules[ruleIndex - settings.dialectRules.size()]) == 0)
                    {
                        continue;
                    }

                    curRule = settings.rules[settings.activeRules[ruleIndex - settings.dialectRules.size()]];
                }

                if (!itIsInRange(From, it, curRule.inputPattern.size()))
                {
                    continue;
                }

                // do rule here
                if (curRule.engineName == "sapling")
                {
                    doRuleAcorn(From, it, curRule, settings);
                }
                else if (settings.engines.count(curRule.engineName) == 0)
                {
                    throw rule_error("Unknown rule engine '" + curRule.engineName + "'");
                }
                else
                {
                    settings.engines[curRule.engineName](From, it, curRule, settings);
                }
            }
        }
    }

    // cout << "Done.\n";

    if (settings.doRuleLogFile && settings.ruleLogFile.is_open())
    {
        settings.ruleLogFile.close();
    }

    return;
}

void loadDialectFile(const std::string &File, AcornSettings &settings)
{
    if (settings.dialectLock)
    {
        return;
    }

    /*
    // comment
    [engine]
    clear
    "in" "out"
    final
    */

    // Open dialect file
    std::ifstream inp(File);
    if (!inp.is_open())
    {
        throw rule_error("Failed to open dialect file '" + File + "'");
    }

    std::string engine = "sapling";

    std::string line;
    int lineNum = 0;
    while (getline(inp, line))
    {
        lineNum++;

        if (line.size() == 0)
        {
            continue;
        }

        if (line.size() > 1 && line.front() == '[' && line.back() == ']')
        {
            // Engine change flag

            engine = line.substr(1, line.size() - 2);

            if (settings.engines.count(engine) == 0)
            {
                throw rule_error("Error: Unknown rule engine '" + engine + "'");
            }

            continue;
        }

        std::stringstream lineStream(line);
        std::string cur;

        lineStream >> cur;
        if (cur.size() != 0 && (cur.front() == '"' || cur.front() == '\''))
        {
            // Remember that all args must be strings, even here

            std::string name;
            std::string inputStr, outputStr;
            Rule toAdd;

            name = "dialect_rule_" + std::to_string(settings.dialectRules.size());

            // Insert into dialectRules list
            settings.dialectRules.push_back(name);

            // Get input pattern
            inputStr = cur;
            while (!inputStr.empty() && (inputStr.back() != inputStr.front()))
            {
                lineStream >> cur;
                inputStr += " " + cur;
            }
            inputStr = inputStr.substr(1, inputStr.size() - 2);

            // Get output pattern
            lineStream >> outputStr;
            while (!outputStr.empty() && (outputStr.back() != outputStr.front()))
            {
                lineStream >> cur;
                outputStr += " " + cur;
            }
            outputStr = outputStr.substr(1, outputStr.size() - 2);

            // Lex patterns
            Lexer dfa_lexer;
            rm_assert(inputStr != "", "Input rule must be non-empty.");

            auto lexed = dfa_lexer.lex_list(inputStr);
            toAdd.inputPattern = {lexed.begin(), lexed.end()};

            if (outputStr != "")
            {
                lexed = dfa_lexer.lex_list(outputStr);

                toAdd.outputPattern = {lexed.begin(), lexed.end()};
            }

            // Fetch engine based on current
            toAdd.engineName = engine;

            // Insert into rules list
            settings.rules[name] = toAdd;
        }
        else if (cur == "clear")
        {
            // Clear all existing dialect rules

            // Erase all existing dialect rules from existence
            for (auto s : settings.dialectRules)
            {
                settings.rules.erase(s);
            }

            // Clear list of dialect rules
            settings.dialectRules.clear();
        }
        else if (cur == "final")
        {
            // Set the current dialect as final
            // (no more can ever be loaded in this translation)
            settings.dialectLock = true;
            break;
        }
        else
        {
            throw rule_error(File + ":" + std::to_string(lineNum) + " Invalid line '" + line + "'");
        }
    }

    // Close dialect file
    inp.close();
    return;
}

void doRuleAcorn(std::list<Token> &From, std::list<Token>::iterator &i, Rule &curRule, AcornSettings &settings)
{
    auto posInFrom = i;
    std::list<std::string> memory;
    std::map<std::string, std::string> ruleVars;
    bool isMatch = true;
    for (int k = 0; posInFrom != From.end() && k < curRule.inputPattern.size(); k++)
    {
        std::string match = curRule.inputPattern[k];

        if (posInFrom == From.end())
        {
            isMatch = false;
            break;
        }

        // Macro; Cannot overwrite with rules
        if (posInFrom->size() > 1 && posInFrom->back() == '!')
        {
            isMatch = false;
            break;
        }

        // Wildcard; Un-stored match
        if (match == "$$")
        {
            memory.push_back(*posInFrom);

            posInFrom++;
            continue;
        }

        // Globs; Un-stored multi-symbol matches
        else if (match == "$*")
        {
            rm_assert(k + 1 < curRule.inputPattern.size(), "Glob card '$*' must not be end of rule.");
            std::string nextSymb = curRule.inputPattern[k + 1];

            while (*posInFrom != nextSymb)
            {
                memory.push_back(*posInFrom);
                posInFrom++;

                if (posInFrom == From.end())
                {
                    isMatch = false;
                    break;
                }
            }

            continue;
        }
        else if (match == "$+")
        {
            rm_assert(k + 1 < curRule.inputPattern.size(), "Glob card '$+' must not be end of rule.");
            std::string nextSymb = curRule.inputPattern[k + 1];

            if (*posInFrom == nextSymb)
            {
                isMatch = false;
            }

            while (*posInFrom != nextSymb)
            {
                memory.push_back(*posInFrom);
                posInFrom++;

                if (posInFrom == From.end())
                {
                    isMatch = false;
                    break;
                }
            }

            continue;
        }

        // Memory clear
        else if (match == "$~")
        {
            memory.clear();
            continue;
        }

        // Pipe memory onto end of variable
        else if (match.size() == 3 && match.substr(0, 2) == "$>")
        {
            std::string name = match.substr(0, 1) + match.substr(2, 1);

            std::string concatenatedMemory = "";
            for (const std::string &item : memory)
            {
                concatenatedMemory.append(item);
                concatenatedMemory.append(" ");
            }

            if (ruleVars.count(name) == 0)
            {
                ruleVars[name] += " " + concatenatedMemory;
            }
            else
            {
                ruleVars[name] = concatenatedMemory;
            }

            continue;
        }

        // Go back to previous match item unless literal
        else if (match.size() > 2 && match.substr(0, 2) == "$-")
        {
            if (*posInFrom == match.substr(2))
            {
                k--;
            }
        }

        // Negative lookbehind
        // Not prefixed by some literal
        else if (match.size() > 4 && match.substr(0, 4) == "$/<$")
        {
            // PosInFrom does not advance upon success
            if (itCmp(From, posInFrom, -1, match.substr(4)))
            {
                isMatch = false;
                break;
            }

            continue;
        }

        // Negative lookahead
        // Not followed by some literal
        else if (match.size() > 4 && match.substr(0, 4) == "$/>$")
        {
            // PosInFrom does not advance upon success
            if (itCmp(From, posInFrom, 1, match.substr(4)))
            {
                isMatch = false;
                break;
            }

            continue;
        }

        // Suite
        // $[$suite$of$cards$]
        else if (match.size() > 3 && match.substr(0, 3) == "$[$")
        {
            // Matches any of the items within

            int start, end;
            end = 3;

            while (end < match.size())
            {
                // Advance end to the next dollar sign
                start = end;
                do
                {
                    end++;
                } while (end < match.size() && match[end] != '$');

                if (end >= match.size())
                {
                    isMatch = false;
                    break;
                }

                // Do actual compare here
                // If matches, isMatch. Otherwise, keep going.
                if (match.substr(start, end - start) == posInFrom->text)
                {
                    posInFrom++;
                    break;
                }
            }

            memory.push_back(*posInFrom);
        }

        // Negated suite
        // $/[$negated$suite$of$cards$]
        else if (match.size() > 4 && match.substr(0, 4) == "$/[$")
        {
            // Matches anything but the items within

            int start, end;
            end = 4;

            while (end < match.size())
            {
                // Advance end to the next dollar sign
                start = end;
                do
                {
                    end++;
                } while (end < match.size() && match[end] != '$');

                if (end >= match.size())
                {
                    posInFrom++;
                    break;
                }

                // Do actual compare here
                // If matches, is not match. Otherwise, keep going.
                if (match.substr(start, end - start) == posInFrom->text)
                {
                    isMatch = false;
                    break;
                }
            }

            memory.push_back(*posInFrom);
        }

        // $<$open$close$>
        // Pair matching
        else if (match.size() > 7 && match.substr(0, 3) == "$<$")
        {
            int endOfOpener = 3;
            while (endOfOpener < match.size() && match[endOfOpener] != '$')
            {
                endOfOpener++;
            }

            int endOfCloser = endOfOpener + 1;
            while (endOfCloser < match.size() && match[endOfCloser] != '$')
            {
                endOfCloser++;
            }

            std::string opener = match.substr(3, endOfOpener - 3),
                        closer = match.substr(endOfOpener + 1, endOfCloser - endOfOpener - 1);
            long long count = 0;

            auto beginningPosition = posInFrom;

            do
            {
                if (posInFrom == From.end())
                {
                    // Out of range w/o closure: Failure case
                    isMatch = false;
                    break;
                }

                memory.push_back(*posInFrom);

                if (*posInFrom == opener)
                {
                    count++;
                }
                else if (*posInFrom == closer)
                {
                    count--;
                }

                posInFrom++;
            } while (posInFrom != From.end() && count != 0);

            if (!isMatch || posInFrom == beginningPosition || posInFrom == std::next(beginningPosition))
            {
                isMatch = false;
                break;
            }

            continue;
        }

        // Variable glob; Stored multi-symbol match
        else if (match.size() == 3 && match.substr(0, 2) == "$*")
        {
            std::string name = match.substr(0, 1) + match.substr(2, 1);

            rm_assert(k + 1 < curRule.inputPattern.size(), "Glob card '$*' must not be end of rule.");
            std::string nextSymb = curRule.inputPattern[k + 1];

            while (posInFrom != From.end() && *posInFrom != nextSymb)
            {
                if (ruleVars.count(name) == 0)
                {
                    ruleVars[name] += *posInFrom;
                }
                else
                {
                    ruleVars[name] += " " + posInFrom->text;
                }

                memory.push_back(*posInFrom);

                posInFrom++;
                if (posInFrom == From.end())
                {
                    isMatch = false;
                    break;
                }
            }

            continue;
        }

        // Variable; Stored match
        else if (match[0] == '$')
        {
            if (ruleVars.count(match) == 0)
            {
                ruleVars[match] = *posInFrom;
            }
            else
            {
                ruleVars[match] += " " + posInFrom->text;
            }

            memory.push_back(*posInFrom);

            posInFrom++;
        }

        // Literal; Must match verbatim
        else if (match == posInFrom->text)
        {
            posInFrom++;
            memory.push_back(*posInFrom);

            continue;
        }

        // Failure case
        else
        {
            isMatch = false;
            break;
        }
    }

    // If match, do replacement
    if (isMatch)
    {
        // Variable table is already built at this point

        // Get new contents
        std::list<Token> newContents;

        Token templ = *i;
        templ.text = "NULL";

        for (int sIndex = 0; sIndex < curRule.outputPattern.size(); sIndex++)
        {
            std::string s = curRule.outputPattern[sIndex];

            if (ruleVars.count(s) != 0)
            {
                std::string raw = ruleVars[s];

                Lexer dfa_lexer;
                std::list<Token> lexed = dfa_lexer.lex_list(raw);
                for (auto s : lexed)
                {
                    newContents.push_back(s);
                }
            }
            else if (s == "$<")
            {
                // Merge operator
                rm_assert(!newContents.empty(), "Cannot use merge card '$<' on empty output.");
                rm_assert(sIndex + 1 < curRule.outputPattern.size(), "Merge card '$<' must not be end of rule.");

                std::string toInsert = curRule.outputPattern[sIndex + 1];

                if (ruleVars.count(toInsert) != 0)
                {
                    Lexer dfa_lexer;
                    std::string raw = ruleVars[toInsert];
                    std::list<Token> lexed = dfa_lexer.lex_list(raw);

                    toInsert = "";
                    for (auto str : lexed)
                    {
                        toInsert.append(str);
                    }
                }

                if (sIndex + 1 < curRule.outputPattern.size())
                {
                    sIndex++;
                }

                newContents.back().text += toInsert;
            }
            else
            {
                templ.text = s;
                newContents.push_back(templ);
            }
        }

        // Log if needed
        if (settings.doRuleLogFile)
        {
            settings.ruleLogFile << "Sapling rule w/ Input pattern\t";

            for (const auto &what : curRule.inputPattern)
            {
                settings.ruleLogFile << "'" << what.text << "' ";
            }

            settings.ruleLogFile << "\nOutput pattern\t";

            for (const auto &what : curRule.outputPattern)
            {
                settings.ruleLogFile << "'" << what.text << "' ";
            }

            settings.ruleLogFile << "\nMatch '";
        }

        // Erase old contents
        templ = *i;
        while (i != posInFrom)
        {
            if (settings.doRuleLogFile)
            {
                settings.ruleLogFile << i->text << ' ';
            }
            i = From.erase(i);
        }

        // Correct lines and files
        for (auto it = newContents.begin(); it != newContents.end(); it++)
        {
            it->file = templ.file;
            it->line = templ.line;
        }

        if (settings.doRuleLogFile)
        {
            settings.ruleLogFile << "'\nIs now '";

            for (const auto &what : newContents)
            {
                settings.ruleLogFile << what.text << ' ';
            }

            settings.ruleLogFile << "'\n\n";
        }

        // Insert new contents
        i = From.insert(i, newContents.begin(), newContents.end());
    }

    ruleVars.clear();
}

void addEngine(const std::string &name,
               void (*hook)(std::list<Token> &, std::list<Token>::iterator &, Rule &, AcornSettings &),
               AcornSettings &settings)
{
    settings.engines[name] = hook;
    return;
}

#undef rm_assert
