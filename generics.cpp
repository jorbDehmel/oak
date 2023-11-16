/*
Implementation for the generics.hpp file of Oak
Oak ~0.0.12

Jordan Dehmel, 2023
*/

#include "generics.hpp"

// External definition of createSequence, defined in sequence.cpp
// This avoids circular dependencies
extern sequence createSequence(const vector<string> &From);

// A pair of <name, number_of_generics> maps to a vector of symbols within
map<string, vector<genericInfo>> generics;

// Returns true if template substitution would make the two typeVecs the same
bool checkTypeVec(const vector<string> &candidateTypeVec,
                  const vector<string> &genericTypeVec,
                  const vector<string> &genericNames,
                  const vector<vector<string>> &substitutions)
{
    if (genericNames.size() != substitutions.size())
    {
        return false;
    }

    if (candidateTypeVec.size() == 1 && (candidateTypeVec[0] == "struct" || candidateTypeVec[0] == "enum"))
    {
        if (genericTypeVec.size() == 1 && (genericTypeVec[0] == "struct" || genericTypeVec[0] == "enum"))
        {
            return true;
        }
    }

    // Build substitution table
    map<string, vector<string>> subMap;
    for (int i = 0; i < genericNames.size(); i++)
    {
        subMap[genericNames[i]] = vector<string>();
        for (auto item : substitutions[i])
        {
            subMap[genericNames[i]].push_back(item);
        }
    }

    // Do substitutions
    // vector<string> afterSub;
    int i = 0;

    for (string symb : genericTypeVec)
    {
        if (subMap.count(symb) == 0)
        {
            // afterSub.push_back(symb);

            if (i >= candidateTypeVec.size())
            {
                return false;
            }

            if (candidateTypeVec[i] != symb)
            {
                // Failure case UNLESS this is a argument name
                if (!(i + 1 < candidateTypeVec.size() && candidateTypeVec[i + 1] == ":"))
                {
                    return false;
                }
            }

            i++;
        }
        else
        {
            int j = 0;
            while (j < subMap[symb].size() && subMap[symb][j] == "^")
            {
                if (candidateTypeVec[i] != "^")
                {
                    return false;
                }

                i++;
                j++;
            }

            vector<string> remaining;
            for (int k = j; k < subMap[symb].size(); k++)
            {
                remaining.push_back(subMap[symb][k]);
            }

            if (j != subMap[symb].size() && mangle(remaining) != candidateTypeVec[i])
            {
                for (auto newSymb : remaining)
                {
                    if (i >= candidateTypeVec.size())
                    {
                        return false;
                    }

                    if (candidateTypeVec[i] != newSymb)
                    {
                        return false;
                    }

                    i++;
                }
            }
            else
            {
                i++;
            }
        }
    }

    return true;
}

// Returns true if two instances are the same
bool checkInstances(const vector<vector<string>> &a, const vector<vector<string>> &b)
{
    if (a.size() != b.size())
    {
        return false;
    }

    for (int i = 0; i < a.size(); i++)
    {
        if (a[i].size() != b[i].size())
        {
            return false;
        }

        for (int j = 0; j < a[i].size(); j++)
        {
            if (a[i][j] != b[i][j])
            {
                return false;
            }
        }
    }

    return true;
}

// Skips all error checking; DO NOT FEED THIS THINGS THAT MAY ALREADY HAVE INSTANCES
void __instantiateGeneric(const string &what, genericInfo &info, const vector<vector<string>> &genericSubs)
{
    // Build substitution table
    map<string, vector<string>> substitutions;
    for (int i = 0; i < genericSubs.size(); i++)
    {
        substitutions[info.genericNames[i]] = genericSubs[i];
    }

    // Make a copy of the template
    vector<string> copy;
    for (auto item : info.symbols)
    {
        if (substitutions.count(item) != 0)
        {
            for (auto subItem : substitutions[item])
            {
                copy.push_back(subItem);
            }
        }
        else
        {
            copy.push_back(item);
        }
    }

    // Call on substituted results
    createSequence(copy);

    // Needs block
    if (info.needsBlock.size() != 0)
    {
        copy.clear();

        // Create copy of inst block
        for (auto item : info.needsBlock)
        {
            copy.push_back(item);
        }

        // Iterate and mangle inst block
        for (int i = 0; i < copy.size(); i++)
        {
            if (substitutions.count(copy[i]) != 0)
            {
                string temp = copy[i];
                copy.erase(copy.begin() + i);
                for (auto s : substitutions[temp])
                {
                    copy.insert(copy.begin() + i, s);
                    i++;
                }
            }
        }

        try
        {
            // Call on substituted inst block
            createSequence(copy);
        }
        catch (exception &e)
        {
            cout << tags::red_bold
                 << "\nDuring execution of needs block " << what << "<";

            int itemIndex = 0;
            for (auto item : genericSubs)
            {
                int subItemIndex = 0;
                for (auto subItem : item)
                {
                    cout << subItem;

                    if (subItemIndex + 1 < item.size())
                    {
                        cout << " ";
                    }

                    subItemIndex++;
                }

                if (itemIndex + 1 < genericSubs.size())
                {
                    cout << ", ";
                }

                itemIndex++;
            }

            cout << ">:\n\n"
                 << tags::reset;

            throw generic_error("During needs block execution: " + string(e.what()));
        }
    }

    return;
}

string instantiateGeneric(const string &what,
                          const vector<vector<string>> &genericSubs,
                          const vector<string> &typeVec)
{
    // Ensure it exists
    if (generics.count(what) == 0)
    {
        cout << tags::red_bold
             << "During attempt to instantiate template '" << what << "' w/ generics:\n";

        for (auto s : genericSubs)
        {
            cout << '\t';
            for (auto t : s)
            {
                cout << t << ' ';
            }
            cout << '\n';
        }
        cout << tags::reset << '\n';

        throw generic_error("Error! No template exists with which to instantiate template '" + what + "'.");
    }

    // Get mangled version (only meaningful for struct instantiations)
    string mangleStr = mangleStruct(what, genericSubs);

    bool didInstantiate = false;
    for (auto &candidate : generics[what])
    {
        if (checkTypeVec(typeVec, candidate.typeVec, candidate.genericNames, genericSubs))
        {
            bool hasInstance = false;

            // Search for existing instance here
            for (auto instance : candidate.instances)
            {
                if (checkInstances(instance, genericSubs))
                {
                    hasInstance = true;
                    break;
                }
            }

            if (!hasInstance)
            {
                candidate.instances.push_back(genericSubs);
                __instantiateGeneric(what, candidate, genericSubs);
            }

            didInstantiate = true;
            break;
        }
    }

    if (!didInstantiate)
    {
        cout << tags::yellow_bold
             << "Desired:\n\t" << what << " w/ type ";

        for (auto item : typeVec)
        {
            cout << item << ' ';
        }

        cout << "\nWith generics substitutions:\n";

        for (auto item : genericSubs)
        {
            cout << "\t'";

            for (auto b : item)
            {
                cout << b << ' ';
            }

            cout << "' (mangle: " << mangle(item) << ")"
                 << '\n';
        }

        cout << "Against:\n";

        for (auto item : generics[what])
        {
            cout << '\t' << what << " w/ type ";
            for (auto b : item.typeVec)
            {
                cout << b << ' ';
            }
            cout << '\n';
        }

        cout << tags::reset;

        throw generic_error("Error! Candidates exist for template '" + what + "', but none are viable.");
    }

    // Return mangled version
    return mangleStr;
}

void addGeneric(const vector<string> &what,
                const string &name,
                const vector<string> &genericsList,
                const vector<string> &needsBlock,
                const vector<string> &typeVec)
{
    genericInfo toAdd;

    for (auto a : genericsList)
    {
        if (a == ",")
        {
            continue;
        }

        toAdd.genericNames.push_back(a);
    }

    for (auto a : what)
    {
        toAdd.symbols.push_back(a);
    }

    for (auto a : needsBlock)
    {
        toAdd.needsBlock.push_back(a);
    }

    for (auto a : typeVec)
    {
        toAdd.typeVec.push_back(a);
    }

    // ENSURE IT DOESN'T ALREADY EXIST HERE; If it does, return w/o error
    bool doesExist = false;
    for (auto candidate : generics[name])
    {
        if (candidate.genericNames.size() == genericsList.size())
        {
            bool matchesCandidate = true;
            for (int j = 0; j < candidate.typeVec.size(); j++)
            {
                if (candidate.typeVec[j] != toAdd.typeVec[j])
                {
                    matchesCandidate = false;
                    break;
                }
            }

            if (matchesCandidate)
            {
                doesExist = true;
                break;
            }
        }
    }

    if (!doesExist)
    {
        generics[name].push_back(toAdd);
    }

    return;
}

void printGenericDumpInfo(ostream &file)
{
    for (auto p : generics)
    {
        file << "Identifier: '" << p.first << "'\n";

        file << "Instantiation candidates:\n";
        for (auto cand : p.second)
        {
            file << '\t' << p.first << "<";
            for (int i = 0; i < cand.genericNames.size(); i++)
            {
                file << cand.genericNames[i];

                if (i + 1 < cand.genericNames.size())
                {
                    file << ", ";
                }
            }
            file << ">\n";

            file << "\t\tType Vector:";
            for (int i = 0; i < cand.typeVec.size(); i++)
            {
                if (i % 10 == 0)
                {
                    file << "\n\t\t\t";
                }

                file << cand.typeVec[i] << ' ';
            }

            file << '\n'
                 << "\t\tContents:";
            for (int i = 0; i < cand.symbols.size(); i++)
            {
                if (i % 10 == 0)
                {
                    file << "\n\t\t\t";
                }

                file << cand.symbols[i] << ' ';
            }

            file << '\n'
                 << "\t\tNeeds block:";
            for (int i = 0; i < cand.needsBlock.size(); i++)
            {
                if (i % 10 == 0)
                {
                    file << "\n\t\t\t";
                }

                file << cand.needsBlock[i] << ' ';
            }

            file << '\n'
                 << "\t\tInstances:\n";
            for (auto inst : cand.instances)
            {
                file << "\t\t\t" << p.first << "<";

                for (int vIndex = 0; vIndex < inst.size(); vIndex++)
                {
                    for (int iIndex = 0; iIndex < inst[vIndex].size(); iIndex++)
                    {
                        file << inst[vIndex][iIndex];

                        if (iIndex + 1 < inst[vIndex].size())
                        {
                            file << ' ';
                        }
                    }

                    if (vIndex + 1 < inst.size())
                    {
                        file << ", ";
                    }
                }

                file << ">\n";
            }
        }
    }
}

/*
// Future of generics:

let example<t>: struct
{
    data: t,
    next: ^example<t>,
}
needs
{
    // Rather than New<t>;, we ensure a type to the instance
    // When entering the instantiator, only the first generic
    // is replaced. Those in the type are skipped.

    // Nested generic types as members should automatically be
    // instantiated.

    New<t>(self: ^example<t>);
    Copy<t>(self: ^example<t>);

    // Instantiation blocks should be filtered by argument type(s)
    // and should be strictly unique because of this.
}

let New<t>(self: ^example<t>) -> void {}
let Copy<t>(self: ^example<t>) -> void {}
*/
