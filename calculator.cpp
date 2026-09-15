// Mini Calculator - a simple Win32 GUI calculator.
// No external dependencies. Builds into a single, portable .exe.
// The calculation logic lives in calc.hpp; this file is the GUI.

#include <windows.h>

#include <string>

#include "calc.hpp"

// ---------------------------------------------------------------------------
// Layout constants
// ---------------------------------------------------------------------------
namespace layout {
constexpr int MARGIN   = 14;
constexpr int BTN_W    = 60;
constexpr int BTN_H    = 46;
constexpr int GAP      = 5;
constexpr int DISP_H   = 44;
constexpr int CLIENT_W = MARGIN * 2 + BTN_W * 4 + GAP * 3;       // 283
constexpr int GRID_TOP = MARGIN + DISP_H + GAP + 4;               // 67
constexpr int CLIENT_H = GRID_TOP + BTN_H * 5 + GAP * 4 + MARGIN; // 331

int bx(int col) { return MARGIN + col * (BTN_W + GAP); }
int by(int row) { return GRID_TOP + row * (BTN_H + GAP); }
}  // namespace layout

// ---------------------------------------------------------------------------
// Control identifiers
// ---------------------------------------------------------------------------
enum : int {
    IDC_DISP = 1,
    IDC_0 = 100, IDC_1, IDC_2, IDC_3, IDC_4,
    IDC_5, IDC_6, IDC_7, IDC_8, IDC_9,
    IDC_DOT, IDC_EQ, IDC_PLUS, IDC_MINUS, IDC_MUL, IDC_DIV,
    IDC_PCT, IDC_NEG, IDC_C, IDC_CE, IDC_BS
};

// ---------------------------------------------------------------------------
// GUI <-> logic bridge
// ---------------------------------------------------------------------------
namespace calc {

State st;

void updateDisplay(HWND hwnd) {
    std::wstring text;
    if (st.error)   text = st.errMsg;
    else            text = buildExpression();
    SetWindowTextW(GetDlgItem(hwnd, IDC_DISP), text.c_str());
}

void handleCommand(HWND hwnd, int id) {
    if (id >= IDC_0 && id <= IDC_9) {
        onDigit(L'0' + (id - IDC_0));
    } else {
        switch (id) {
            case IDC_DOT:   onDot(); break;
            case IDC_EQ:    onEquals(); break;
            case IDC_PLUS:  onOperator(L'+'); break;
            case IDC_MINUS: onOperator(L'-'); break;
            case IDC_MUL:   onOperator(L'*'); break;
            case IDC_DIV:   onOperator(L'/'); break;
            case IDC_PCT:   onPercent(); break;
            case IDC_NEG:   onNegate(); break;
            case IDC_BS:    onBackspace(); break;
            case IDC_C:     resetAll(); break;
            case IDC_CE:    onClearEntry(); break;
            default: return;
        }
    }
    updateDisplay(hwnd);
}

}  // namespace calc

// ---------------------------------------------------------------------------
// Window / GUI code
// ---------------------------------------------------------------------------
namespace gui {

HINSTANCE g_hInst = nullptr;
HFONT g_dispFont = nullptr;
HFONT g_btnFont = nullptr;

struct KeyDef {
    int id;
    const wchar_t* text;
};

HWND addButton(HWND parent, int id, const wchar_t* text, int x, int y) {
    HWND b = CreateWindowExW(0, L"BUTTON", text,
                             WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                             x, y, layout::BTN_W, layout::BTN_H,
                             parent, reinterpret_cast<HMENU>(id), g_hInst, nullptr);
    if (g_btnFont) SendMessageW(b, WM_SETFONT, reinterpret_cast<WPARAM>(g_btnFont), TRUE);
    return b;
}

void createChildren(HWND hwnd) {
    NONCLIENTMETRICSW ncm{};
    ncm.cbSize = sizeof(ncm);
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

    LOGFONTW bf = ncm.lfMessageFont;
    bf.lfHeight = -MulDiv(13, 96, 72);
    g_btnFont = CreateFontIndirectW(&bf);

    LOGFONTW df = ncm.lfMessageFont;
    df.lfHeight = -MulDiv(22, 96, 72);
    g_dispFont = CreateFontIndirectW(&df);

    HWND disp = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"0",
                                WS_CHILD | WS_VISIBLE | WS_TABSTOP |
                                ES_RIGHT | ES_READONLY | ES_AUTOHSCROLL,
                                layout::MARGIN, layout::MARGIN,
                                layout::CLIENT_W - 2 * layout::MARGIN, layout::DISP_H,
                                hwnd, reinterpret_cast<HMENU>(IDC_DISP), g_hInst, nullptr);
    if (g_dispFont) SendMessageW(disp, WM_SETFONT, reinterpret_cast<WPARAM>(g_dispFont), TRUE);

    const KeyDef grid[5][4] = {
        { {IDC_C,   L"C"},      {IDC_CE, L"CE"},    {IDC_BS, L"\u232B"}, {IDC_DIV, L"\u00F7"} },
        { {IDC_7,   L"7"},      {IDC_8,  L"8"},     {IDC_9,  L"9"},      {IDC_MUL, L"\u00D7"} },
        { {IDC_4,   L"4"},      {IDC_5,  L"5"},     {IDC_6,  L"6"},      {IDC_MINUS, L"\u2212"} },
        { {IDC_1,   L"1"},      {IDC_2,  L"2"},     {IDC_3,  L"3"},      {IDC_PLUS, L"+"} },
        { {IDC_NEG, L"+/-"},    {IDC_0,  L"0"},     {IDC_DOT, L"."},     {IDC_EQ,   L"="} },
    };
    for (int r = 0; r < 5; ++r)
        for (int c = 0; c < 4; ++c)
            addButton(hwnd, grid[r][c].id, grid[r][c].text, layout::bx(c), layout::by(r));
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            createChildren(hwnd);
            return 0;
        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED)
                calc::handleCommand(hwnd, static_cast<int>(LOWORD(wParam)));
            return 0;
        case WM_DESTROY:
            if (g_dispFont) DeleteObject(g_dispFont);
            if (g_btnFont) DeleteObject(g_btnFont);
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

}  // namespace gui

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    gui::g_hInst = hInstance;
    SetProcessDPIAware();

    WNDCLASSW wc{};
    wc.lpfnWndProc = gui::WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = L"MiniCalcWindow";
    wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    RegisterClassW(&wc);

    RECT rc{ 0, 0, layout::CLIENT_W, layout::CLIENT_H };
    const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRectEx(&rc, style, FALSE, 0);
    const int ww = rc.right - rc.left;
    const int wh = rc.bottom - rc.top;

    const int sw = GetSystemMetrics(SM_CXSCREEN);
    const int sh = GetSystemMetrics(SM_CYSCREEN);

    HWND hwnd = CreateWindowExW(0, L"MiniCalcWindow", L"Calculator",
                                style | WS_VISIBLE,
                                (sw - ww) / 2, (sh - wh) / 2, ww, wh,
                                nullptr, nullptr, hInstance, nullptr);
    if (!hwnd) return 1;

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}