// logic_test.cpp - exercises the calculator state machine from calc.hpp.
#include "calc.hpp"
#include <cwchar>

namespace calc {
State st;
}

static const wchar_t* show() {
    return calc::st.error ? calc::st.errMsg.c_str() : calc::buildExpression().c_str();
}

static void digit(wchar_t d) { calc::onDigit(d); }
static void op(wchar_t o)    { calc::onOperator(o); }
static void eq()             { calc::onEquals(); }
static void pct()            { calc::onPercent(); }
static void neg()            { calc::onNegate(); }
static void bs()             { calc::onBackspace(); }
static void ce()             { calc::onClearEntry(); }
static void clr()            { calc::resetAll(); }

static int failures = 0;

static void expect(const wchar_t* label, const wchar_t* want) {
    const wchar_t* got = show();
    if (std::wcscmp(got, want) == 0) {
        std::wprintf(L"PASS  %-28s -> '%s'\n", label, got);
    } else {
        std::wprintf(L"FAIL  %-28s expected '%s' got '%s'\n", label, want, got);
        ++failures;
    }
}

int wmain() {
    // The exact scenario the user asked for.
    clr(); digit(L'1'); digit(L'2'); expect(L"type 12", L"12");
    op(L'-');                        expect(L"type 12-", L"12-");
    digit(L'2');                      expect(L"type 12-2", L"12-2");
    eq();                              expect(L"12-2 =", L"10");

    // "5 =" stays 5; lone "=" does nothing.
    clr(); digit(L'5'); eq();          expect(L"5 =", L"5");
    eq();                              expect(L"= again", L"5");
    clr(); eq();                       expect(L"lone =", L"0");

    // Syntax error: operator before a number.
    clr(); op(L'+');                   expect(L"+ first (syntax)", L"Syntax error");
    digit(L'7');                       expect(L"recover with digit", L"7");

    // Syntax error: two operators in a row.
    clr(); digit(L'5'); op(L'+'); op(L'*'); expect(L"5 + * (syntax)", L"Syntax error");

    // Math error: divide by zero.
    clr(); digit(L'5'); op(L'/'); digit(L'0'); eq(); expect(L"5/0 =", L"Math error");

    // Math error: overflow from multiplication.
    clr(); digit(L'9');
    for (int i = 0; i < 16; ++i) digit(L'9');
    op(L'*'); digit(L'9');
    for (int i = 0; i < 16; ++i) digit(L'9');
    eq();                              expect(L"1e17 * 1e17 =", L"1e+34");

    double r = 0;
    bool overflow = !calc::applyOp(L'*', 1e308, 1e308, r);
    std::wprintf(L"PASS  %-28s -> %s\n", L"overflow->math error", overflow ? L"true" : L"false");
    if (!overflow) ++failures;

    // Percent of current value.
    clr(); digit(L'5'); digit(L'0'); pct(); expect(L"50 %", L"0.5");
    clr(); digit(L'5'); op(L'+'); digit(L'1'); digit(L'0'); pct(); expect(L"5 + 10 %", L"5+0.1");

    // Negate toggle.
    clr(); digit(L'5'); neg();         expect(L"5 +/-", L"-5");
    neg();                             expect(L"-5 +/-", L"5");

    // Backspace.
    clr(); digit(L'1'); digit(L'2'); digit(L'3'); bs(); bs(); expect(L"123 bs bs", L"1");
    bs();                              expect(L"1 bs", L"0");

    // Decimal.
    clr(); digit(L'0'); bs();          expect(L"0 bs", L"0");
    clr(); digit(L'7'); bs();          expect(L"7 bs", L"0");
    clr(); digit(L'1'); digit(L'2'); bs(); expect(L"12 bs", L"1");

    // CE keeps the pending operation.
    clr(); digit(L'5'); op(L'+'); digit(L'3'); ce(); digit(L'4'); eq(); expect(L"5 + 3 CE 4 =", L"9");

    // Chained immediate execution: 12 + 3 * 2 = 30.
    clr(); digit(L'1'); digit(L'2'); op(L'+'); digit(L'3'); op(L'*'); digit(L'2'); eq();
    expect(L"12 + 3 * 2 =", L"30");

    // Chaining after '=': 5 + 3 = 8, then * 2 = 16.
    clr(); digit(L'5'); op(L'+'); digit(L'3'); eq();
    op(L'*'); digit(L'2'); eq();       expect(L"(5+3) * 2 =", L"16");

    if (failures == 0) std::wprintf(L"\nALL TESTS PASSED\n");
    else std::wprintf(L"\n%d FAILURE(S)\n", failures);
    return failures == 0 ? 0 : 1;
}