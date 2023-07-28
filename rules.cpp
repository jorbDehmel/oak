#include "rules.hpp"
using namespace std;

map<string, rule> rules;
vector<string> activeRules;
map<string, vector<string>> bundles;

void doRules(vector<string> &From)
{
    vector<int> ruleStates;
    vector<map<string, string>> ruleVars;

    // Initialize state machines
    for (int i = 0; i < activeRules.size(); i++)
    {
        ruleStates.push_back(0);
        ruleVars.push_back(map<string, string>());
    }

    for (int i = 0; i < From.size(); i++)
    {
        if (From[i].size() >= 2 && From[i].substr(0, 2) == "//")
        {
            continue;
        }

        // Definitions and bundles
        else if (From[i] == "$let")
        {
            // $let NAME "input format" -> "output format" ;
            // $let BUNDLE_NAME = NAME NAME NAME ;

            rm_assert(i + 2 < From.size(), "Malformed rule; $let must be followed by bundle or definition.");

            string name = From[i + 1];

            // Erase from i until this index, but not this index.
            int eraseUntil;

            if (From[i + 2] == "=")
            {
                // Bundle
                int j = i + 3;

                while (j < From.size() && From[j] != ";")
                {
                    if (From[j] != ",")
                    {
                        bundles[name].push_back(From[j]);
                    }

                    j++;
                }

                eraseUntil = j + 1;
            }
            else
            {
                rm_assert(i + 6 < From.size(), "Malformed rule; Definition does not have enough space to be well-formed.");

                // Definitions
                string inpFormat, outFormat;
                inpFormat = From[i + 3];

                rm_assert(From[i + 4] == "->", "Malformed rule; Input format must be following by 'yields' arrow (->).");

                outFormat = From[i + 5];

                rm_assert(From[i + 6] == ";", "Malformed rule; Output format in definition must be followed by semicolon.");

                rm_assert(inpFormat.front() == '"' && inpFormat.back() == '"', "Input format must be string enclosed.");
                rm_assert(outFormat.front() == '"' && outFormat.back() == '"', "Output format must be string enclosed.");

                rule toAdd;
                toAdd.inputPattern = lex(inpFormat);
                toAdd.outputPattern = lex(outFormat);

                // Rules do not start out active
                rules[name] = toAdd;

                eraseUntil = i + 7;
            }

            for (int j = 0; j < eraseUntil - i; j++)
            {
                From.erase(From.begin() + i);
            }
            i--;
            continue;
        }
        else
        {
            // Iterate through rules
            for (int rule = 0; rule < activeRules.size(); rule++)
            {
                string pat = rules[activeRules[rule]].inputPattern[ruleStates[rule]];

                // Wildcard; Always a match, but never accessible
                if (pat == "$$")
                {
                    ruleStates[rule]++;
                }

                // Named variable; Always a match, stored
                else if (pat.front() == '$')
                {
                    ruleVars[rule][pat] = From[i];

                    ruleStates[rule]++;
                }

                // Literal; Must match symbol verbatim
                else if (pat == From[i])
                {
                    ruleStates[rule]++;
                }

                // Failure case; Reset
                else
                {
                    ruleStates[rule] = 0;
                    ruleVars[rule].clear();
                }

                // Check for success
                if (ruleStates[rule] == rules[activeRules[rule]].inputPattern.size())
                {
                    // Success case; Replace
                    int startIndex = 1 + i - rules[activeRules[rule]].inputPattern.size();

                    // Get old
                    vector<string> subset;
                    for (int k = startIndex; k <= i; k++)
                    {
                        subset.push_back(From[k]);
                    }

                    // Get new
                    vector<string> toInsert = replace(subset, rules[activeRules[rule]]);

                    // Erase old
                    for (int k = 1 + i - rules[activeRules[rule]].inputPattern.size(); k <= i; k++)
                    {
                        From.erase(From.begin() + startIndex);
                    }

                    // Insert new
                    for (int k = toInsert.size() - 1; k >= 0; k--)
                    {
                        From.insert(From.begin() + startIndex, toInsert[k]);
                    }
                }
            }
        }
    }

    return;
}

vector<string> replace(const vector<string> Subset, const rule &Rule)
{
}
