// calculator.cpp
// The window and buttons only. All the math happens in Calculator.h.
//
// HOW A WINDOWS GUI WORKS IN 5 STEPS
// (read these comments in order; the code below follows the same order)
//
//   1. Register a "window class"    - tell Windows what our window looks like
//                                    and which function will react to events.
//   2. Create the window            - Windows draws the frame/title bar.
//   3. Enter the "message loop"     - a while-loop that runs forever, waiting
//                                    for events (clicks, close button, ...).
//   4. WndProc() handles events     - Windows calls WndProc for every event.
//                                    We only care about two of them:
//                                      WM_CREATE  -> window was just created
//                                      WM_COMMAND -> a button was clicked
//   5. WM_COMMAND          - the id of the clicked button arrives in LOWORD(wParam).
//                            We translate it to a Calculator method call,
//                            then update the screen with getDisplay().
//
// Build for Windows with:
//   g++ -std=c++17 -O2 -mwindows -municode -static calculator.cpp -o calculator.exe
//
// NOTE: this file is UTF-8. The button symbols below are typed directly
// (÷ × − ⌫) instead of escape codes so the source reads like the button.

#include <windows.h>

#include "Calculator.h"

// ---------------------------------------------------------------------------
// Window layout (fixed-size window, no resizing needed)
// ---------------------------------------------------------------------------
const int MARGIN   = 14;   // space around the edges
const int BTN_W    = 60;   // button width
const int BTN_H    = 46;   // button height
const int GAP      = 5;    // space between buttons
const int DISP_H   = 44;   // height of the screen on top

const int GRID_TOP = MARGIN + DISP_H + GAP + 4;               // where the buttons start
const int CLIENT_W = MARGIN * 2 + BTN_W * 4 + GAP * 3;        // 283 px wide
const int CLIENT_H = GRID_TOP + BTN_H * 5 + GAP * 4 + MARGIN; // 331 px tall

// position of a button on its column/row (0-based):
int buttonX(int col) { return MARGIN + col * (BTN_W + GAP); }
int buttonY(int row) { return GRID_TOP + row * (BTN_H + GAP); }

// ---------------------------------------------------------------------------
// Control ids
// Every button (and the screen) gets a number. When a button is clicked,
// Windows tells us this number in LOWORD(wParam), so we know which one it was.
// ---------------------------------------------------------------------------
enum {
    IDC_DISPLAY = 1,          // the screen (an edit control)

    IDC_0 = 100, IDC_1, IDC_2, IDC_3, IDC_4,
    IDC_5, IDC_6, IDC_7, IDC_8, IDC_9,   // digit keys: 100..109

    IDC_DOT,          // 110  .
    IDC_EQUALS,       // 111  =
    IDC_ADD,          // 112  +
    IDC_SUB,          // 113  -
    IDC_MUL,          // 114  x
    IDC_DIV,          // 115  /
    IDC_PERCENT,      // 116  %
    IDC_NEGATE,       // 117  +/-
    IDC_C,            // 118  C
    IDC_CE,           // 119  CE
    IDC_BACKSPACE     // 120  <-
};

// ---------------------------------------------------------------------------
// One Calculator object for the whole window.
// ---------------------------------------------------------------------------
Calculator g_calc;

// ---------------------------------------------------------------------------
// Create one button at column/row.
// ---------------------------------------------------------------------------
HWND createButton(HWND parent, int id, const wchar_t* text, int col, int row) {
    // CreateWindowExW is THE function that creates any control:
    //   "BUTTON"        -> it builds a Windows button
    //   WS_CHILD        -> it lives inside our window
    //   WS_VISIBLE      -> show it immediately
    //   WS_TABSTOP      -> the TAB key can jump to it
    //   BS_PUSHBUTTON   -> the standard "press me" button
    return CreateWindowExW(
        0, L"BUTTON", text,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        buttonX(col), buttonY(row), BTN_W, BTN_H,   // position and size
        parent, (HMENU)(INT_PTR)id,                 // its parent and its id
        GetModuleHandleW(NULL), NULL);              // instance + extra data (none)
}

// ---------------------------------------------------------------------------
// Create the screen on top and the 20 buttons below it.
// ---------------------------------------------------------------------------
void createChildren(HWND parent) {
    // the screen: a read-only text box, numbers right-aligned
    CreateWindowExW(
        WS_EX_CLIENTEDGE,       // draw a sunken border around it
        L"EDIT", L"0",          // an edit control, starts showing "0"
        WS_CHILD | WS_VISIBLE | ES_RIGHT | ES_READONLY | ES_AUTOHSCROLL,
        MARGIN, MARGIN, CLIENT_W - 2 * MARGIN, DISP_H,
        parent, (HMENU)(INT_PTR)IDC_DISPLAY, GetModuleHandleW(NULL), NULL);

    // The buttons, laid out like this (row 0 is the top row):
    //
    //   row 0:  C     CE    <-    /
    //   row 1:  7     8     9     x
    //   row 2:  4     5     6     -
    //   row 3:  1     2     3     +
    //   row 4:  +/-   0     .     =
    //
    // "keys[5][4]" is a 2D table (the DSA bit): one record per button.
    //   keys[row][col].id   = the number Windows reports when clicked
    //   keys[row][col].text = what is printed on the button
    struct Key { int id; const wchar_t* text; };
    const Key keys[5][4] = {
        { {IDC_C,         L"C"},   {IDC_CE,         L"CE"},  {IDC_BACKSPACE, L"<-"}, {IDC_DIV, L"÷"} },
        { {IDC_7,         L"7"},   {IDC_8,          L"8"},   {IDC_9,         L"9"},   {IDC_MUL, L"×"} },
        { {IDC_4,         L"4"},   {IDC_5,          L"5"},   {IDC_6,         L"6"},   {IDC_SUB, L"−"} },
        { {IDC_1,         L"1"},   {IDC_2,          L"2"},   {IDC_3,         L"3"},   {IDC_ADD, L"+"} },
        { {IDC_NEGATE,    L"+/-"}, {IDC_0,          L"0"},   {IDC_DOT,       L"."},   {IDC_EQUALS, L"="} },
    };
    // two nested for-loops visit every cell of the table
    for (int row = 0; row < 5; ++row)
        for (int col = 0; col < 4; ++col)
            createButton(parent, keys[row][col].id, keys[row][col].text, col, row);
}

// ---------------------------------------------------------------------------
// A button was clicked: tell the Calculator, then refresh the screen.
// ---------------------------------------------------------------------------
void handleCommand(HWND window, int id) {
    if (id >= IDC_0 && id <= IDC_9) {
        g_calc.inputDigit((char)(L'0' + (id - IDC_0)));   // ids 100..109 = digits 0..9
    } else {
        switch (id) {
            case IDC_DOT:       g_calc.inputDot(); break;
            case IDC_EQUALS:    g_calc.equals(); break;
            case IDC_ADD:       g_calc.setOperator(L'+'); break;
            case IDC_SUB:       g_calc.setOperator(L'-'); break;
            case IDC_MUL:       g_calc.setOperator(L'*'); break;
            case IDC_DIV:       g_calc.setOperator(L'/'); break;
            case IDC_PERCENT:   g_calc.percent(); break;
            case IDC_NEGATE:    g_calc.negate(); break;
            case IDC_BACKSPACE: g_calc.backspace(); break;
            case IDC_C:         g_calc.clearAll(); break;
            case IDC_CE:        g_calc.clearEntry(); break;
            default: return;
        }
    }
    // put the calculator's reply into the screen box
    SetWindowTextW(GetDlgItem(window, IDC_DISPLAY), g_calc.getDisplay().c_str());
}

// ---------------------------------------------------------------------------
// Step 4: WndProc - Windows calls this for every "event" (message).
// We only react to the two messages that matter to us.
// ---------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:                 // the window was just created
            createChildren(window);     // build the screen + all 20 buttons
            return 0;

        case WM_COMMAND:                // some child control was activated
            if (HIWORD(wParam) == BN_CLICKED)          // ...it was a click
                handleCommand(window, LOWORD(wParam)); // LOWORD = the button's id
            return 0;

        case WM_DESTROY:                // the window is being closed
            PostQuitMessage(0);         // make the message loop stop (step 3 ends)
            return 0;

        default:
            // anything else (resize, paint, ...) -> let Windows handle it
            return DefWindowProcW(window, msg, wParam, lParam);
    }
}

// ---------------------------------------------------------------------------
// Steps 1-3 and 5: the program start.
// ---------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    SetProcessDPIAware();   // one line so the app is not blurry on high-DPI screens

    // STEP 1: register the window class (a "recipe" for our window)
    WNDCLASSW wc = {};                      // {} = start with all fields set to 0
    wc.lpfnWndProc = WndProc;               // who reacts to events (step 4)
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);            // the arrow mouse cursor
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);        // the classic grey background
    wc.lpszClassName = L"MiniCalculator";                  // the class name
    RegisterClassW(&wc);

    // figure out the full window size (client area + title bar + borders)
    RECT rect = { 0, 0, CLIENT_W, CLIENT_H };
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRectEx(&rect, style, FALSE, 0);
    int winW = rect.right - rect.left;
    int winH = rect.bottom - rect.top;

    // start it in the middle of the screen
    int x = (GetSystemMetrics(SM_CXSCREEN) - winW) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - winH) / 2;

    // STEP 2: create the actual window
    HWND window = CreateWindowExW(
        0, L"MiniCalculator", L"Calculator",   // class, window title
        style | WS_VISIBLE,                    // fixed size, shown immediately
        x, y, winW, winH,                      // position and size
        NULL, NULL, instance, NULL);           // no parent, no menu, etc.
    if (!window) return 1;                     // creating failed

    // STEP 3: the message loop - keep waiting for events forever
    MSG msg;                                   // one event
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {// wait for an event...
        TranslateMessage(&msg);                // (keyboard helper, not needed here)
        DispatchMessageW(&msg);                // ...and send it to WndProc
    }
    return (int)msg.wParam;                    // the "quit" value the loop ended with
}