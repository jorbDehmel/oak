/*
Implementation for the generics.hpp file of Oak
Oak ~0.0.12

Jordan Dehmel, 2023
*/

#include "generics.hpp"

map<pair<string, int>, genericInfo> generics;

string instantiateGeneric(const string &what, const vector<string> &genericSubs)
{
    // Ensure it exists
    pair<string, int> key = make_pair(what, genericSubs.size());
    if (generics.count(key) == 0)
    {
        cout << tags::red_bold
             << "During attempt to instantiate template '" << what << "' w/ generics:\n";

        for (auto s : genericSubs)
        {
            cout << '\t' << s << '\n';
        }

        cout << tags::reset << '\n';

        throw generic_error("Error! No template exists with which to instantiate template '" + what + "' w/ " + to_string(genericSubs.size()) + " generics.");
    }

    // Get mangled version
    string mangle = mangleStruct(what, genericSubs);

    // Check for pre-existing instantiation
    if (!generics[key].hasBeenInstantiated)
    {
        generics[key].hasBeenInstantiated = true;

        // All templates for generics exist in the global space,
        // so the result of this doesn't matter.

        // Make a copy of the template
        genericInfo &info = generics[make_pair(what, genericSubs.size())];
        vector<string> copy = info.symbols;

        // First, we need to know what we are substituting.
        map<string, string> substitutions;
        for (int i = 0; i < genericSubs.size(); i++)
        {
            substitutions[info.genericNames[i]] = genericSubs[i];
        }

        // Iterate and mangle template
        for (int i = 0; i < copy.size(); i++)
        {
            // Identify region to do substitution in
            // Mangling is handled during createSequence
            // below, so we only need to worry about
            // substitution here.

            if (substitutions.count(copy[i]) != 0)
            {
                // Do replacement if one is available
                copy[i] = substitutions[copy[i]];
            }
        }

        // Call on substituted results
        createSequence(copy);

        // Create copy of inst block
        if (info.instBlock.size() != 0)
        {
            copy.clear();
            copy = info.instBlock;

            // Iterate and mangle inst block
            for (int i = 0; i < copy.size(); i++)
            {
                if (substitutions.count(copy[i]) != 0)
                {
                    copy[i] = substitutions[copy[i]];
                }
            }

            // Call on substituted inst block
            createSequence(copy);
        }

        // Now the inst block and definition have both been handled.
    }
    // Otherwise, no need for instantiation. Just return mangle.

    // Return mangled version
    return mangle;
}

void addGeneric(const vector<string> &what, const string &name, const vector<string> &genericsList, const vector<string> &instBlock)
{
    genericInfo toAdd;

    toAdd.genericNames = genericsList;
    toAdd.symbols = what;
    toAdd.instBlock = instBlock;

    pair<string, int> key = make_pair(name, (int)genericsList.size());

    if (generics.count(key) == 0)
    {
        generics[key] = toAdd;
    }
    else
    {
        // Adjust generics to existing ones; A call to this will now instantiate
        // both the old and the new.

        // Build substitution dictionary
        map<string, string> substitutions;
        for (int i = 0; i < toAdd.genericNames.size(); i++)
        {
            substitutions[toAdd.genericNames[i]] = generics[key].genericNames[i];
        }

        // Iterate and replace, appending along the way.
        for (int i = 0; i < toAdd.symbols.size(); i++)
        {
            // Substitute if possible
            if (substitutions.count(toAdd.symbols[i]) != 0)
            {
                toAdd.symbols[i] = substitutions[toAdd.symbols[i]];
            }

            generics[key].symbols.push_back(toAdd.symbols[i]);
        }

        if (toAdd.instBlock.size() > 0)
        {
            // Same, but for inst block
            for (int i = 0; i < toAdd.instBlock.size(); i++)
            {
                // Substitute if possible
                if (substitutions.count(toAdd.instBlock[i]) != 0)
                {
                    toAdd.instBlock[i] = substitutions[toAdd.instBlock[i]];
                }

                generics[key].instBlock.push_back(toAdd.instBlock[i]);
            }
        }

        // Important! Allows reinstantiation
        generics[key].hasBeenInstantiated = false;
    }

    return;
}
