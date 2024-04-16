/*
Implementation for the generics.hpp file of Oak
Oak ~0.0.12

Jordan Dehmel, 2023
*/

#include "oakc_fns.hpp"
#include "oakc_structs.hpp"
#include "options.hpp"
#include "tags.hpp"
#include <unistd.h>

// Returns true if template substitution would make the two
// typeVecs the same
bool checkTypeVec(
    const std::list<std::string> &candidateTypeVec,
    const std::list<std::string> &genericTypeVec,
    const std::list<std::string> &genericNames,
    const std::list<std::list<std::string>> &substitutions)
{
    if (genericNames.size() != substitutions.size())
    {
        return false;
    }

    if (candidateTypeVec.size() == 1 &&
        (candidateTypeVec.front() == "struct" ||
         candidateTypeVec.front() == "enum"))
    {
        if (genericTypeVec.size() == 1 &&
            (genericTypeVec.front() == "struct" ||
             genericTypeVec.front() == "enum"))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    // Build substitution table
    std::map<std::string, std::list<std::string>> subMap;

    auto nameIter = genericNames.begin();
    auto subIter = substitutions.begin();

    while (nameIter != genericNames.end() &&
           subIter != substitutions.end())
    {
        subMap[*nameIter] = std::list<std::string>(*subIter);

        nameIter++;
        subIter++;
    }

    // Do substitutions
    auto candIter = candidateTypeVec.begin();
    for (std::string symb : genericTypeVec)
    {
        if (subMap.count(symb) == 0)
        {
            if (candIter == candidateTypeVec.end())
            {
                return false;
            }

            if (*candIter != symb)
            {
                // Failure case UNLESS this is a argument name

                bool result = false;

                candIter++;
                if (candIter == candidateTypeVec.end())
                {
                    result = true;
                }
                else if (*candIter != ":")
                {
                    result = true;
                }
                candIter--;

                if (result)
                {
                    return false;
                }
            }

            candIter++;
        }
        else
        {
            auto j = subMap[symb].begin();
            for (; j != subMap[symb].end() && *j == "^"; j++)
            {
                if (*candIter != "^")
                {
                    return false;
                }

                candIter++;
            }

            std::list<std::string> remaining = {
                j, subMap[symb].end()};

            if (j != subMap[symb].end() &&
                mangle(remaining) != *candIter)
            {
                for (auto newSymb : remaining)
                {
                    if (candIter == candidateTypeVec.end())
                    {
                        return false;
                    }

                    if (*candIter != newSymb)
                    {
                        return false;
                    }

                    candIter++;
                }
            }
            else
            {
                candIter++;
            }
        }
    }

    return true;
}

/*
Takes a type and a list of candidates. Returns true if the
type is a prefix of any of the candidates.
*/
bool typeIsPrefixOfAny(
    const Type &t,
    const std::list<MultiTableSymbol> &candidates)
{
    std::list<bool> isViable;
    for (int i = 0; i < candidates.size(); i++)
    {
        isViable.push_back(true);
    }

    // Iterate over items in passed type
    for (int i = 0; i < t.size(); i++)
    {
        // Iterate over candidates

        auto candIter = candidates.begin();
        auto viableIter = isViable.begin();

        for (; candIter != candidates.end() &&
               viableIter != isViable.end();
             candIter++, viableIter++)
        {
            if (!*viableIter)
            {
                continue;
            }

            // Do checking here
            if (candIter->type[i].info != var_name &&
                t[i].info != var_name &&
                !(candIter->type[i].info == t[i].info &&
                  candIter->type[i].name == t[i].name))
            {
                *viableIter = false;
            }
        }
    }

    // Return result
    for (const auto candidate : isViable)
    {
        if (candidate)
        {
            return true;
        }
    }
    return false;
}

// Returns true if two instances are the same
bool checkInstances(const std::list<std::list<std::string>> &a,
                    const std::list<std::list<std::string>> &b)
{
    if (a.size() != b.size())
    {
        return false;
    }

    auto l = a.begin();
    auto r = b.begin();
    for (int i = 0; i < a.size(); i++)
    {
        if (l->size() != r->size())
        {
            return false;
        }

        auto ll = l->begin();
        auto rr = r->begin();
        for (int j = 0; j < l->size(); j++)
        {
            if (*ll != *rr)
            {
                return false;
            }

            ll++;
            rr++;
        }

        l++;
        r++;
    }

    return true;
}

// Skips all error checking; DO NOT FEED THIS THINGS THAT MAY
// ALREADY HAVE INSTANCES Returns true if it was successful
std::string __instantiateGeneric(
    const std::string &what, GenericInfo &info,
    const std::list<std::list<std::string>> &genericSubs,
    AcornSettings &settings)
{
    // Build substitution table
    std::map<std::string, std::list<std::string>> substitutions;

    auto l = info.genericNames.begin();
    auto r = genericSubs.begin();

    while (l != info.genericNames.end() &&
           r != genericSubs.end())
    {
        substitutions[*l] = *r;

        l++;
        r++;
    }

    std::list<Token> copy;

    // Needs block (pre, so no functions)
    if (info.preBlock.size() != 0)
    {
        // Create copy of pre block
        for (auto item : info.preBlock)
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

        try
        {
            // Call on substituted pre block
            createSequence(copy, settings);
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
    createSequence(copy, settings);

    // Post block
    if (info.postBlock.size() != 0)
    {
        copy.clear();
        for (auto item : info.postBlock)
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

        try
        {
            // Call on substituted post block
            createSequence(copy, settings);
        }
        catch (std::exception &e)
        {
            throw generic_error(
                "Post-block failed to instantiate given "
                "prerequisites: " +
                std::string(e.what()));
        }
    }

    return "";
}

std::string instantiateGeneric(
    const std::string &what,
    const std::list<std::list<std::string>> &genericSubs,
    const std::list<std::string> &typeVec,
    AcornSettings &settings)
{
    // Get mangled version (only meaningful for struct
    // instantiations)
    std::string oldCurFile = settings.curFile;
    int oldCurLine = settings.curLine;

    std::string mangleStr = mangleStruct(what, genericSubs);
    std::list<std::string> errors;

    bool didInstantiate = false;

    // Check for existing symbol that would satisfy this
    // If such a symbol exists, set didInstantiate to true and
    // skip candidate checking

    // Existing struct / enum check
    if (settings.structData.count(mangleStr) != 0)
    {
        didInstantiate = true;
    }

    // Existing function check (ignores New and Del because they
    // are usually autogen)
    else if (what != "New" && what != "Del" &&
             typeIsPrefixOfAny(toType(typeVec, settings),
                               settings.table[what]))
    {
        didInstantiate = true;
    }

    if (!didInstantiate)
    {
        // Ensure it exists
        if (settings.generics.count(what) == 0)
        {
            std::cout
                << tags::red_bold
                << "During attempt to instantiate template '"
                << what << "' w/ generics:\n";

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

            throw generic_error(
                "Error: No template exists with which to "
                "instantiate template '" +
                what + "'.");
        }

        for (auto &candidate : settings.generics[what])
        {
            settings.curFile = candidate.originFile;

            if (checkTypeVec(typeVec, candidate.typeVec,
                             candidate.genericNames,
                             genericSubs))
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
                    std::string result = __instantiateGeneric(
                        what, candidate, genericSubs, settings);
                    errors.push_back(result);

                    if (result == "")
                    {
                        // Instantiated
                        didInstantiate = true;
                        break;
                    }
                    else
                    {
                        errors.back().append(
                            " (" + settings.curFile.string() +
                            ":" +
                            std::to_string(settings.curLine) +
                            ")");
                    }

                    // Else, failed with this template. Not an
                    // overall failure.
                }
                else
                {
                    didInstantiate = true;
                    break;
                }
            }
            else
            {
                errors.push_back(
                    "Did not match generic substitution. (" +
                    settings.curFile.string() + ":" +
                    std::to_string(settings.curLine) + ")");
            }
        }
    }

    settings.curFile = oldCurFile;
    settings.curLine = oldCurLine;

    if (!didInstantiate)
    {
        std::cout << "No valid instantiation candidate could "
                     "be found. Candidates:\n"
                  << tags::yellow_bold << "Desired:\n  " << what
                  << " w/ type ";

        for (auto item : typeVec)
        {
            std::cout << item << ' ';
        }

        std::cout << "\nWith substitutions:\n";

        for (auto item : genericSubs)
        {
            std::cout << "  '";
            for (auto b : item)
            {
                std::cout << b << ' ';
            }

            std::cout << "'\n";
        }

        std::cout << "Against:\n";

        auto it = errors.begin();
        for (auto item : settings.generics[what])
        {
            std::cout << tags::yellow_bold << "  " << what
                      << " w/ type ";
            for (auto b : item.typeVec)
            {
                std::cout << b << ' ';
            }
            std::cout << tags::reset << ": "
                      << (it != errors.end() ? *it
                                             : "Unknown error")
                      << '\n';

            it++;
        }

        std::cout << '\n' << tags::reset;

        throw generic_error(
            "Error: Candidates exist for template '" + what +
            "', but none are viable.");
    }

    // Return mangled version
    return mangleStr;
}

void addGeneric(const std::list<Token> &what,
                const std::string &name,
                const std::list<std::string> &genericsList,
                const std::list<std::string> &typeVec,
                const std::list<Token> &preBlock,
                const std::list<Token> &postBlock,
                AcornSettings &settings)
{
    GenericInfo toAdd;
    toAdd.originFile = settings.curFile;

    for (auto a : genericsList)
    {
        if (a == ",")
        {
            continue;
        }

        toAdd.genericNames.push_back(a);
    }

    toAdd.symbols.assign(what.begin(), what.end());
    toAdd.preBlock.assign(preBlock.begin(), preBlock.end());
    toAdd.postBlock.assign(postBlock.begin(), postBlock.end());
    toAdd.typeVec.assign(typeVec.begin(), typeVec.end());

    // ENSURE IT DOESN'T ALREADY EXIST HERE; If it does, return
    // w/o error
    bool doesExist = false;
    for (auto candidate : settings.generics[name])
    {
        if (candidate.genericNames.size() ==
            genericsList.size())
        {
            bool matchesCandidate = true;

            auto l = candidate.typeVec.begin();
            auto r = toAdd.typeVec.begin();

            while (l != candidate.typeVec.end())
            {
                if (*l != *r)
                {
                    matchesCandidate = false;
                    break;
                }

                l++;
                r++;
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
        settings.generics[name].push_front(toAdd);
    }
    else
    {
        std::cout << tags::yellow_bold
                  << "Warning: Not re-adding existing template "
                  << name << " w/ type `";
        for (const auto &item : typeVec)
        {
            std::cout << item << ' ';
        }
        std::cout << "`.\n";
    }

    return;
}

void printGenericDumpInfo(std::ostream &file,
                          AcornSettings &settings)
{
    int i = 0;
    for (auto p : settings.generics)
    {
        file << "Identifier: '" << p.first << "'\n";

        file << "Instantiation candidates:\n";
        for (auto cand : p.second)
        {
            file << '\t' << p.first << "<";
            i = 0;
            for (auto it : cand.genericNames)
            {
                file << it;

                if (i + 1 < cand.genericNames.size())
                {
                    file << ", ";
                }

                i++;
            }
            file << ">\n";

            file << "\t\tType list:";
            i = 0;
            for (auto it : cand.typeVec)
            {
                if (i % 10 == 0)
                {
                    file << "\n\t\t\t";
                }

                file << it << ' ';

                i++;
            }

            file << '\n' << "\t\tContents:";
            i = 0;
            for (auto it : cand.symbols)
            {
                if (i % 10 == 0)
                {
                    file << "\n\t\t\t";
                }

                file << it.text << ' ';

                i++;
            }

            file << '\n' << "\t\tPre block:";
            i = 0;
            for (auto it : cand.preBlock)
            {
                if (i % 10 == 0)
                {
                    file << "\n\t\t\t";
                }

                file << it.text << ' ';

                i++;
            }

            file << '\n' << "\t\tPost block:";
            i = 0;
            for (auto it : cand.postBlock)
            {
                if (i % 10 == 0)
                {
                    file << "\n\t\t\t";
                }

                file << it.text << ' ';

                i++;
            }

            file << '\n' << "\t\tInstances:\n";
            for (auto inst : cand.instances)
            {
                file << "\t\t\t" << p.first << "<";

                for (auto v : inst)
                {
                    for (auto vi : v)
                    {
                        file << vi << ' ';
                    }

                    file << ", ";
                }

                file << ">\n";
            }
        }
    }
}
