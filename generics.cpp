/*
Implementation for the generics.hpp file of Oak
Oak ~0.0.12

Jordan Dehmel, 2023
*/

#include "generics.hpp"

map<pair<string, int>, vector<genericInfo>> generics;

string instantiateGeneric(const string &what, const vector<vector<string>> &genericSubs)
{
    // Ensure it exists
    pair<string, int> key = make_pair(what, genericSubs.size());
    if (generics.count(key) == 0)
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

        throw generic_error("Error! No template exists with which to instantiate template '" + what + "' w/ " + to_string(genericSubs.size()) + " generics.");
    }

    // Get mangled version (only meaningful for struct instantiations)
    string mangle = mangleStruct(what, genericSubs);

    // Get correct indices
    vector<int> indices;

    // Iterate over instantiation candidates
    for (int i = 0; i < generics[key].size(); i++)
    {
        // Check whether the current instantiation candidate
        // has any conflicting instantiations. If so, do not
        // instantiate it. Otherwise, add its index to the
        // list of indices to instantiate.

        bool hasInstance = false;
        for (auto instance : generics[key][i].instances)
        {
            /*
            cout << DB_INFO << "Checking instance\n\t";
            for (auto a : instance)
            {
                cout << "\"";
                for (auto b : a)
                {
                    cout << "'" << b << "' ";
                }
                cout << "\", ";
            }
            cout << "\nAgainst requested\n\t";
            for (auto a : genericSubs)
            {
                cout << "\"";
                for (auto b : a)
                {
                    cout << "'" << b << "' ";
                }
                cout << "\", ";
            }
            cout << "\n";
            */

            // Check whether this instance is equivalent to
            // the requested one
            bool isGood = true;
            for (int arg = 0; arg < instance.size(); arg++)
            {
                // Ensure that the compared sizes are valid
                if (instance[arg].size() != genericSubs[arg].size())
                {
                    isGood = false;
                    break;
                }

                // Iterate over sub-argument symbols
                for (int subarg = 0; subarg < instance[arg].size(); subarg++)
                {
                    if (instance[arg][subarg] != genericSubs[arg][subarg])
                    {
                        isGood = false;
                        break;
                    }
                }

                if (!isGood)
                {
                    break;
                }
            }

            // cout << DB_INFO << "Result: " << (isGood ? "true" : "false") << '\n';

            if (isGood)
            {
                hasInstance = true;
                break;
            }
        }

        // If an instance was found, do NOT add it.
        if (!hasInstance)
        {
            indices.push_back(i);
        }
    }

    // cout << DB_INFO << "Instantiating " << indices.size() << " candidates\n";

    // If needed, instantiate candidates
    if (indices.size() != 0)
    {
        for (auto i : indices)
        {
            // Mark instantiation
            generics[key][i].instances.push_back(genericSubs);

            // All templates for generics exist in the global space,
            // so the result of this doesn't matter.

            // Make a copy of the template
            genericInfo &info = generics[key][i];
            vector<string> copy = info.symbols;

            // First, we need to know what we are substituting.
            map<string, vector<string>> substitutions;
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
                    string temp = copy[i];
                    copy.erase(copy.begin() + i);
                    for (auto s : substitutions[temp])
                    {
                        copy.insert(copy.begin() + i, s);
                        i++;
                    }
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

            // Now the inst block and definition have both been handled.
        }
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
    // toAdd.instances.clear();

    pair<string, int> key = make_pair(name, (int)genericsList.size());

    generics[key].push_back(toAdd);

    return;
}
