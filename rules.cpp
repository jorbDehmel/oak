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

    if (lexed[i].size() > 2 && lexed[i].substr(0, 2) == "//")
    {
        lexed.erase(lexed.begin() + i);
    }

    if (lexed[i] != "(")
    {
        throw runtime_error("Internal error; Malformed call to getMacroArgs.");
    }

    lexed.erase(lexed.begin() + i);

    if (lexed[i].size() > 2 && lexed[i].substr(0, 2) == "//")
    {
        lexed.erase(lexed.begin() + i);
    }

    string cur = "";
    while (!lexed.empty() && lexed[i] != ";")
    {
        if (lexed[i] == "," || lexed[i] == ")")
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
                if (rules.count(activeRules[ruleIndex]) == 0)
                {
                    // Invalid rule name
                    continue;
                }

                int isMatch = true;

                rule curRule;
                if (ruleIndex < dialectRules.size())
                {
                    curRule = rules[dialectRules[ruleIndex]];
                }
                else
                {
                    curRule = rules[activeRules[ruleIndex - dialectRules.size()]];
                }

                if (i + curRule.inputPattern.size() >= From.size())
                {
                    continue;
                }

                int posInFrom = i;
                for (int k = 0; k < curRule.inputPattern.size(); k++)
                {
                    string &match = curRule.inputPattern[k];

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
                    for (auto s : curRule.outputPattern)
                    {
                        if (ruleVars[ruleIndex].count(s) != 0)
                        {
                            newContents.push_back(ruleVars[ruleIndex][s]);
                        }
                        else
                        {
                            newContents.push_back(s);
                        }
                    }

                    // Erase old contents
                    for (int k = 0; k < curRule.inputPattern.size(); k++)
                    {
                        From.erase(From.begin() + i);
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

void loadDialectFile(const string &File)
{
    // Temp
    throw runtime_error("UNIMPLEMENTED");

    /*
    // comment
    clear
    rule "name" "in" "out"
    */

    // Open dialect file
    ifstream inp(File);
    if (!inp.is_open())
    {
        throw rule_error("Failed to open dialect file '" + File + "'");
    }

    string line;
    while (getline(inp, line))
    {
        if (line.size() > 1 && line.substr(0, 2) == "//")
        {
            continue;
        }

        stringstream lineStream(line);
        string cur;

        lineStream >> cur;
        if (cur == "rule")
        {
            // Remember that all args must be strings, even here

            string name;
            string inputStr, outputStr;
            rule toAdd;

            // Get name

            // Insert into dialectRules list
            dialectRules.push_back(name);

            // Get input pattern

            // Get output pattern

            // Lex patterns
            toAdd.inputPattern = lex(inputStr);
            toAdd.outputPattern = lex(outputStr);

            // Insert into rules list
            rules[name] = toAdd;
        }
        else if (cur == "clear")
        {
            dialectRules.clear();
        }
    }

    // Close dialect file
    inp.close();
    return;
}
