/*
Implementation for the generics.hpp file of Oak
Oak ~0.0.12

Jordan Dehmel, 2023
*/

#include "generics.hpp"
#include "symbol_table.hpp"
#include "type_builder.hpp"

// External definition of createSequence, defined in sequence.cpp
// This avoids circular dependencies
extern sequence createSequence(const std::vector<std::string> &From);

// A pair of <name, number_of_generics> maps to a vector of symbols within
std::map<std::string, std::vector<genericInfo>> generics;

// Returns true if template substitution would make the two typeVecs the same
bool checkTypeVec(const std::vector<std::string> &candidateTypeVec, const std::vector<std::string> &genericTypeVec,
                  const std::vector<std::string> &genericNames,
                  const std::vector<std::vector<std::string>> &substitutions)
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
        else
        {
            return false;
        }
    }

    // Build substitution table
    std::map<std::string, std::vector<std::string>> subMap;
    for (int i = 0; i < genericNames.size(); i++)
    {
        subMap[genericNames[i]] = std::vector<std::string>();
        for (auto item : substitutions[i])
        {
            subMap[genericNames[i]].push_back(item);
        }
    }

    // Do substitutions
    // vector<string> afterSub;
    int i = 0;

    for (std::string symb : genericTypeVec)
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

            std::vector<std::string> remaining;
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

/*
Takes a type and a vector of candidates. Returns true if the
type is a prefix of any of the candidates.
*/
bool typeIsPrefixOfAny(const Type &t, const std::vector<__multiTableSymbol> &candidates)
{
    std::vector<bool> isViable;
    for (int i = 0; i < candidates.size(); i++)
    {
        isViable.push_back(true);
    }

    // Iterate over items in passed type
    for (int i = 0; i < t.size(); i++)
    {
        // Iterate over candidates
        for (int candIndex = 0; candIndex < isViable.size(); candIndex++)
        {
            if (!isViable[candIndex])
            {
                continue;
            }

            // Do checking here
            if (candidates[candIndex].type[i].info != var_name && t[i].info != var_name &&
                !(candidates[candIndex].type[i].info == t[i].info && candidates[candIndex].type[i].name == t[i].name))
            {
                isViable[candIndex] = false;
            }
        }
    }

    // Return result
    // int i = 0;
    for (const auto candidate : isViable)
    {
        if (candidate)
        {
            // cout << toStr(&t) << " is a prefix of " << toStr(&candidates[i].type) << '\n';
            return true;
        }
        // i++;
    }
    return false;
}

// Returns true if two instances are the same
bool checkInstances(const std::vector<std::vector<std::string>> &a, const std::vector<std::vector<std::string>> &b)
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
// Returns true if it was successful
std::string __instantiateGeneric(const std::string &what, genericInfo &info,
                                 const std::vector<std::vector<std::string>> &genericSubs)
{
    // Build substitution table
    std::map<std::string, std::vector<std::string>> substitutions;
    for (int i = 0; i < genericSubs.size(); i++)
    {
        substitutions[info.genericNames[i]] = genericSubs[i];
    }

    std::vector<std::string> copy;

    // Needs block (pre, so no functions)
    if (info.preBlock.size() != 0)
    {
        // Create copy of pre block
        copy.reserve(info.preBlock.size());
        copy.assign(info.preBlock.begin(), info.preBlock.end());

        // Iterate and mangle pre block
        for (int i = 0; i < copy.size(); i++)
        {
            if (substitutions.count(copy[i]) != 0)
            {
                std::string temp = copy[i];
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
            // Call on substituted pre block
            createSequence(copy);
        }
        catch (std::exception &e)
        {
            // This is only a failure case for this template
            return e.what();
        }
    }

    // Make a copy of the template
    copy.clear();
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

    // Post block
    if (info.postBlock.size() != 0)
    {
        copy.clear();

        // Create copy of post block
        copy.reserve(info.postBlock.size());
        copy.assign(info.postBlock.begin(), info.postBlock.end());

        // Iterate and mangle post block
        for (int i = 0; i < copy.size(); i++)
        {
            if (substitutions.count(copy[i]) != 0)
            {
                std::string temp = copy[i];
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
            // Call on substituted post block
            createSequence(copy);
        }
        catch (std::exception &e)
        {
            // This is only a failure case for this template
            return e.what();
        }
    }

    return "";
}

std::string instantiateGeneric(const std::string &what, const std::vector<std::vector<std::string>> &genericSubs,
                               const std::vector<std::string> &typeVec)
{
    // Get mangled version (only meaningful for struct instantiations)
    std::string oldCurFile = curFile;
    int oldCurLine = curLine;

    std::string mangleStr = mangleStruct(what, genericSubs);
    std::vector<std::string> errors;

    bool didInstantiate = false;

    // Check for existing symbol that would satisfy this
    // If such a symbol exists, set didInstantiate to true and
    // skip candidate checking

    // Existing struct / enum check
    if (structData.count(mangleStr) != 0)
    {
        didInstantiate = true;
    }

    // Existing function check (ignores New and Del because they
    // are usually autogen)
    else if (what != "New" && what != "Del" && typeIsPrefixOfAny(toType(typeVec), table[what]))
    {
        didInstantiate = true;
    }

    if (!didInstantiate)
    {
        // Ensure it exists
        if (generics.count(what) == 0)
        {
            std::cout << tags::red_bold << "During attempt to instantiate template '" << what << "' w/ generics:\n";

            for (auto s : genericSubs)
            {
                std::cout << '\t';
                for (auto t : s)
                {
                    std::cout << t << ' ';
                }
                std::cout << '\n';
            }
            std::cout << tags::reset << '\n';

            throw generic_error("Error! No template exists with which to instantiate template '" + what + "'.");
        }

        for (auto &candidate : generics[what])
        {
            curFile = candidate.originFile;

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
                    std::string result = __instantiateGeneric(what, candidate, genericSubs);
                    errors.push_back(result);

                    if (result == "")
                    {
                        // Instantiated
                        didInstantiate = true;
                        break;
                    }
                    else
                    {
                        errors.back().append(" (" + curFile + ":" + std::to_string(curLine) + ")");
                    }

                    // Else, failed with this template. Not an overall failure.
                }
                else
                {
                    didInstantiate = true;
                    break;
                }
            }
            else
            {
                errors.push_back("Did not match generic substitution. (" + curFile + ":" + std::to_string(curLine) +
                                 ")");
            }
        }
    }

    curFile = oldCurFile;
    curLine = oldCurLine;

    if (!didInstantiate)
    {
        std::cout << tags::yellow_bold << "Desired:\n\t" << what << " w/ type ";

        for (auto item : typeVec)
        {
            std::cout << item << ' ';
        }

        std::cout << "\nWith generics substitutions:\n";

        for (auto item : genericSubs)
        {
            std::cout << "\t'";

            for (auto b : item)
            {
                std::cout << b << ' ';
            }

            std::cout << "' (mangle: " << mangle(item) << ")" << '\n';
        }

        std::cout << "Against:\n";

        int i = 0;
        for (auto item : generics[what])
        {
            std::cout << '\t' << what << " w/ type ";
            for (auto b : item.typeVec)
            {
                std::cout << b << ' ';
            }
            std::cout << '\n' << "Failed with error:\n\t" << (i < errors.size() ? errors[i] : "Unknown error") << '\n';

            i++;
        }

        std::cout << tags::reset;

        throw generic_error("Error! Candidates exist for template '" + what + "', but none are viable.");
    }

    // Return mangled version
    return mangleStr;
}

void addGeneric(const std::vector<std::string> &what, const std::string &name,
                const std::vector<std::string> &genericsList, const std::vector<std::string> &typeVec,
                const std::vector<std::string> &preBlock, const std::vector<std::string> &postBlock)
{
    genericInfo toAdd;
    toAdd.originFile = curFile;

    for (auto a : genericsList)
    {
        if (a == ",")
        {
            continue;
        }

        toAdd.genericNames.push_back(a);
    }

    toAdd.symbols.reserve(what.size());
    toAdd.symbols.assign(what.begin(), what.end());

    toAdd.preBlock.reserve(preBlock.size());
    toAdd.preBlock.assign(preBlock.begin(), preBlock.end());

    toAdd.postBlock.reserve(postBlock.size());
    toAdd.postBlock.assign(postBlock.begin(), postBlock.end());

    toAdd.typeVec.reserve(typeVec.size());
    toAdd.typeVec.assign(typeVec.begin(), typeVec.end());

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

void printGenericDumpInfo(std::ostream &file)
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

            file << '\n' << "\t\tContents:";
            for (int i = 0; i < cand.symbols.size(); i++)
            {
                if (i % 10 == 0)
                {
                    file << "\n\t\t\t";
                }

                file << cand.symbols[i] << ' ';
            }

            file << '\n' << "\t\tPre block:";
            for (int i = 0; i < cand.preBlock.size(); i++)
            {
                if (i % 10 == 0)
                {
                    file << "\n\t\t\t";
                }

                file << cand.preBlock[i] << ' ';
            }

            file << '\n' << "\t\tPost block:";
            for (int i = 0; i < cand.postBlock.size(); i++)
            {
                if (i % 10 == 0)
                {
                    file << "\n\t\t\t";
                }

                file << cand.postBlock[i] << ' ';
            }

            file << '\n' << "\t\tInstances:\n";
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
