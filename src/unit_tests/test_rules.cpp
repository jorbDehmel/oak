/*
Unit testing for Oak.

Jordan Dehmel, 2024
jdehmel@outlook.com
*/

#include "test.hpp"

////////////////////////////////////////////////////////////////
// Utility functions

// Assert that the given Sapling rule creates the given output
// when presented with the given input.
void ensureRule(const std::string &text, const std::string &expected, const std::string &inputPattern,
                const std::string &outputPattern)
{
    Lexer l;
    AcornSettings settings;

    auto lexedText = l.lex_list(text);
    auto lexedExpected = l.lex_list(expected);
    auto lexedInputPattern = l.lex_list(inputPattern);
    auto lexedOutputPattern = l.lex_list(outputPattern);

    Rule r;
    r.engineName = "sapling";
    r.inputPattern.assign(lexedInputPattern.begin(), lexedInputPattern.end());
    r.outputPattern.assign(lexedOutputPattern.begin(), lexedOutputPattern.end());

    auto it = lexedText.begin();

    // Apply rule
    doRuleAcorn(lexedText, it, r, settings);

    // Assert equality
    assertEqual(lexedText, lexedExpected);
}

////////////////////////////////////////////////////////////////
// Test cases

void testGlobs()
{
}

void testVariables()
{
}

void testAppendation()
{
}

void testWildcards()
{
}

void testSuits()
{
}

void testLookarounds()
{
}

void testMethods()
{
    ensureRule("a.b(1, 2, 3)", "b(a, 1, 2, 3)", "inputPattern", "outputPattern");
}

////////////////////////////////////////////////////////////////
// Main function

int main()
{
    // Run test cases
    testGlobs();
    testVariables();
    testAppendation();
    testWildcards();
    testSuits();
    testLookarounds();
    testMethods();

    return 0;
}
