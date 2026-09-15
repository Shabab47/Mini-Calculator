// calculator.cpp
// The window and buttons only. All the math happens in Calculator.h.
// Build for Windows with: g++ -std=c++17 -O2 -mwindows -municode -static calculator.cpp -o calculator.exe

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

const int GRID_TOP  = MARGIN + DISP_H + GAP + 4;              // where buttons start
const int CLIENT_W  = MARGIN * 2 + BTN_W * 4 + GAP * 3;       // 283 px wide
const int CLIENT_H  = GRID_TOP + BTN_H * 5 + GAP * 4 + MARGIN; // 331 px tall

// helper: position of a button at column/row numbers (0-based)
int buttonX(int col) { return MARGIN + col * (BTN_W + GAP); }
int buttonY(int row) { return GRID_TOP + row * (BTN_H + GAP); }

// ---------------------------------------------------------------------------
// Control ids: every button (and the screen) gets a number so the program
// can recognise which one was pressed.
// ---------------------------------------------------------------------------
enum {
    IDC_DISPLAY = 1,          // the screen (an edit control)

    IDC_0 = 100, IDC_1, IDC_2, IDC_3, IDC_4,
    IDC_5, IDC_6, IDC_7, IDC_8, IDC_9,   // digits 100..109

    IDC_DOT,                  // 110
    IDC_EQUALS,               // 111
    IDC_ADD,                  // 112
    IDC_SUB,                  // 113
    IDC_MUL,                  // 114
    IDC_DIV,                  // 115
    IDC_PERCENT,              // 116
    IDC_NEGATE,               // 117
    IDC_C,                    // 118
    IDC_CE,                   // 119
    IDC_BACKSPACE             // 120
};

// ---------------------------------------------------------------------------
// One Calculator object for the whole window
// ---------------------------------------------------------------------------
Calculator g_calc;

// ---------------------------------------------------------------------------
// Create one button at column/row.
// ---------------------------------------------------------------------------
HWND createButton(HWND parent, int id, const wchar_t* text, int col, int row) {
    return CreateWindowExW(
        0, L"BUTTON", text,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        buttonX(col), buttonY(row), BTN_W, BTN_H,   // position and size
        parent, (HMENU)(INT_PTR)id,                  // id = (HMENU)(INT_PTR)id
        GetModuleHandleW(NULL), NULL);
}

// ---------------------------------------------------------------------------
// Create the screen on top and the 20 buttons below it.
// ---------------------------------------------------------------------------
void createChildren(HWND parent) {
    // the screen: a read-only text box, numbers aligned to the right
    CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"0",
        WS_CHILD | WS_VISIBLE | ES_RIGHT | ES_READONLY | ES_AUTOHSCROLL,
        MARGIN, MARGIN, CLIENT_W - 2 * MARGIN, DISP_H,
        parent, (HMENU)(INT_PTR)IDC_DISPLAY, GetModuleHandleW(NULL), NULL);

    // the buttons, row by row.
    // note: \u00F7 = division sign, \u00D7 = times, \u2212 = minus, \u232B = backspace
    struct Key { int id; const wchar_t* text; };
    const Key keys[5][4] = {
        { {IDC_C,         L"C"},     {IDC_CE,         L"CE"},   {IDC_BACKSPACE, L"\u232B"}, {IDC_DIV, L"\u00F7"} },
        { {IDC_7,         L"7"},     {IDC_8,          L"8"},    {IDC_9,         L"9"},      {IDC_MUL, L"\u00D7"} },
        { {IDC_4,         L"4"},     {IDC_5,          L"5"},    {IDC_6,         L"6"},      {IDC_SUB, L"\u2212"} },
        { {IDC_1,         L"1"},     {IDC_2,          L"2"},    {IDC_3,         L"3"},      {IDC_ADD, L"+"} },
        { {IDC_NEGATE,    L"+/-"},   {IDC_0,          L"0"},    {IDC_DOT,       L"."},      {IDC_EQUALS, L"="} },
    };
    for (int row = 0; row < 5; ++row)
        for (int col = 0; col < 4; ++col)
            createButton(parent, keys[row][col].id, keys[row][col].text, col, row);
}

// ---------------------------------------------------------------------------
// A button was pressed: tell the Calculator, then refresh the screen.
// ---------------------------------------------------------------------------
void handleCommand(HWND window, int id) {
    if (id >= IDC_0 && id <= IDC_9) {
        g_calc.inputDigit((char)(L'0' + (id - IDC_0)));      // digits translate directly
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
// The window function: Windows calls this to send us messages (resize,
// clicks, close, ...). We only care about two of them.
// ---------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:                 // the window is being created
            createChildren(window);
            return 0;

        case WM_COMMAND:                // a child control was activated
            if (HIWORD(wParam) == BN_CLICKED)   // a button was clicked
                handleCommand(window, LOWORD(wParam));  // LOWORD = the button's id
            return 0;

        case WM_DESTROY:                // the window is being closed
            PostQuitMessage(0);         // tell the message loop to stop
            return 0;

        default:
            return DefWindowProcW(window, msg, wParam, lParam);  // default handling
    }
}

// ---------------------------------------------------------------------------
// Program start
// ---------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    SetProcessDPIAware();   // one line so the app is not blurry on high-DPI screens

    // describe the window: its function, background, cursor...
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);    // classic grey background
    wc.lpszClassName = L"MiniCalculator";
    RegisterClassW(&wc);

    // figure out the total window size (client area + title bar + borders)
    RECT rect = { 0, 0, CLIENT_W, CLIENT_H };
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRectEx(&rect, style, FALSE, 0);
    int winW = rect.right - rect.left;
    int winH = rect.bottom - rect.top;

    // start it in the middle of the screen
    int x = (GetSystemMetrics(SM_CXSCREEN) - winW) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - winH) / 2;

    HWND window = CreateWindowExW(
        0, L"MiniCalculator", L"Calculator",
        style | WS_VISIBLE, x, y, winW, winH,
        NULL, NULL, instance, NULL);
    if (!window) return 1;

    // the message loop: this runs forever, reacting to clicks, until the
    // window is closed. This is how a Windows program stays alive.
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);     // this calls WndProc for each message
    }
    return (int)msg.wParam;
}