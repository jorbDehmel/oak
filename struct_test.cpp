#include <iostream>
#include <vector>
#include <string>

#include "symbol-table.hpp"

using namespace std;

int main()
{
    atomics.insert("map");

    string what = "let test: struct\n{\n\ta: u16,\n\tb: u32,\n\tc, d, e: u16,\n\tfrank_zappa: *map<str, u16>,\n}\n";
    cout << what << '\n';

    addStruct(what);

    string what2 = "let test2<T>: struct {data: T, data2: test,}";
    cout << "What2: '" << what2 << "'\n";

    addStruct(what2);

    for (auto p : structData)
    {
        cout << p.first << " has members:\n";
        for (auto s : p.second.members)
        {
            cout << "\t'" << s.first << "', which is a '" << toStr(&structData[p.first].members[s.first]) << "'\n";
        }
    }

    cout << "Done.\n";

    return 0;
}
