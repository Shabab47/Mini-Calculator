// Calculator.h
// The "brain" of the calculator. It only stores state and does math,
// it knows nothing about windows, buttons or graphics.
//
// This is a normal OOP class: private data (the state) + public methods
// (what the buttons do). Study it and the whole program makes sense.

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <cmath>      // for std::isfinite
#include <cstdio>     // for std::swprintf
#include <string>     // for std::wstring and std::stod

class Calculator {
public:
    // Constructor: every calculator starts cleared.
    Calculator() { clearAll(); }

    // ---------- button actions ----------
    void clearAll();        // C   : reset everything
    void clearEntry();      // CE  : clear only the number being typed
    void backspace();       // BS  : delete the last digit typed
    void inputDigit(char d);            // pressed keys 0..9
    void inputDot();                    // pressed the . key
    void setOperator(wchar_t newOp);    // pressed + - * /
    void equals();                      // pressed =
    void percent();                     // pressed %
    void negate();                      // pressed +/-

    // ---------- output ----------
    std::wstring getDisplay() const;    // what the screen should show

private:
    // ---------- data (the state) ----------
    double total;         // the running result: first number, or result so far
    wchar_t op;           // last operator: '+', '-', '*', '/' or '\0' if none
    std::wstring entry;   // the number currently being typed on screen
    bool waiting;         // true right after an operator: next digit = new number
    bool didType;         // true once the user has typed any number
    bool isError;         // true while an error message is shown
    std::wstring errMsg;  // "Math error" or "Syntax error"

    // ---------- private helpers ----------
    void showError(const wchar_t* msg);              // show an error, forget everything
    double parseEntry();                             // read "entry" as a number
    bool finishOperation();                          // do: total = total <op> entry
    bool doMath(double a, double b, wchar_t operation,
                double& result);                     // actually calculates
    std::wstring formatNumber(double value) const;   // turn a number into text
};

// ========================= implementation ==================================

void Calculator::clearAll() {
    total = 0;
    op = L'\0';
    entry = L"0";
    waiting = false;
    didType = false;
    isError = false;
    errMsg = L"";
}

void Calculator::showError(const wchar_t* msg) {
    clearAll();          // forget everything...
    isError = true;      // ...and remember to show the message
    errMsg = msg;
}

double Calculator::parseEntry() {
    return std::stod(entry);    // "12.5" -> the number 12.5
}

bool Calculator::doMath(double a, double b, wchar_t operation, double& result) {
    if (operation == L'+')           result = a + b;
    else if (operation == L'-')      result = a - b;
    else if (operation == L'*')      result = a * b;
    else if (operation == L'/') {
        if (b == 0) return false;    // cannot divide by zero
        result = a / b;
    } else {
        return false;                // unknown operator
    }
    // false if the result went to infinity or became "not a number"
    return std::isfinite(result);
}

bool Calculator::finishOperation() {
    double right = parseEntry();        // the number on screen
    double result;
    if (!doMath(total, right, op, result)) {
        showError(L"Math error");       // overflow, divide by zero, etc.
        return false;
    }
    total = result;
    op = L'\0';                         // this operation is now finished
    return true;
}

void Calculator::inputDigit(char d) {
    if (isError) clearAll();            // after an error, any digit starts fresh

    if (waiting) {                      // a new number was expected
        entry = L"";
        waiting = false;
    }
    if (entry == L"0") entry = L"";     // "007" should show as "7"

    entry += (wchar_t)d;                // add the typed digit
    didType = true;
}

void Calculator::inputDot() {
    if (isError) return;

    if (waiting) {                      // "." right after an operator
        entry = L"0.";
        waiting = false;
        didType = true;
        return;
    }
    if (entry == L"0") {                // ".5" behaves like "0.5"
        entry = L"0.";
        didType = true;
        return;
    }
    if (entry.find(L'.') != std::wstring::npos) return;   // only one dot allowed
    entry += L'.';
    didType = true;
}

void Calculator::setOperator(wchar_t newOp) {
    if (isError) return;

    // Case 1: two operators in a row, e.g. "5 + x" -> invalid
    if (op != L'\0' && waiting) {
        showError(L"Syntax error");
        return;
    }

    // Case 2: an operation is half-typed, e.g. "12 + 3 x" -> finish 12+3 first
    if (op != L'\0' && !waiting) {
        if (!finishOperation()) return;     // stops on "Math error"
        op = newOp;
        waiting = true;
        didType = true;
        return;
    }

    // Case 3: right after an "=" result, e.g. "14 x" -> keep going from 14
    if (waiting) {
        op = newOp;
        didType = true;
        return;
    }

    // Case 4: operator before any number, e.g. starting with "+" -> invalid
    if (!didType) {
        showError(L"Syntax error");
        return;
    }

    // Case 5: normal case, e.g. "12 +" -> remember 12 and wait for operand
    total = parseEntry();
    op = newOp;
    waiting = true;
    didType = true;
}

void Calculator::equals() {
    if (isError) return;

    if (op == L'\0') return;    // "5 =" keeps 5, a lonely "=" does nothing
    if (waiting) return;        // "5 + =" has no second number yet -> nothing

    if (!finishOperation()) return;
    entry = formatNumber(total);
    waiting = true;             // next keypress starts a brand new number
    didType = true;
}

void Calculator::percent() {
    if (isError) return;
    if (waiting) return;                        // nothing typed yet to take a percent of
    entry = formatNumber(parseEntry() / 100.0); // 50 % -> 0.5
    didType = true;
}

void Calculator::negate() {
    if (isError) return;
    if (waiting) return;        // nothing typed yet (refuse, keeps it simple)
    if (entry == L"0") return;

    if (entry[0] == L'-') entry = entry.substr(1);   // "-5" -> "5"
    else                  entry = L"-" + entry;      //  "5" -> "-5"
}

void Calculator::backspace() {
    if (isError) return;
    if (waiting) return;                        // don't edit a finished result
    if (entry.empty() || entry == L"0") return; // nothing to delete

    entry = entry.substr(0, entry.size() - 1);
    if (entry.empty() || entry == L"-") entry = L"0";
}

void Calculator::clearEntry() {
    if (isError) { clearAll(); return; }
    entry = L"0";
    waiting = false;
    // note: the pending operator is kept, so "5 + 3 CE 4 =" means "5 + 4"
}

std::wstring Calculator::formatNumber(double value) const {
    wchar_t buffer[64];
    std::swprintf(buffer, 64, L"%.15g", value);   // up to 15 significant digits
    return std::wstring(buffer);
}

std::wstring Calculator::getDisplay() const {
    if (isError) return errMsg;                             // "Math error" / "Syntax error"

    if (op != L'\0') {                                      // inside an expression
        std::wstring screen = formatNumber(total);
        screen += op;                                       // e.g. "12+"
        if (!waiting) screen += entry;                      // e.g. "12+2"
        return screen;
    }

    if (waiting) return formatNumber(total);                // result right after "="
    return entry;                                           // just a number on screen
}

#endif  // CALCULATOR_H