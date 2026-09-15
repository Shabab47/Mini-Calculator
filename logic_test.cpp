// logic_test.cpp - a tiny test program for the Calculator class.
// Build (console, no window):  g++ -std=c++17 -static -municode logic_test.cpp -o logic_test.exe
// Run: logic_test.exe

#include <cstdio>

#include "Calculator.h"

Calculator c;       // one calculator, tested below
int bad = 0;        // how many tests failed

// check that the screen shows exactly "wanted"
void check(const wchar_t* label, const wchar_t* wanted) {
    std::wstring got = c.getDisplay();
    if (got == wanted) {
        std::wprintf(L"PASS  %-18ls -> '%ls'\n", label, got.c_str());
    } else {
        std::wprintf(L"FAIL  %-18ls expected '%ls' got '%ls'\n", label, wanted, got.c_str());
        ++bad;
    }
}

int wmain() {
    // the scenario you asked for: 12 + 2 = 14
    c.clearAll();
    c.inputDigit(L'1'); c.inputDigit(L'2');   check(L"type 12", L"12");
    c.setOperator(L'+');                       check(L"press +", L"12+");
    c.inputDigit(L'2');                        check(L"press 2", L"12+2");
    c.equals();                                check(L"press =", L"14");

    // minus also shows the whole expression
    c.clearAll();
    c.inputDigit(L'1'); c.inputDigit(L'2'); c.setOperator(L'-');
    c.inputDigit(L'2'); c.equals();            check(L"12 - 2 =", L"10");

    // "5 =" keeps 5; a lonely "=" does nothing
    c.clearAll();
    c.inputDigit(L'5'); c.equals();            check(L"5 =", L"5");
    c.equals();                                check(L"= again", L"5");
    c.clearAll();
    c.equals();                                check(L"lonely =", L"0");

    // syntax errors
    c.clearAll();
    c.setOperator(L'+');                       check(L"+ first", L"Syntax error");
    c.inputDigit(L'7');                        check(L"recover with 7", L"7");
    c.clearAll();
    c.inputDigit(L'5'); c.setOperator(L'+'); c.setOperator(L'*');
                                               check(L"5 + *", L"Syntax error");

    // math error: divide by zero, then recover
    c.clearAll();
    c.inputDigit(L'5'); c.setOperator(L'/'); c.inputDigit(L'0'); c.equals();
                                               check(L"5 / 0 =", L"Math error");
    c.inputDigit(L'2');                        check(L"recover with 2", L"2");

    // percent, negate, backspace
    c.clearAll();
    c.inputDigit(L'5'); c.inputDigit(L'0'); c.percent();  check(L"50 %", L"0.5");
    c.clearAll();
    c.inputDigit(L'5'); c.negate();            check(L"5 +/-", L"-5");
    c.negate();                                check(L"-5 +/-", L"5");
    c.clearAll();
    c.inputDigit(L'1'); c.inputDigit(L'2'); c.inputDigit(L'3');
    c.backspace(); c.backspace();              check(L"123 bs bs", L"1");

    // CE keeps the pending operation: 5 + 3 CE 4 = 9
    c.clearAll();
    c.inputDigit(L'5'); c.setOperator(L'+'); c.inputDigit(L'3');
    c.clearEntry(); c.inputDigit(L'4'); c.equals();   check(L"5 + 3 CE 4 =", L"9");

    // continue after a result: (5 + 3) * 2 = 16
    c.clearAll();
    c.inputDigit(L'5'); c.setOperator(L'+'); c.inputDigit(L'3'); c.equals();
    c.setOperator(L'*'); c.inputDigit(L'2'); c.equals();  check(L"(5+3) * 2 =", L"16");

    if (bad == 0) {
        std::wprintf(L"\nALL TESTS PASSED\n");
        return 0;
    }
    std::wprintf(L"\n%d TEST(S) FAILED\n", bad);
    return 1;
}