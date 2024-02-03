/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2023 - present
GPLv3 held by author
*/

#include "symbol_table.hpp"
#include "sequence_resources.hpp"
#include "type_builder.hpp"
#include <string>

// External definitions so as to avoid a dependency loop.

extern std::string mangle(const std::vector<std::string> &what);
extern std::string mangleStruct(const std::string &name, const std::vector<std::vector<std::string>> &generics);
extern std::string instantiateGeneric(const std::string &what, const std::vector<std::vector<std::string>> &genericSubs,
                                      const std::vector<std::string> &typeVec, AcornSettings &settings);

/*
Erases any non-function symbols which were not present
in the original table. However, skips all functions.
If not contradicted by the above rules, bases off of
the current table (not backup).
*/
std::vector<std::pair<std::string, std::string>> restoreSymbolTable(MultiSymbolTable &backup,
                                                                    MultiSymbolTable &realTable)
{
    std::vector<std::pair<std::string, std::string>> out;

    MultiSymbolTable newTable;
    for (auto p : realTable)
    {
        for (auto s : p.second)
        {
            // Functions are always added- the logic for
            // this is handled elsewhere.
            if (s.type[0].info == function)
            {
                // Add to new table for sure
                newTable[p.first];
                newTable[p.first].push_back(s);
            }

            // Variables are more complicated
            else
            {
                // Check for presence in backup
                bool present = false;
                for (auto cand : backup[p.first])
                {
                    if (cand.type == s.type)
                    {
                        present = true;
                        break;
                    }
                }

                // If was present in backup, add for sure
                if (present)
                {
                    // Add to table for sure
                    newTable[p.first];
                    newTable[p.first].push_back(s);
                }

                // Otherwise, do not add (do destructor literal check)
                else
                {
                    // Variable falling out of scope
                    // Do not call Del if is atomic literal
                    if (!(s.type[0].info == atomic &&
                          (s.type[0].name == "u8" || s.type[0].name == "i8" || s.type[0].name == "u16" ||
                           s.type[0].name == "i16" || s.type[0].name == "u32" || s.type[0].name == "i32" ||
                           s.type[0].name == "u64" || s.type[0].name == "i64" || s.type[0].name == "u128" ||
                           s.type[0].name == "i128" || s.type[0].name == "f32" || s.type[0].name == "f64" ||
                           s.type[0].name == "f128" || s.type[0].name == "bool" || s.type[0].name == "str")) &&
                        s.type[0].info != function && s.type[0].info != pointer && s.type[0].info != arr &&
                        s.type[0].info != sarr && p.first != "")
                    {
                        // Del_FN_PTR_typename_MAPS_void
                        out.push_back(
                            std::make_pair(p.first, "Del_FN_PTR_" + s.type[0].name + "_MAPS_void(&" + p.first + ");"));
                    }
                }
            }
        }
    }

    realTable = newTable;

    return out;
}
