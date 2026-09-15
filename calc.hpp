// Mini Calculator - pure state-machine logic (no GUI).
#pragma once

#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>

namespace calc {

struct State {
    double acc = 0.0;            // running accumulator
    wchar_t op = 0;              // 0, '+', '-', '*', '/'
    std::wstring entry = L"0";   // current number being typed
    bool opPressed = false;      // next digit starts a fresh entry
    bool typed = false;          // user has actually entered something
    bool error = false;
    std::wstring errMsg;
};

extern State st;

inline bool isFinite(double v) {
    return !std::isnan(v) && !std::isinf(v);
}

inline std::wstring formatNumber(double v) {
    wchar_t buf[64];
    std::swprintf(buf, 64, L"%.15g", v);
    return std::wstring(buf);
}

inline void resetAll() {
    st = State();
}

inline void setError(const wchar_t* msg) {
    st = State();
    st.error = true;
    st.errMsg = msg;
}

inline double parseEntry() {
    return std::stod(st.entry);
}

inline bool applyOp(wchar_t op, double a, double b, double& out) {
    switch (op) {
        case L'+': out = a + b; break;
        case L'-': out = a - b; break;
        case L'*': out = a * b; break;
        case L'/':
            if (b == 0.0) return false;
            out = a / b;
            break;
        default: return false;
    }
    return isFinite(out);
}

inline void onDigit(wchar_t d) {
    if (st.error) resetAll();  // recover: any digit starts fresh
    if (st.opPressed) { st.entry.clear(); st.opPressed = false; }
    if (st.entry == L"0") st.entry.clear();
    st.entry += d;
    st.typed = true;
}

inline void onDot() {
    if (st.error) return;
    if (st.opPressed) {
        st.entry = L"0.";
        st.opPressed = false;
        st.typed = true;
        return;
    }
    if (st.entry == L"0") { st.entry = L"0."; st.typed = true; return; }
    if (st.entry.find(L'.') != std::wstring::npos) return;  // already has a dot
    st.entry += L'.';
    st.typed = true;
}

inline void onOperator(wchar_t o) {
    if (st.error) return;

    // An operator with no operand between (e.g. "5 + x") is invalid syntax.
    if (st.op != 0 && st.opPressed) {
        setError(L"Syntax error");
        return;
    }

    // Finish the pending operation, then start a new one (immediate execution).
    if (st.op != 0 && !st.opPressed) {
        double v = 0.0, r = 0.0;
        try { v = parseEntry(); } catch (...) { setError(L"Syntax error"); return; }
        if (!applyOp(st.op, st.acc, v, r)) { setError(L"Math error"); return; }
        st.acc = r;
        st.op = o;
        st.opPressed = true;
        st.typed = true;
        return;
    }

    // No pending operation.
    if (st.opPressed) {
        // Just computed a result (e.g. "5 + 2 ="), keep it as base.
        st.op = o;
        st.typed = true;
        return;
    }

    // Operator pressed before any number was entered -> invalid syntax.
    if (!st.typed) {
        setError(L"Syntax error");
        return;
    }

    double v = 0.0;
    try { v = parseEntry(); } catch (...) { setError(L"Syntax error"); return; }
    st.acc = v;
    st.op = o;
    st.opPressed = true;
}

inline void onEquals() {
    if (st.error) return;

    // "5 =" -> keeps showing 5. A lone "=" does nothing.
    if (st.op == 0) return;
    // No second operand typed (e.g. "5 + =") -> nothing happens.
    if (st.opPressed) return;

    double v = 0.0, r = 0.0;
    try { v = parseEntry(); } catch (...) { setError(L"Syntax error"); return; }
    if (!applyOp(st.op, st.acc, v, r)) { setError(L"Math error"); return; }

    st.acc = r;
    st.op = 0;
    st.entry = formatNumber(r);
    st.opPressed = true;  // next digit (or operator) starts fresh
    st.typed = true;
}

inline void onPercent() {
    if (st.error) return;
    if (st.opPressed) return;  // nothing typed to take a percent of
    try {
        st.entry = formatNumber(parseEntry() / 100.0);
        st.typed = true;
    } catch (...) {
        setError(L"Syntax error");
    }
}

inline void onNegate() {
    if (st.error) return;
    if (st.opPressed) return;  // nothing typed to negate
    if (st.entry == L"0") return;
    if (!st.entry.empty() && st.entry[0] == L'-') st.entry.erase(0, 1);
    else st.entry = L"-" + st.entry;
}

inline void onBackspace() {
    if (st.error) return;
    if (st.opPressed) return;  // don't rewrite a result/accumulator
    if (st.entry.empty() || st.entry == L"0") return;
    st.entry.pop_back();
    if (st.entry.empty() || st.entry == L"-") st.entry = L"0";
}

inline void onClearEntry() {
    if (st.error) { resetAll(); return; }
    st.entry = L"0";
    st.opPressed = false;
}

inline std::wstring buildExpression() {
    if (st.op == 0) {
        if (st.opPressed) return formatNumber(st.acc);  // result after '='
        return st.entry;                                // single number / fresh
    }
    std::wstring expr = formatNumber(st.acc) + st.op;   // e.g. "12-"
    if (!st.opPressed) expr += st.entry;                // operand typed: "12-2"
    return expr;
}

}  // namespace calc