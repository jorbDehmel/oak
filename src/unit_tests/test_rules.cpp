/*
Unit testing for Oak.

Jordan Dehmel, 2024
jdehmel@outlook.com
*/

#include "../oakc_fns.hpp"

////////////////////////////////////////////////////////////////
// Utility functions

static void assertEqual(const std::list<Token> &obs,
                        const std::list<Token> &exp)
{
    bool isEqual = true;

    if (obs.size() != exp.size())
    {
        isEqual = false;
    }
    else
    {
        auto a = obs.cbegin();
        auto b = exp.cbegin();

        while (a != obs.cend() && b != exp.cend())
        {
            if (a->text != b->text)
            {
                isEqual = false;
                break;
            }

            ++a;
            ++b;
        }
    }

    if (!isEqual)
    {
        std::cerr << "Observed:\n";
        for (const auto &item : obs)
        {
            std::cerr << item.text << ' ';
        }
        std::cerr << "\nIs not equal to expected:\n";
        for (const auto &item : exp)
        {
            std::cerr << item.text << ' ';
        }
        std::cerr << '\n';

        throw std::runtime_error(__FUNCTION__ +
                                 std::string(":") +
                                 std::to_string(__LINE__));
    }
}

// Assert that the given Sapling rule creates the given output
// when presented with the given input.
void ensureRule(const std::string &text,
                const std::string &expected,
                const std::string &inputPattern,
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
    r.inputPattern.assign(lexedInputPattern.begin(),
                          lexedInputPattern.end());
    r.outputPattern.assign(lexedOutputPattern.begin(),
                           lexedOutputPattern.end());

    auto it = lexedText.begin();

    // Apply rule
    while (it != lexedText.end())
    {
        doRuleAcorn(lexedText, it, r, settings);
        it++;
    }

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

void testPairMatching()
{
    ensureRule("foo {} bar", "foo hi bar", "$~ $<${$}$> $>v",
               "hi");
    ensureRule("foo {{}} bar", "foo hi bar", "$~ $<${$}$> $>v",
               "hi");
    ensureRule("foo {{{}{}}{}} bar", "foo hi bar",
               "$~ $<${$}$> $>v", "hi");
    ensureRule("foo {<>} bar", "foo hi bar", "$~ $<${$}$> $>v",
               "hi");
    ensureRule("foo {()} bar", "foo hi bar", "$~ $<${$}$> $>v",
               "hi");
    ensureRule("foo {[]} bar", "foo hi bar", "$~ $<${$}$> $>v",
               "hi");
    ensureRule("foo { a<foo>; } bar", "foo hi bar",
               "$~ $<${$}$> $>v", "hi");
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
    testPairMatching();

    return 0;
}
