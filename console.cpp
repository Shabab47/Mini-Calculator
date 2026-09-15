// console.cpp
// A LEARNING VERSION of the calculator that runs in the terminal - no window.
// It uses the exact same Calculator class as the GUI, so you can read every
// step of the brain in one tiny main() and trace it with a debugger.
//
// Build: g++ -std=c++17 -static -municode console.cpp -o console.exe
// Run:   console.exe
//
// The console only stores two things: a Calculator object and the key pressed.
// For every key we simply call the matching Calculator method and print the
// screen. The GUI does the exact same calls - nothing more.

#include <cstdio>     // fwprintf, stdout
#include <cwchar>     // getwchar, WEOF, wint_t

#include "Calculator.h"

// print the list of usable keys once, at the start
void printHelp() {
    std::fputws(
        L"\nKeys:\n"
        L"  0-9        a digit\n"
        L"  + - * /    an operator\n"
        L"  =          equals\n"
        L"  .          decimal point\n"
        L"  %          percent\n"
        L"  n          negate  (+/-)\n"
        L"  b          backspace\n"
        L"  e          clear entry\n"
        L"  c          clear all\n"
        L"  q          quit\n", stdout);
}

int wmain() {
    Calculator calc;            // one calculator - the whole program's brain
    printHelp();

    for (;;) {                  // loop forever until 'q' is pressed
        // 1) show the screen
        std::fwprintf(stdout, L"\nScreen: %ls\n", calc.getDisplay().c_str());
        std::fwprintf(stdout, L"Key? ");
        std::fflush(stdout);

        // 2) read one key
        int c = std::getwchar();          // get one character; WEOF = failed/exit
        if (c == WEOF) break;
        wchar_t key = (wchar_t)c;

        // 3) digits are the only range, handle them first
        if (key >= L'0' && key <= L'9') {
            calc.inputDigit((char)key);   // '0'..'9' -> char 0..9
            continue;
        }

        // 4) everything else is one key -> one method call
        switch (key) {
            case L'+': calc.setOperator(L'+'); break;
            case L'-': calc.setOperator(L'-'); break;
            case L'*': calc.setOperator(L'*'); break;
            case L'/': calc.setOperator(L'/'); break;
            case L'=': calc.equals();        break;
            case L'.': calc.inputDot();      break;
            case L'%': calc.percent();       break;
            case L'n': calc.negate();        break;
            case L'b': calc.backspace();     break;
            case L'e': calc.clearEntry();    break;
            case L'c': calc.clearAll();      break;
            case L'q':
            case L'Q':
                std::fputws(L"\nBye!\n", stdout);
                return 0;
            default:
                std::fwprintf(stdout, L"Unknown key '%lc' - see the list above.\n", (wchar_t)c);
        }
    }
    return 0;
}