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
    if (generics.count(make_pair(what, genericSubs.size())) == 0)
    {
        throw generic_error("Error! No template exists with which to instantiate generic '" + what + "'.");
    }

    // Get mangled version
    string mangle = mangleStruct(what, genericSubs);

    // Check for pre-existing instantiation
    if (table.count(mangle) == 0)
    {
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
    }
    // Otherwise, no need for instantiation. Just return mangle.

    // Return mangled version
    return mangle;
}

void addGeneric(const vector<string> &what, const string &name, const vector<string> &genericsList)
{
    genericInfo toAdd;

    toAdd.genericNames = genericsList;
    toAdd.symbols = what;

    generics[make_pair(name, (int)genericsList.size())] = toAdd;

    return;
}
