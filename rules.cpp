#include "rules.hpp"
using namespace std;

map<string, rule> rules;
vector<string> activeRules, dialectRules;
map<string, vector<string>> bundles;

// I is the point in Lexed at which a macro name was found
// CONSUMPTIVE!
vector<string> getMacroArgs(vector<string> &lexed, const int &i)
{
    vector<string> out;

    lexed.erase(lexed.begin() + i);

    while (lexed[i].size() > 2 && lexed[i].substr(0, 2) == "//")
    {
        lexed.erase(lexed.begin() + i);
    }

    if (lexed[i] != "(")
    {
        throw runtime_error("Internal error; Malformed call to getMacroArgs.");
    }

    // Erase opening parenthesis
    lexed.erase(lexed.begin() + i);

    while (lexed[i].size() > 2 && lexed[i].substr(0, 2) == "//")
    {
        lexed.erase(lexed.begin() + i);
    }

    string cur = "";
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
        else if (lexed[i].size() < 3 || lexed[i].substr(0, 2) != "//")
        {
            cur += lexed[i];
        }

        lexed.erase(lexed.begin() + i);
    }

    if (!lexed.empty())
    {
        lexed.erase(lexed.begin() + i);
    }

    return out;
}

void doRules(vector<string> &From)
{
    vector<map<string, string>> ruleVars;

    // Initialize variable maps
    for (int i = 0; i < dialectRules.size() + activeRules.size(); i++)
    {
        ruleVars.push_back(map<string, string>());
    }

    for (int i = 0; i < From.size(); i++)
    {
        if (From[i].size() >= 2 && From[i].substr(0, 2) == "//")
        {
            continue;
        }

        // Add a new rule to the list of all rules
        else if (From[i] == "new_rule!")
        {
            // new_rule!("NAME", "INP_PAT", "OUT_PAT");

            auto args = getMacroArgs(From, i);
            for (int j = 0; j < args.size(); j++)
            {
                rm_assert(args[j].size() >= 2, "Rule macro argument is of insufficient size.");
                rm_assert((args[j].front() == '"' && args[j].back() == '"') || (args[j].front() == '\'' && args[j].back() == '\''), "All rule macro arguments must be strings.");
                args[j].pop_back();
                args[j].erase(0, 1);
            }

            rm_assert(args.size() == 3, "new_rule! requires exactly three arguments.");

            string name = args[0];
            rule toAdd;

            toAdd.inputPattern = lex(args[1]);
            toAdd.outputPattern = lex(args[2]);

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
                rm_assert((args[j].front() == '"' && args[j].back() == '"') || (args[j].front() == '\'' && args[j].back() == '\''), "All rule macro arguments must be strings.");
                args[j].pop_back();
                args[j].erase(0, 1);
            }

            rm_assert(args.size() > 0, "use_rule! requires at least one argument.");

            for (auto arg : args)
            {
                rm_assert(rules.count(arg) != 0 || bundles.count(arg) != 0, "Rule '" + arg + "' does not exist and is not a bundle.");

                // bundles override rules with the same name
                if (bundles.count(arg) != 0)
                {
                    for (auto r : bundles[arg])
                    {
                        activeRules.push_back(r);
                        ruleVars.push_back(map<string, string>());
                    }
                }
                else
                {
                    activeRules.push_back(arg);
                    ruleVars.push_back(map<string, string>());
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
                rm_assert((args[j].front() == '"' && args[j].back() == '"') || (args[j].front() == '\'' && args[j].back() == '\''), "All rule macro arguments must be strings.");
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
                rm_assert((args[j].front() == '"' && args[j].back() == '"') || (args[j].front() == '\'' && args[j].back() == '\''), "All rule macro arguments must be strings.");
                args[j].pop_back();
                args[j].erase(0, 1);
            }

            rm_assert(args.size() > 1, "bundle_rule! requires at least two arguments.");

            string name = args[0];
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
                int isMatch = true;

                rule curRule;
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

                int posInFrom = i;
                for (int k = 0; posInFrom < From.size() && k < curRule.inputPattern.size(); k++)
                {
                    string match = curRule.inputPattern[k];

                    // Empty string in input match; IDK why this would happen
                    if (match == "")
                    {
                        continue;
                    }

                    while (From[posInFrom].size() >= 2 && From[posInFrom].substr(0, 2) == "//")
                    {
                        posInFrom++;

                        if (posInFrom >= From.size())
                        {
                            isMatch = false;
                            break;
                        }
                    }

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
                        posInFrom++;
                        continue;
                    }

                    // Globs; Un-stored multi-symbol matches
                    else if (match == "$*")
                    {
                        throw_assert(k + 1 < curRule.inputPattern.size());
                        string nextSymb = curRule.inputPattern[k + 1];

                        while (From[posInFrom] != nextSymb)
                        {
                            posInFrom++;

                            if (posInFrom >= From.size())
                            {
                                isMatch = false;
                                break;
                            }

                            while (From[posInFrom].size() > 1 && From[posInFrom].substr(0, 2) == "//")
                            {
                                posInFrom++;
                                if (posInFrom >= From.size())
                                {
                                    isMatch = false;
                                    break;
                                }
                            }
                        }

                        continue;
                    }
                    else if (match == "$+")
                    {
                        throw_assert(k + 1 < curRule.inputPattern.size());
                        string nextSymb = curRule.inputPattern[k + 1];

                        if (From[posInFrom] == nextSymb)
                        {
                            isMatch = false;
                        }

                        while (From[posInFrom] != nextSymb)
                        {
                            posInFrom++;

                            if (posInFrom >= From.size())
                            {
                                isMatch = false;
                                break;
                            }

                            while (From[posInFrom].size() > 1 && From[posInFrom].substr(0, 2) == "//")
                            {
                                posInFrom++;
                                if (posInFrom >= From.size())
                                {
                                    isMatch = false;
                                    break;
                                }
                            }
                        }

                        continue;
                    }

                    // Variable glob; Stored multi-symbol match
                    else if (match.size() == 3 && match.substr(0, 2) == "$*")
                    {
                        string name = match.substr(0, 1) + match.substr(2, 1);

                        throw_assert(k + 1 < curRule.inputPattern.size());
                        string nextSymb = curRule.inputPattern[k + 1];

                        while (From[posInFrom] != nextSymb)
                        {
                            ruleVars[ruleIndex][name] += " " + From[posInFrom];

                            posInFrom++;
                            if (posInFrom >= From.size())
                            {
                                isMatch = false;
                                break;
                            }

                            while (From[posInFrom].size() > 1 && From[posInFrom].substr(0, 2) == "//")
                            {
                                posInFrom++;
                                if (posInFrom >= From.size())
                                {
                                    isMatch = false;
                                    break;
                                }
                            }
                        }

                        continue;
                    }

                    // Variable; Stored match
                    else if (match.size() == 2 && match[0] == '$')
                    {
                        ruleVars[ruleIndex][match] = From[posInFrom];
                        posInFrom++;
                    }

                    // Literal; Must match verbatim
                    else if (match == From[posInFrom])
                    {
                        posInFrom++;
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
                    vector<string> newContents;
                    for (int sIndex = 0; sIndex < curRule.outputPattern.size(); sIndex++)
                    {
                        string s = curRule.outputPattern[sIndex];

                        if (ruleVars[ruleIndex].count(s) != 0)
                        {
                            string raw = ruleVars[ruleIndex][s];

                            vector<string> lexed = lex(raw);
                            for (auto s : lexed)
                            {
                                newContents.push_back(s);
                            }
                        }
                        else if (s == "$<")
                        {
                            // Merge operator
                            throw_assert(!newContents.empty());
                            throw_assert(sIndex + 1 < curRule.outputPattern.size());

                            string toInsert = curRule.outputPattern[sIndex + 1];

                            if (ruleVars[ruleIndex].count(toInsert) != 0)
                            {
                                string raw = ruleVars[ruleIndex][toInsert];
                                vector<string> lexed = lex(raw);

                                toInsert = "";
                                for (auto str : lexed)
                                {
                                    toInsert.append(str);
                                }
                            }

                            newContents.back().append(toInsert);
                        }
                        else
                        {
                            newContents.push_back(s);
                        }
                    }

                    // Erase old contents
                    for (int k = i; k < posInFrom; k++)
                    {
                        if (From[i].size() > 1 && From[i].substr(2) == "//")
                        {
                            i++;
                            k--;
                        }
                        else
                        {
                            From.erase(From.begin() + i);
                        }
                    }

                    // Insert new contents
                    for (int k = newContents.size() - 1; k >= 0; k--)
                    {
                        From.insert(From.begin() + i, newContents[k]);
                    }
                }

                ruleVars[ruleIndex].clear();
            }
        }
    }

    return;
}

bool dialectLock = false;
void loadDialectFile(const string &File)
{
    if (dialectLock)
    {
        return;
    }

    /*
    // comment
    clear
    "in" "out"
    final
    */

    // Open dialect file
    ifstream inp(File);
    if (!inp.is_open())
    {
        throw rule_error("Failed to open dialect file '" + File + "'");
    }

    string line;
    int lineNum = 0;
    while (getline(inp, line))
    {
        lineNum++;

        if (line.size() > 1 && line.substr(0, 2) == "//")
        {
            continue;
        }
        else if (line.size() == 0)
        {
            continue;
        }

        stringstream lineStream(line);
        string cur;

        lineStream >> cur;
        if (cur.size() != 0 && (cur.front() == '"' || cur.front() == '\''))
        {
            // Remember that all args must be strings, even here

            string name;
            string inputStr, outputStr;
            rule toAdd;

            name = "dialect_rule_" + to_string(dialectRules.size());

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
            toAdd.inputPattern = lex(inputStr);
            toAdd.outputPattern = lex(outputStr);

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
            throw rule_error(File + ":" + to_string(lineNum) + " Invalid line '" + line + "'");
        }
    }

    // Close dialect file
    inp.close();
    return;
}
