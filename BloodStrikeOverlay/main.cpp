#include <windows.h>
#include <vector>
#include <string>
#include <cmath>

struct ToggleButton
{
    std::wstring label;
    bool* value;
    RECT rect;
};

static bool g_visible = true;
static bool g_dragging = false;
static POINT g_dragStartMouse;
static POINT g_dragStartWindow;
static bool g_aimbot = false;
static bool g_esp = false;
static bool g_skeleton = false;
static bool g_names = false;
static bool g_fly = false;

static RECT g_windowRect = { 120, 120, 470, 430 };
static RECT g_titleBarRect = { 120, 120, 470, 155 };

static std::vector<ToggleButton> g_buttons;

static void CreateButtons()
{
    g_buttons.clear();

    ToggleButton items[] = {
        { L"Aimbot", &g_aimbot },
        { L"ESP", &g_esp },
        { L"Skeleton", &g_skeleton },
        { L"Names", &g_names },
        { L"Fly", &g_fly },
    };

    for (size_t i = 0; i < 5; ++i)
    {
        RECT r = { g_windowRect.left + 24, g_windowRect.top + 44 + (int)i * 52, g_windowRect.left + 300, g_windowRect.top + 80 + (int)i * 52 };
        g_buttons.push_back({ items[i].label, items[i].value, r });
    }
}

static bool PointInRect(const POINT& p, const RECT& rect)
{
    return p.x >= rect.left && p.x <= rect.right && p.y >= rect.top && p.y <= rect.bottom;
}

static void DrawToggle(HDC hdc, const RECT& rect, const std::wstring& text, bool enabled)
{
    HBRUSH bgBrush = CreateSolidBrush(enabled ? RGB(32, 168, 100) : RGB(62, 62, 62));
    HPEN borderPen = CreatePen(PS_SOLID, 2, RGB(18, 18, 18));
    HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, bgBrush);
    HPEN oldPen = (HPEN)SelectObject(hdc, borderPen);

    Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

    SelectObject(hdc, oldPen);
    DeleteObject(borderPen);

    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    RECT textRect = { rect.left + 18, rect.top + 8, rect.right - 8, rect.bottom - 8 };
    DrawTextW(hdc, text.c_str(), -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    RECT switchRect = { rect.right - 42, rect.top + 8, rect.right - 14, rect.bottom - 8 };
    HBRUSH knobBrush = CreateSolidBrush(enabled ? RGB(220, 220, 220) : RGB(140, 140, 140));
    HBRUSH oldBrush2 = (HBRUSH)SelectObject(hdc, knobBrush);
    Rectangle(hdc, switchRect.left, switchRect.top, switchRect.right, switchRect.bottom);
    SelectObject(hdc, oldBrush2);
    DeleteObject(knobBrush);

    int knobX = enabled ? switchRect.right - 14 : switchRect.left + 2;
    int knobY = switchRect.top + 2;
    FillRect(hdc, &(RECT{ knobX, knobY, knobX + 12, knobY + 12 }), CreateSolidBrush(enabled ? RGB(255, 255, 255) : RGB(90, 90, 90)));

    SelectObject(hdc, oldBrush);
    DeleteObject(bgBrush);
}

static void PaintWindow(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    RECT client;
    GetClientRect(hwnd, &client);

    HBRUSH bg = CreateSolidBrush(RGB(12, 12, 18));
    FillRect(hdc, &client, bg);
    DeleteObject(bg);

    // Title bar
    RECT titleBar = { 0, 0, client.right, 38 };
    HBRUSH titleBrush = CreateSolidBrush(RGB(22, 22, 32));
    FillRect(hdc, &titleBar, titleBrush);
    DeleteObject(titleBrush);

    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);
    RECT titleText = { 12, 7, client.right - 12, 32 };
    DrawTextW(hdc, L"Blood Strike - Dev Overlay", -1, &titleText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // Controls
    for (const auto& btn : g_buttons)
    {
        DrawToggle(hdc, btn.rect, btn.label, *btn.value);
    }

    RECT statusRect = { 20, client.bottom - 80, client.right - 20, client.bottom - 20 };
    HBRUSH statusBrush = CreateSolidBrush(RGB(22, 22, 32));
    FillRect(hdc, &statusRect, statusBrush);
    DeleteObject(statusBrush);

    std::wstring status = L"Status: Ready";
    SetTextColor(hdc, *g_aimbot || *g_esp || *g_skeleton || *g_names || *g_fly ? RGB(80, 220, 140) : RGB(220, 220, 220));
    RECT statusText = { 30, client.bottom - 68, client.right - 30, client.bottom - 24 };
    DrawTextW(hdc, status.c_str(), -1, &statusText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    EndPaint(hwnd, &ps);
}

static void UpdateWindowRectFromControls()
{
    g_windowRect = { 120, 120, 470, 430 };
    g_titleBarRect = { 120, 120, 470, 155 };
    CreateButtons();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
        UpdateWindowRectFromControls();
        return 0;

    case WM_KEYDOWN:
        if (wParam == VK_INSERT)
        {
            g_visible = !g_visible;
            ShowWindow(hwnd, g_visible ? SW_SHOW : SW_HIDE);
        }
        return 0;

    case WM_LBUTTONDOWN:
    {
        POINT pt = { LOWORD(lParam), HIWORD(lParam) };
        if (PointInRect(pt, g_titleBarRect))
        {
            g_dragging = true;
            g_dragStartMouse = pt;
            RECT wRect;
            GetWindowRect(hwnd, &wRect);
            g_dragStartWindow = { wRect.left, wRect.top };
            SetCapture(hwnd);
        }

        for (const auto& btn : g_buttons)
        {
            if (PointInRect(pt, btn.rect))
            {
                *btn.value = !(*btn.value);
                InvalidateRect(hwnd, NULL, TRUE);
                return 0;
            }
        }
        return 0;
    }

    case WM_LBUTTONUP:
        if (g_dragging)
        {
            g_dragging = false;
            ReleaseCapture();
        }
        return 0;

    case WM_MOUSEMOVE:
        if (g_dragging)
        {
            POINT pt = { LOWORD(lParam), HIWORD(lParam) };
            RECT wRect;
            GetWindowRect(hwnd, &wRect);
            int dx = pt.x - g_dragStartMouse.x;
            int dy = pt.y - g_dragStartMouse.y;
            SetWindowPos(hwnd, HWND_TOPMOST, g_dragStartWindow.x + dx, g_dragStartWindow.y + dy, 0, 0,
                SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOOWNERZORDER);
        }
        return 0;

    case WM_PAINT:
        PaintWindow(hwnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
    const wchar_t CLASS_NAME[] = L"BloodStrikeOverlayWindowClass";

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassEx(&wc);

    UpdateWindowRectFromControls();

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
        CLASS_NAME,
        L"Blood Strike Dev Overlay",
        WS_POPUP,
        120, 120, 350, 320,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd)
        return -1;

    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 220, LWA_ALPHA);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
