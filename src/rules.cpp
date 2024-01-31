/*
Jordan Dehmel
2023 - present
jdehmel@outlook.com
*/

#include "rules.hpp"

#define rm_assert(expression, message)                                                                                 \
    ((bool)(expression) ? true : throw rule_error(message " (Failed assertion: '" #expression "')"))

std::map<std::string, Rule> rules;
std::vector<std::string> activeRules, dialectRules;
std::map<std::string, std::vector<std::string>> bundles;
std::map<std::string, void (*)(std::vector<Token> &, int &, Rule &)> engines;

bool doRuleLogFile = false;
std::ofstream ruleLogFile;

// I is the point in Lexed at which a macro name was found
// CONSUMPTIVE!
std::vector<std::string> getMacroArgs(std::vector<Token> &lexed, const int &i)
{
    std::vector<std::string> out;

    lexed.erase(lexed.begin() + i);

    if (lexed[i] != "(")
    {
        throw std::runtime_error("Internal error; Malformed call to getMacroArgs.");
    }

    // Erase opening parenthesis
    lexed.erase(lexed.begin() + i);

    std::string cur = "";
    int count = 1;

    while (!lexed.empty() && lexed[i] != ";")
    {
        if (lexed[i] == "(")
        {
            count++;
        }
        else if (lexed[i] == ")")
        {
            count--;
        }

        if ((lexed[i] == "," && count == 1) || (lexed[i] == ")" && count == 0))
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

            cur += lexed[i];
        }

        lexed.erase(lexed.begin() + i);
    }

    return out;
}

void doRules(std::vector<Token> &From)
{
    if (doRuleLogFile)
    {
        ruleLogFile.open(".oak_build/__rule_log.log", std::ios::out | std::ios::app);
        if (!ruleLogFile.is_open())
        {
            throw rule_error("Failed to open rule log file '.oak_build/__rule_log.log'");
        }

        ruleLogFile << "\n\n/////////// In file " << curFile << " ///////////\n\n";
    }

    for (int i = 0; i < From.size(); i++)
    {
        // Add a new rule to the list of all rules
        if (From[i] == "new_rule!")
        {
            auto args = getMacroArgs(From, i);
            for (int j = 0; j < args.size(); j++)
            {
                rm_assert(args[j].size() >= 2, "Rule macro argument is of insufficient size.");
                rm_assert((args[j].front() == '"' && args[j].back() == '"') ||
                              (args[j].front() == '\'' && args[j].back() == '\''),
                          "All rule macro arguments must be strings.");
                args[j].pop_back();
                args[j].erase(0, 1);
            }

            rm_assert(args.size() == 3 || args.size() == 4, "new_rule! requires three or four arguments.");

            std::string name = args[0];
            Rule toAdd;

            if (args.size() == 3)
            {
                // Default: Use sapling
                toAdd.doRule = doRuleAcorn;
            }
            else if (args.size() == 4)
            {
                // Custom rule engine: Look up in engine table

                rm_assert(engines.count(args[3]) != 0, "Failed to load unknown rule engine '" + args[3] + "'");
                toAdd.doRule = engines[args[3]];
            }

            lexer dfa_lexer;
            toAdd.inputPattern = dfa_lexer.lex(args[1]);
            toAdd.outputPattern = dfa_lexer.lex(args[2]);

            rules[name] = toAdd;
        }

        // Use a rule that already exists
        else if (From[i] == "use_rule!")
        {
            // use_rule!("NAME", "NAME", ...);

            auto args = getMacroArgs(From, i);
            for (int j = 0; j < args.size(); j++)
            {
                rm_assert(args[j].size() >= 2, "Rule macro argument is of insufficient size.");
                rm_assert((args[j].front() == '"' && args[j].back() == '"') ||
                              (args[j].front() == '\'' && args[j].back() == '\''),
                          "All rule macro arguments must be strings.");
                args[j].pop_back();
                args[j].erase(0, 1);
            }

            rm_assert(args.size() > 0, "use_rule! requires at least one argument.");

            for (auto arg : args)
            {
                rm_assert(rules.count(arg) != 0 || bundles.count(arg) != 0,
                          "Rule '" + arg + "' does not exist and is not a bundle.");

                // bundles override rules with the same name
                if (bundles.count(arg) != 0)
                {
                    for (auto r : bundles[arg])
                    {
                        activeRules.push_back(r);
                    }
                }
                else
                {
                    activeRules.push_back(arg);
                }
            }
        }

        // Stop using a rule that is in use
        else if (From[i] == "rem_rule!")
        {
            // rem_rule!("NAME", "NAME", ...);

            auto args = getMacroArgs(From, i);
            for (int j = 0; j < args.size(); j++)
            {
                rm_assert(args[j].size() >= 2, "Rule macro argument is of insufficient size.");
                rm_assert((args[j].front() == '"' && args[j].back() == '"') ||
                              (args[j].front() == '\'' && args[j].back() == '\''),
                          "All rule macro arguments must be strings.");
                args[j].pop_back();
                args[j].erase(0, 1);
            }

            if (args.size() == 0)
            {
                activeRules.clear();
            }

            for (auto arg : args)
            {
                bool found = false;
                for (int j = activeRules.size() - 1; j >= 0; j--)
                {
                    if (activeRules[j] == arg)
                    {
                        found = true;
                        activeRules.erase(activeRules.begin() + j);
                        break;
                    }
                }

                rm_assert(found, "Rule '" + arg + "' is not in use.");
            }
        }

        // Bundle multiple rules into one
        else if (From[i] == "bundle_rule!")
        {
            // bundle_rule!("BUNDLE_NAME", "NAME", ...);

            auto args = getMacroArgs(From, i);
            for (int j = 0; j < args.size(); j++)
            {
                rm_assert(args[j].size() >= 2, "Rule macro argument is of insufficient size.");
                rm_assert((args[j].front() == '"' && args[j].back() == '"') ||
                              (args[j].front() == '\'' && args[j].back() == '\''),
                          "All rule macro arguments must be strings.");
                args[j].pop_back();
                args[j].erase(0, 1);
            }

            rm_assert(args.size() > 1, "bundle_rule! requires at least two arguments.");

            std::string name = args[0];
            for (int j = 1; j < args.size(); j++)
            {
                bundles[name].push_back(args[j]);
            }
        }

        // Regular case; Non-rule-macro symbol. Check against active rules.
        else
        {
            for (int ruleIndex = 0; ruleIndex < dialectRules.size() + activeRules.size(); ruleIndex++)
            {
                Rule curRule;
                if (ruleIndex < dialectRules.size())
                {
                    if (rules.count(dialectRules[ruleIndex]) == 0)
                    {
                        continue;
                    }

                    curRule = rules[dialectRules[ruleIndex]];
                }
                else
                {
                    if (rules.count(activeRules[ruleIndex - dialectRules.size()]) == 0)
                    {
                        continue;
                    }

                    curRule = rules[activeRules[ruleIndex - dialectRules.size()]];
                }

                if (i + curRule.inputPattern.size() >= From.size())
                {
                    continue;
                }

                // do rule here
                curRule.doRule(From, i, curRule);
            }
        }
    }

    // cout << "Done.\n";

    if (doRuleLogFile && ruleLogFile.is_open())
    {
        ruleLogFile.close();
    }

    return;
}

void loadDialectFile(const std::string &File)
{
    static bool dialectLock = false;

    if (dialectLock)
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

            if (engines.count(engine) == 0)
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

            name = "dialect_rule_" + std::to_string(dialectRules.size());

            // Insert into dialectRules list
            dialectRules.push_back(name);

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
            lexer dfa_lexer;
            rm_assert(inputStr != "", "Input rule must be non-empty.");
            toAdd.inputPattern = dfa_lexer.lex(inputStr);

            if (outputStr != "")
            {
                toAdd.outputPattern = dfa_lexer.lex(outputStr);
            }

            // Fetch engine based on current
            toAdd.doRule = engines[engine];

            // Insert into rules list
            rules[name] = toAdd;
        }
        else if (cur == "clear")
        {
            // Clear all existing dialect rules

            // Erase all existing dialect rules from existence
            for (auto s : dialectRules)
            {
                rules.erase(s);
            }

            // Clear list of dialect rules
            dialectRules.clear();
        }
        else if (cur == "final")
        {
            // Set the current dialect as final
            // (no more can ever be loaded in this translation)
            dialectLock = true;
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

void doRuleAcorn(std::vector<Token> &From, int &i, Rule &curRule)
{
    int posInFrom = i;
    std::vector<std::string> memory;
    std::map<std::string, std::string> ruleVars;
    bool isMatch = true;
    for (int k = 0; posInFrom < From.size() && k < curRule.inputPattern.size(); k++)
    {
        std::string match = curRule.inputPattern[k];

        if (posInFrom >= From.size())
        {
            isMatch = false;
            break;
        }

        // Macro; Cannot overwrite with rules
        if (From[posInFrom].size() > 1 && From[posInFrom].back() == '!')
        {
            isMatch = false;
            break;
        }

        // Wildcard; Un-stored match
        if (match == "$$")
        {
            memory.push_back(From[posInFrom]);

            posInFrom++;
            continue;
        }

        // Globs; Un-stored multi-symbol matches
        else if (match == "$*")
        {
            rm_assert(k + 1 < curRule.inputPattern.size(), "Glob card '$*' must not be end of rule.");
            std::string nextSymb = curRule.inputPattern[k + 1];

            while (From[posInFrom] != nextSymb)
            {
                memory.push_back(From[posInFrom]);
                posInFrom++;

                if (posInFrom >= From.size())
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

            if (From[posInFrom] == nextSymb)
            {
                isMatch = false;
            }

            while (From[posInFrom] != nextSymb)
            {
                memory.push_back(From[posInFrom]);
                posInFrom++;

                if (posInFrom >= From.size())
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
            if (From[posInFrom] == match.substr(2))
            {
                k--;
            }
        }

        // Negative lookbehind
        // Not prefixed by some literal
        else if (match.size() > 4 && match.substr(0, 4) == "$/<$")
        {
            // PosInFrom does not advance upon success
            if (posInFrom > 0 && From[posInFrom - 1] == match.substr(4))
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
            if (posInFrom + 1 < From.size() && From[posInFrom + 1] == match.substr(4))
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
                if (match.substr(start, end - start) == From[posInFrom].text)
                {
                    posInFrom++;
                    break;
                }
            }

            memory.push_back(From[posInFrom]);
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
                if (match.substr(start, end - start) == From[posInFrom].text)
                {
                    isMatch = false;
                    break;
                }
            }

            memory.push_back(From[posInFrom]);
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

            int beginningPosition = posInFrom;

            do
            {
                if (posInFrom > From.size())
                {
                    // Out of range w/o closure: Failure case
                    isMatch = false;
                    break;
                }

                memory.push_back(From[posInFrom]);

                if (From[posInFrom] == opener)
                {
                    count++;
                }
                else if (From[posInFrom] == closer)
                {
                    count--;
                }

                posInFrom++;
            } while (posInFrom < From.size() && count != 0);

            if (!isMatch || posInFrom == beginningPosition || posInFrom == beginningPosition + 1)
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

            while (posInFrom < From.size() && From[posInFrom] != nextSymb)
            {
                if (ruleVars.count(name) == 0)
                {
                    ruleVars[name] += From[posInFrom];
                }
                else
                {
                    ruleVars[name] += " " + From[posInFrom].text;
                }

                memory.push_back(From[posInFrom]);

                posInFrom++;
                if (posInFrom >= From.size())
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
                ruleVars[match] = From[posInFrom];
            }
            else
            {
                ruleVars[match] += " " + From[posInFrom].text;
            }

            memory.push_back(From[posInFrom]);

            posInFrom++;
        }

        // Literal; Must match verbatim
        else if (match == From[posInFrom].text)
        {
            posInFrom++;
            memory.push_back(From[posInFrom]);

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
        std::vector<Token> newContents;

        Token templ = From[i];
        templ.text = "NULL";

        for (int sIndex = 0; sIndex < curRule.outputPattern.size(); sIndex++)
        {
            std::string s = curRule.outputPattern[sIndex];

            if (ruleVars.count(s) != 0)
            {
                std::string raw = ruleVars[s];

                lexer dfa_lexer;
                std::vector<Token> lexed = dfa_lexer.lex(raw);
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
                    lexer dfa_lexer;
                    std::string raw = ruleVars[toInsert];
                    std::vector<Token> lexed = dfa_lexer.lex(raw);

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
        if (doRuleLogFile)
        {
            ruleLogFile << "Sapling rule w/ Input pattern\t";

            for (const auto &what : curRule.inputPattern)
            {
                ruleLogFile << "'" << what.text << "' ";
            }

            ruleLogFile << "\nOutput pattern\t";

            for (const auto &what : curRule.outputPattern)
            {
                ruleLogFile << "'" << what.text << "' ";
            }

            ruleLogFile << "\nMatch '";
        }

        // Erase old contents
        for (int k = i; k < posInFrom; k++)
        {
            if (doRuleLogFile)
            {
                ruleLogFile << From[i].text << ' ';
            }
            From.erase(From.begin() + i);
        }

        if (doRuleLogFile)
        {
            ruleLogFile << "'\nIs now '";

            for (const auto &what : newContents)
            {
                ruleLogFile << what.text << ' ';
            }

            ruleLogFile << "'\n\n";
        }

        // Insert new contents
        for (int k = newContents.size() - 1; k >= 0; k--)
        {
            From.insert(From.begin() + i, newContents[k]);
        }
    }

    ruleVars.clear();
}

void addEngine(const std::string &name, void (*hook)(std::vector<Token> &, int &, Rule &))
{
    engines[name] = hook;
    return;
}

#undef rm_assert
