#define _UNICODE
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "msimg32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

static const COLORREF COL_BG = RGB(8, 6, 14);      
static const COLORREF COL_PANEL = RGB(16, 12, 24);    
static const COLORREF COL_PURPLE = RGB(168, 85, 247); 
static const COLORREF COL_PURPLE_DIM = RGB(90, 48, 145);  
static const COLORREF COL_PURPLE_SOFT = RGB(200, 160, 250); 
static const COLORREF COL_WHITE_TXT = RGB(235, 225, 245);
static const COLORREF COL_SNOW_A = RGB(210, 175, 255); 
static const COLORREF COL_SNOW_B = RGB(150, 110, 210); 
static const COLORREF COL_PRESSED_BG = RGB(30, 18, 45);
static const COLORREF COL_DISABLED_BG = RGB(14, 12, 18);
static const COLORREF COL_DISABLED_TX = RGB(90, 80, 100);
static const COLORREF COL_DISABLED_BD = RGB(60, 50, 70);
static const COLORREF COL_ERROR = RGB(230, 90, 90);

static const wchar_t* APP_TITLE = L"GIGACHAD Loader - t.me/wizz3ard";
static const int WIN_W = 520;
static const int WIN_H = 420;

#define IDC_EDIT_USER   1001
#define IDC_EDIT_PASS   1002
#define IDC_BTN_LOGIN   1003
#define IDC_BTN_MAIN0   1010
#define IDC_BTN_MAIN1   1011
#define IDC_BTN_MAIN2   1012
#define IDC_BTN_MAIN3   1013
#define IDC_BTN_MAIN4   1014

#define TIMER_ANIM         1   
#define TIMER_LOGIN_LOAD   2   
#define TIMER_BTN_LOAD      3   

#define ANIM_INTERVAL_MS 16    

#define FLAKE_BMP_SIZE 14

enum class AppState { Login, Loading, Main, ButtonLoading };

struct SnowFlake {
    float x, y;
    float speed;
    float drift;
    float phase;
    float scale;   
    bool  bright;  
};

struct AppData {
    HWND hWnd = nullptr;
    HWND hEditUser = nullptr;
    HWND hEditPass = nullptr;
    HWND hBtnLogin = nullptr;
    HWND hBtnMain[5] = { nullptr };

    HFONT fontTitle = nullptr;
    HFONT fontLabel = nullptr;
    HFONT fontButton = nullptr;
    HFONT fontSmall = nullptr;

    AppState state = AppState::Login;
    std::wstring loginError;

    int spinnerAngle = 0;
    int pendingButton = -1;

    std::vector<SnowFlake> snow;

    HBITMAP memBitmap = nullptr;
    HDC     memDC = nullptr;
    int     memW = 0, memH = 0;

    HBRUSH brushBg = nullptr;
    HBRUSH brushPanel = nullptr;
    HBRUSH brushPressed = nullptr;
    HBRUSH brushDisabled = nullptr;
    HBRUSH brushNull = nullptr;

    HPEN penTitleLine = nullptr;
    HPEN penBorder = nullptr;
    HPEN penSpinnerFg = nullptr;
    HPEN penSpinnerDim = nullptr;
    HPEN penBtnBorder = nullptr;
    HPEN penBtnBorderDim = nullptr;
    HPEN penBtnBorderDis = nullptr;
    HPEN penNull = nullptr;

    HDC     flakeDCBright = nullptr;
    HBITMAP flakeBmpBright = nullptr;
    HDC     flakeDCDim = nullptr;
    HBITMAP flakeBmpDim = nullptr;
};

static AppData g;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void CreateLoginControls(HWND hWnd);
void CreateMainButtons(HWND hWnd);
void LayoutControls(HWND hWnd);
void ShowLoginScreen();
void StartLoginLoading();
void ShowMainScreen();
void StartButtonLoading(int index);
void FinishButtonLoading();
void InitSnow(int w, int h);
void UpdateSnow(int w, int h);
void PaintScene(HDC hdc, RECT rc);
void DrawSnow(HDC hdc, RECT rc);
void DrawSpinner(HDC hdc, int cx, int cy, int radius, int angle);
void EnsureBackBuffer(HDC hdc, int w, int h);
void DrawThemedButtonItem(LPDRAWITEMSTRUCT dis, const wchar_t* text, bool enabled);
void CreateCachedGdiObjects();
void FreeCachedGdiObjects();
HBITMAP MakeFlakeBitmap(HDC refDC, HDC& outDC, COLORREF tint);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow)
{
    srand((unsigned)time(nullptr));

    INITCOMMONCONTROLSEX icc{ sizeof(icc), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icc);

    const wchar_t* CLASS_NAME = L"GigaChadWindowClass";

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    RegisterClassExW(&wc);

    RECT wr{ 0, 0, WIN_W, WIN_H };
    DWORD style = (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
        | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    AdjustWindowRect(&wr, style, FALSE);

    HWND hWnd = CreateWindowExW(
        0, CLASS_NAME, APP_TITLE, style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right - wr.left, wr.bottom - wr.top,
        nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) return 0;
    g.hWnd = hWnd;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        g.fontTitle = CreateFontW(34, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
        g.fontLabel = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
        g.fontButton = CreateFontW(18, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
        g.fontSmall = CreateFontW(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");

        CreateCachedGdiObjects();

        CreateLoginControls(hWnd);
        CreateMainButtons(hWnd);
        LayoutControls(hWnd);
        ShowLoginScreen();

        RECT rc; GetClientRect(hWnd, &rc);
        InitSnow(rc.right, rc.bottom);

        SetTimer(hWnd, TIMER_ANIM, ANIM_INTERVAL_MS, nullptr);
        break;
    }

    case WM_CTLCOLOREDIT:
    {
        HDC hdcEdit = (HDC)wParam;
        SetBkColor(hdcEdit, COL_PANEL);
        SetTextColor(hdcEdit, COL_PURPLE_SOFT);
        return (LRESULT)g.brushPanel;
    }

    case WM_CTLCOLORSTATIC:
    {
        HDC hdcS = (HDC)wParam;
        SetBkMode(hdcS, TRANSPARENT);
        SetTextColor(hdcS, COL_PURPLE_SOFT);
        return (LRESULT)g.brushNull;
    }

    case WM_DRAWITEM:
    {
        LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
        wchar_t text[128];
        GetWindowTextW(dis->hwndItem, text, 128);
        BOOL enabled = IsWindowEnabled(dis->hwndItem);
        DrawThemedButtonItem(dis, text, enabled != FALSE);
        return TRUE;
    }

    case WM_COMMAND:
    {
        int id = LOWORD(wParam);
        if (id == IDC_BTN_LOGIN && HIWORD(wParam) == BN_CLICKED)
        {
            wchar_t user[64]{}, pass[64]{};
            GetWindowTextW(g.hEditUser, user, 64);
            GetWindowTextW(g.hEditPass, pass, 64);

            if (wcscmp(user, L"1234") == 0 && wcscmp(pass, L"1234") == 0)
            {
                g.loginError.clear();
                StartLoginLoading();
            }
            else
            {
                g.loginError = L"Invalid username or password";
                InvalidateRect(hWnd, nullptr, FALSE);
            }
        }
        else if (id >= IDC_BTN_MAIN0 && id <= IDC_BTN_MAIN4 && HIWORD(wParam) == BN_CLICKED)
        {
            StartButtonLoading(id - IDC_BTN_MAIN0);
        }
        break;
    }

    case WM_TIMER:
    {
        if (wParam == TIMER_ANIM)
        {
            RECT rc; GetClientRect(hWnd, &rc);
            UpdateSnow(rc.right, rc.bottom);
            g.spinnerAngle = (g.spinnerAngle + 6) % 360;

            if (g.state == AppState::Loading || g.state == AppState::ButtonLoading)
            {
                InvalidateRect(hWnd, nullptr, FALSE);
            }
            else
            {
                InvalidateRect(hWnd, nullptr, FALSE);
            }
        }
        else if (wParam == TIMER_LOGIN_LOAD)
        {
            KillTimer(hWnd, TIMER_LOGIN_LOAD);
            ShowMainScreen();
        }
        else if (wParam == TIMER_BTN_LOAD)
        {
            KillTimer(hWnd, TIMER_BTN_LOAD);
            FinishButtonLoading();
        }
        break;
    }

    case WM_ERASEBKGND:
        return 1; 

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        RECT rc; GetClientRect(hWnd, &rc);
        EnsureBackBuffer(hdc, rc.right, rc.bottom);
        PaintScene(g.memDC, rc);
        BitBlt(hdc, 0, 0, rc.right, rc.bottom, g.memDC, 0, 0, SRCCOPY);
        EndPaint(hWnd, &ps);
        break;
    }

    case WM_DESTROY:
        KillTimer(hWnd, TIMER_ANIM);
        KillTimer(hWnd, TIMER_LOGIN_LOAD);
        KillTimer(hWnd, TIMER_BTN_LOAD);
        if (g.memDC) DeleteDC(g.memDC);
        if (g.memBitmap) DeleteObject(g.memBitmap);
        DeleteObject(g.fontTitle);
        DeleteObject(g.fontLabel);
        DeleteObject(g.fontButton);
        DeleteObject(g.fontSmall);
        FreeCachedGdiObjects();
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hWnd, msg, wParam, lParam);
    }
    return 0;
}

void CreateCachedGdiObjects()
{
    g.brushBg = CreateSolidBrush(COL_BG);
    g.brushPanel = CreateSolidBrush(COL_PANEL);
    g.brushPressed = CreateSolidBrush(COL_PRESSED_BG);
    g.brushDisabled = CreateSolidBrush(COL_DISABLED_BG);
    g.brushNull = (HBRUSH)GetStockObject(NULL_BRUSH);

    g.penTitleLine = CreatePen(PS_SOLID, 2, COL_PURPLE_DIM);
    g.penBorder = CreatePen(PS_SOLID, 2, COL_PURPLE_DIM);
    g.penSpinnerFg = CreatePen(PS_SOLID, 4, COL_PURPLE);
    g.penSpinnerDim = CreatePen(PS_SOLID, 4, RGB(50, 40, 65));
    g.penBtnBorder = CreatePen(PS_SOLID, 2, COL_PURPLE);
    g.penBtnBorderDim = CreatePen(PS_SOLID, 2, COL_PURPLE_DIM);
    g.penBtnBorderDis = CreatePen(PS_SOLID, 2, COL_DISABLED_BD);
    g.penNull = (HPEN)GetStockObject(NULL_PEN);

    HDC screenDC = GetDC(nullptr);
    g.flakeBmpBright = MakeFlakeBitmap(screenDC, g.flakeDCBright, COL_SNOW_A);
    g.flakeBmpDim = MakeFlakeBitmap(screenDC, g.flakeDCDim, COL_SNOW_B);
    ReleaseDC(nullptr, screenDC);
}

void FreeCachedGdiObjects()
{
    DeleteObject(g.brushBg);
    DeleteObject(g.brushPanel);
    DeleteObject(g.brushPressed);
    DeleteObject(g.brushDisabled);

    DeleteObject(g.penTitleLine);
    DeleteObject(g.penBorder);
    DeleteObject(g.penSpinnerFg);
    DeleteObject(g.penSpinnerDim);
    DeleteObject(g.penBtnBorder);
    DeleteObject(g.penBtnBorderDim);
    DeleteObject(g.penBtnBorderDis);

    if (g.flakeDCBright) DeleteDC(g.flakeDCBright);
    if (g.flakeBmpBright) DeleteObject(g.flakeBmpBright);
    if (g.flakeDCDim) DeleteDC(g.flakeDCDim);
    if (g.flakeBmpDim) DeleteObject(g.flakeBmpDim);
}

HBITMAP MakeFlakeBitmap(HDC refDC, HDC& outDC, COLORREF tint)
{
    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = FLAKE_BMP_SIZE;
    bmi.bmiHeader.biHeight = -FLAKE_BMP_SIZE; 
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP hbm = CreateDIBSection(refDC, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!hbm || !bits) return nullptr;

    BYTE r = GetRValue(tint), gC = GetGValue(tint), b = GetBValue(tint);

    UINT32* px = (UINT32*)bits;
    float c = FLAKE_BMP_SIZE / 2.0f;
    for (int y = 0; y < FLAKE_BMP_SIZE; ++y)
    {
        for (int x = 0; x < FLAKE_BMP_SIZE; ++x)
        {
            float dx = (x + 0.5f) - c;
            float dy = (y + 0.5f) - c;
            float dist = sqrtf(dx * dx + dy * dy);
            float t = 1.0f - (dist / c);
            if (t < 0) t = 0;
            if (t > 1) t = 1;
            t = t * t * (3 - 2 * t); 
            BYTE a = (BYTE)(t * 255.0f);
            BYTE pr = (BYTE)((r * a) / 255);
            BYTE pg = (BYTE)((gC * a) / 255);
            BYTE pb = (BYTE)((b * a) / 255);
            px[y * FLAKE_BMP_SIZE + x] = (a << 24) | (pr << 16) | (pg << 8) | pb;
        }
    }

    outDC = CreateCompatibleDC(refDC);
    SelectObject(outDC, hbm);
    return hbm;
}

void CreateLoginControls(HWND hWnd)
{
    g.hEditUser = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        0, 0, 220, 26, hWnd, (HMENU)IDC_EDIT_USER, GetModuleHandle(nullptr), nullptr);

    g.hEditPass = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_PASSWORD,
        0, 0, 220, 26, hWnd, (HMENU)IDC_EDIT_PASS, GetModuleHandle(nullptr), nullptr);

    g.hBtnLogin = CreateWindowExW(0, L"BUTTON", L"LOG IN",
        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
        0, 0, 220, 34, hWnd, (HMENU)IDC_BTN_LOGIN, GetModuleHandle(nullptr), nullptr);

    SendMessageW(g.hEditUser, WM_SETFONT, (WPARAM)g.fontLabel, TRUE);
    SendMessageW(g.hEditPass, WM_SETFONT, (WPARAM)g.fontLabel, TRUE);
    SendMessageW(g.hBtnLogin, WM_SETFONT, (WPARAM)g.fontButton, TRUE);
}

void CreateMainButtons(HWND hWnd)
{
    const wchar_t* labels[5] = { L"Dayz", L"Fivem", L"CSGO", L"CS2", L"Rust" };
    for (int i = 0; i < 5; ++i)
    {
        g.hBtnMain[i] = CreateWindowExW(0, L"BUTTON", labels[i],
            WS_CHILD | BS_OWNERDRAW,
            0, 0, 150, 46, hWnd, (HMENU)(INT_PTR)(IDC_BTN_MAIN0 + i),
            GetModuleHandle(nullptr), nullptr);
        SendMessageW(g.hBtnMain[i], WM_SETFONT, (WPARAM)g.fontButton, TRUE);
    }
}

void LayoutControls(HWND hWnd)
{
    RECT rc; GetClientRect(hWnd, &rc);
    int cw = rc.right;

    int fw = 240;
    int cx = (cw - fw) / 2;

    int y = 130;
    y += 20;
    MoveWindow(g.hEditUser, cx, y, fw, 30, TRUE);   
    y += 30 + 22;
    y += 20;
    MoveWindow(g.hEditPass, cx, y, fw, 30, TRUE);   
    y += 30 + 22;
    MoveWindow(g.hBtnLogin, cx, y, fw, 38, TRUE);

    int bw = 150, bh = 50, gap = 16;
    int row1w = bw * 3 + gap * 2;
    int row2w = bw * 2 + gap;
    int startY = 160;

    int x1 = (cw - row1w) / 2;
    for (int i = 0; i < 3; ++i)
        MoveWindow(g.hBtnMain[i], x1 + i * (bw + gap), startY, bw, bh, TRUE);

    int x2 = (cw - row2w) / 2;
    for (int i = 0; i < 2; ++i)
        MoveWindow(g.hBtnMain[3 + i], x2 + i * (bw + gap), startY + bh + gap, bw, bh, TRUE);
}

void ShowLoginScreen()
{
    g.state = AppState::Login;
    ShowWindow(g.hEditUser, SW_SHOW);
    ShowWindow(g.hEditPass, SW_SHOW);
    ShowWindow(g.hBtnLogin, SW_SHOW);
    for (int i = 0; i < 5; ++i) ShowWindow(g.hBtnMain[i], SW_HIDE);
    InvalidateRect(g.hWnd, nullptr, FALSE);
}

void StartLoginLoading()
{
    g.state = AppState::Loading;
    ShowWindow(g.hEditUser, SW_HIDE);
    ShowWindow(g.hEditPass, SW_HIDE);
    ShowWindow(g.hBtnLogin, SW_HIDE);
    SetTimer(g.hWnd, TIMER_LOGIN_LOAD, 5000, nullptr);
    InvalidateRect(g.hWnd, nullptr, FALSE);
}

void ShowMainScreen()
{
    g.state = AppState::Main;
    for (int i = 0; i < 5; ++i) ShowWindow(g.hBtnMain[i], SW_SHOW);
    InvalidateRect(g.hWnd, nullptr, FALSE);
}

void StartButtonLoading(int index)
{
    g.state = AppState::ButtonLoading;
    g.pendingButton = index;
    for (int i = 0; i < 5; ++i) EnableWindow(g.hBtnMain[i], FALSE);
    SetTimer(g.hWnd, TIMER_BTN_LOAD, 1200, nullptr);
    InvalidateRect(g.hWnd, nullptr, FALSE);
}

void FinishButtonLoading()
{
    for (int i = 0; i < 5; ++i) EnableWindow(g.hBtnMain[i], TRUE);
    g.state = AppState::Main;
    InvalidateRect(g.hWnd, nullptr, FALSE);
    MessageBoxW(g.hWnd, L"Cheat Loaded Load Game ;D", L"GigaChad Debug", MB_OK | MB_ICONINFORMATION);
}


void InitSnow(int w, int h)
{
    g.snow.clear();
    int count = 90;
    for (int i = 0; i < count; ++i)
    {
        SnowFlake f;
        f.x = (float)(rand() % (w > 0 ? w : WIN_W));
        f.y = (float)(rand() % (h > 0 ? h : WIN_H));
        f.speed = 0.5f + (rand() % 100) / 70.0f;
        f.drift = ((rand() % 100) / 100.0f - 0.5f) * 0.8f;
        f.phase = (float)(rand() % 628) / 100.0f;
        f.scale = 0.6f + (rand() % 100) / 100.0f * 0.8f; 
        f.bright = (rand() % 3) != 0; 
        g.snow.push_back(f);
    }
}

void UpdateSnow(int w, int h)
{
    if (g.snow.empty()) InitSnow(w, h);
    for (auto& f : g.snow)
    {
        f.phase += 0.04f;
        f.y += f.speed * (f.bright ? 1.0f : 0.7f); 
        f.x += f.drift + sinf(f.phase) * 0.4f;
        if (f.y > h + 6) { f.y = -6; f.x = (float)(rand() % (w > 0 ? w : WIN_W)); }
        if (f.x < -6) f.x = (float)w + 6;
        if (f.x > w + 6) f.x = -6;
    }
}

void DrawSnow(HDC hdc, RECT rc)
{
    BLENDFUNCTION bf{};
    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = 255;
    bf.AlphaFormat = AC_SRC_ALPHA;

    for (auto& f : g.snow)
    {
        int size = (int)(FLAKE_BMP_SIZE * f.scale * 0.6f); 
        if (size < 3) size = 3;
        HDC src = f.bright ? g.flakeDCBright : g.flakeDCDim;
        AlphaBlend(hdc, (int)f.x, (int)f.y, size, size,
            src, 0, 0, FLAKE_BMP_SIZE, FLAKE_BMP_SIZE, bf);
    }
    (void)rc;
}

void DrawSpinner(HDC hdc, int cx, int cy, int radius, int angle)
{
    HPEN old = (HPEN)SelectObject(hdc, g.penSpinnerDim);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, g.brushNull);
    Ellipse(hdc, cx - radius, cy - radius, cx + radius, cy + radius);

    SelectObject(hdc, g.penSpinnerFg);
    double startRad = angle * 3.14159265 / 180.0;
    double sweep = 100.0 * 3.14159265 / 180.0;
    double endRad = startRad + sweep;

    int x1 = cx + (int)(radius * cos(startRad));
    int y1 = cy + (int)(radius * sin(startRad));
    int x2 = cx + (int)(radius * cos(endRad));
    int y2 = cy + (int)(radius * sin(endRad));

    Arc(hdc, cx - radius, cy - radius, cx + radius, cy + radius, x1, y1, x2, y2);

    SelectObject(hdc, old);
    SelectObject(hdc, oldBrush);
}

void EnsureBackBuffer(HDC hdc, int w, int h)
{
    if (g.memDC && g.memW == w && g.memH == h) return;
    if (g.memDC) DeleteDC(g.memDC);
    if (g.memBitmap) DeleteObject(g.memBitmap);
    g.memDC = CreateCompatibleDC(hdc);
    g.memBitmap = CreateCompatibleBitmap(hdc, w, h);
    SelectObject(g.memDC, g.memBitmap);
    g.memW = w; g.memH = h;
}

void PaintScene(HDC hdc, RECT rc)
{
    FillRect(hdc, &rc, g.brushBg);
    SetBkMode(hdc, TRANSPARENT);

    int cw = rc.right, ch = rc.bottom;

    
    HFONT oldFont = (HFONT)SelectObject(hdc, g.fontTitle);
    SetTextColor(hdc, COL_PURPLE);
    RECT titleRc{ 0, 28, cw, 88 };
    DrawTextW(hdc, APP_TITLE, -1, &titleRc, DT_CENTER | DT_SINGLELINE);

    HPEN oldPen = (HPEN)SelectObject(hdc, g.penTitleLine);
    MoveToEx(hdc, cw / 2 - 90, 94, nullptr);
    LineTo(hdc, cw / 2 + 90, 94);
    SelectObject(hdc, oldPen);

    SelectObject(hdc, g.fontSmall);

    if (g.state == AppState::Login)
    {
        int fw = 240;
        int cx = (cw - fw) / 2;

        SetTextColor(hdc, COL_WHITE_TXT);
        RECT lbl{ cx, 130, cx + fw, 130 + 18 };
        DrawTextW(hdc, L"USERNAME", -1, &lbl, DT_LEFT | DT_SINGLELINE);

        RECT lbl2{ cx, 130 + 20 + 30 + 22, cx + fw, 130 + 20 + 30 + 22 + 18 };
        DrawTextW(hdc, L"PASSWORD", -1, &lbl2, DT_LEFT | DT_SINGLELINE);

        if (!g.loginError.empty())
        {
            SetTextColor(hdc, COL_ERROR);
            RECT err{ 0, 330, cw, 352 };
            DrawTextW(hdc, g.loginError.c_str(), -1, &err, DT_CENTER | DT_SINGLELINE);
        }
    }
    else if (g.state == AppState::Loading)
    {
        DrawSpinner(hdc, cw / 2, ch / 2 + 10, 30, g.spinnerAngle);
        SetTextColor(hdc, COL_PURPLE_SOFT);
        RECT lbl{ 0, ch / 2 + 55, cw, ch / 2 + 80 };
        DrawTextW(hdc, L"Loading...", -1, &lbl, DT_CENTER | DT_SINGLELINE);
    }
    else if (g.state == AppState::Main)
    {
        SetTextColor(hdc, COL_PURPLE_SOFT);
        RECT lbl{ 0, 122, cw, 146 };
        DrawTextW(hdc, L"SELECT A Game", -1, &lbl, DT_CENTER | DT_SINGLELINE);
    }
    else if (g.state == AppState::ButtonLoading)
    {
        SetTextColor(hdc, COL_PURPLE_SOFT);
        RECT lbl{ 0, 122, cw, 146 };
        DrawTextW(hdc, L"SELECT A Game", -1, &lbl, DT_CENTER | DT_SINGLELINE);

        DrawSpinner(hdc, cw / 2, ch - 60, 18, g.spinnerAngle);
        RECT lbl2{ 0, ch - 30, cw, ch - 10 };
        DrawTextW(hdc, L"Injecting...", -1, &lbl2, DT_CENTER | DT_SINGLELINE);
    }

    SelectObject(hdc, oldFont);

    DrawSnow(hdc, rc);

    oldPen = (HPEN)SelectObject(hdc, g.penBorder);
    HBRUSH oldB = (HBRUSH)SelectObject(hdc, g.brushNull);
    Rectangle(hdc, 1, 1, cw - 1, ch - 1);
    SelectObject(hdc, oldB);
    SelectObject(hdc, oldPen);
}

void DrawThemedButtonItem(LPDRAWITEMSTRUCT dis, const wchar_t* text, bool enabled)
{
    HDC hdc = dis->hDC;
    RECT rc = dis->rcItem;
    bool pressed = (dis->itemState & ODS_SELECTED) != 0;

    HBRUSH fillBrush = !enabled ? g.brushDisabled : (pressed ? g.brushPressed : g.brushPanel);
    FillRect(hdc, &rc, fillBrush);

    HPEN borderPen = !enabled ? g.penBtnBorderDis : (pressed ? g.penBtnBorder : g.penBtnBorderDim);
    HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, g.brushNull);
    Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, enabled ? COL_PURPLE_SOFT : COL_DISABLED_TX);
    HFONT oldFont = (HFONT)SelectObject(hdc, g.fontButton);
    DrawTextW(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
}
