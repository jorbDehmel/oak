/*
Unit testing for Oak.

Jordan Dehmel, 2024
jdehmel@outlook.com
*/

#include "../oakc_fns.hpp"
#include "test.hpp"
#include <filesystem>
#include <unistd.h>

////////////////////////////////////////////////////////////////
// Test cases

/*
void generate(const std::list<std::string> &Files, const
std::string &Output) ASTNode getAllocSequence(Type &type, const
std::string &name, AcornSettings &settings, const std::string
&num) ASTNode getFreeSequence(const std::string &name,
AcornSettings &settings) std::list<std::pair<std::string,
std::string>> restoreSymbolTable(MultiSymbolTable &backup,
MultiSymbolTable &realTable)
*/
void testMisc()
{
    if (fs::exists("/usr/include/oak/std/unit.oak"))
    {
        generate({"/usr/include/oak/std/unit.oak"}, "unit.md");

        std::string contents = execute("cat unit.md");
        execute("rm unit.md");

        // We cannot determine exactly what the contents are,
        // but there should be some. The magic number 200 was
        // determined to be a good ballpark minimum for a non-
        // empty documentation file.
        fakeAssert(contents.size() >= 200);
    }
}

/*
bool itCmp(const std::list<Token> &inside, const
std::list<Token>::iterator &it, const int &offset, const Token
&compareTo) noexcept bool itIsInRange(const std::list<Token>
&inside, const std::list<Token>::iterator &it, const int
&offset) noexcept Token itGet(const std::list<Token> &inside,
const std::list<Token>::iterator &it, const int &offset)
noexcept
*/
void testIteratorOperations()
{
    std::list<Token> l = {Token("foo"), Token("bar"),
                          Token("foobar"), Token("fizz"),
                          Token("buzz")};

    fakeAssert(itCmp(l, l.begin(), 2, "foobar"));
    fakeAssert(itGet(l, l.begin(), 2) == "foobar");
    fakeAssert(!itCmp(l, l.begin(), 1, "foobar"));
    fakeAssert(itGet(l, l.begin(), 1) != "foobar");
    fakeAssert(itIsInRange(l, l.begin(), 4));
    fakeAssert(!itIsInRange(l, l.begin(), 5));
    fakeAssert(!itIsInRange(l, l.begin(), -1));
}

// std::string execute(const std::string &command)
void testExecute()
{
    fakeAssert(execute("echo \'Hello, world!\'") ==
               "Hello, world!\n");
}

/*
std::string callMacro(const std::string &Name, const
std::list<std::string> &Args, AcornSettings &settings) bool
hasMacro(const std::string &name, AcornSettings &settings)
noexcept void compileMacro(const std::string &Name,
AcornSettings &settings) void addMacro(const std::string &name,
const std::string &contents, AcornSettings &settings)
*/
void testMacros()
{
    AcornSettings s;
    std::string contents =
        "let main(c: i32, v: []str) -> i32 { "
        "include!(\"std/io.oak\"); print(ptrarr!(v, 1)); }";

    fakeAssert(!hasMacro("foo!", s));

    addMacro("foo!", contents, s);

    fakeAssert(hasMacro("foo!", s));

    compileMacro("foo!", s);

    std::string results = callMacro("foo!", {"hi"}, s);
    fakeAssert(results == "hi");
}

/*
std::string mangleStruct(const std::string &name, const
std::list<std::list<std::string>> &generics) std::string
mangleEnum(const std::string &name, const
std::list<std::list<std::string>> &generics) std::string
mangleType(const Type &type) std::string mangleSymb(const
std::string &name, Type &type) std::string mangleSymb(const
std::string &name, const std::string &typeStr) std::string
mangle(const std::list<std::string> &what)
*/
void testMangler()
{
}

/*
long long getFileLastModification(const std::string &filepath,
AcornSettings &settings) unsigned long long int getSize(const
std::string &FilePath) std::string humanReadable(const unsigned
long long int &Size) bool isSourceNewer(const std::string
&source, const std::string &dest, AcornSettings &settings)
*/
void testSystem()
{
    execute("touch foo.txt");
    fakeAssert(getSize("foo.txt") == 0);
    fakeAssert(humanReadable(getSize("foo.txt")) == "0K");

    sleep(2);
    execute("touch bar.txt");

    AcornSettings s;
    fakeAssert(isSourceNewer("bar.txt", "foo.txt", s));
    fakeAssert(!isSourceNewer("foo.txt", "bar.txt", s));

    execute("rm foo.txt bar.txt");
}

////////////////////////////////////////////////////////////////
// Main function

int main()
{
    testMisc();
    testIteratorOperations();
    testExecute();
    testMacros();
    testMangler();
    testSystem();

    return 0;
}
