#include "PCH.h"
#include "ScreenCapper.h"
#include <fstream>

struct handle_data {
    unsigned long process_id;
    HWND window_handle;
};

BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lparam) {
    auto& data = *reinterpret_cast<handle_data*>(lparam);

    unsigned long process_id = 0;
    GetWindowThreadProcessId(handle, &process_id);

    if (data.process_id != process_id) {
        return TRUE;
    }

    data.window_handle = handle;
    return FALSE;
}

HWND find_main_window() {
    handle_data data{};

    data.process_id = GetCurrentProcessId();
    data.window_handle = nullptr;
    EnumWindows(enum_windows_callback, reinterpret_cast<LPARAM>(&data));

    return data.window_handle;
}


bool ScreenCapper::Initialize()
{
    m_DesktopDC = GetDC(GetDesktopWindow()); //Get device context for entire screen

    if (m_DesktopDC == NULL)
    {
        system("Failed to get hdc.");
        return 0;
    }

    m_CaptureDC = CreateCompatibleDC(m_DesktopDC);
    if (m_CaptureDC == NULL)
    {
        system("Failed to create compatible DC.");
        return false;
    }

    return true;
}

bool ScreenCapper::AssignRegion(int x, int y, int width, int height)
{
    if (m_Pixels)
        delete[] m_Pixels;
    m_Pixels = new RGBQUAD[width * height];

    this->m_X = x;
    this->m_Y = y;
    this->m_Width = width;
    this->m_Height = height;
    m_HBitmap = CreateCompatibleBitmap(m_DesktopDC, width, height);
    if (m_HBitmap == NULL)
        return false;
    if (SelectObject(m_CaptureDC, m_HBitmap) == NULL)
        return false;

    return true;
}

bool ScreenCapper::CaptureRegion()
{
    
    BOOL result = BitBlt(m_CaptureDC, 0, 0, m_Width, m_Height, m_DesktopDC,
                            m_X, m_Y, SRCCOPY | CAPTUREBLT);
    if (result == 0)
        return false;

    // Step 2: Get cursor information
    CURSORINFO cursorInfo = { 0 };
    cursorInfo.cbSize = sizeof(cursorInfo);
    if (GetCursorInfo(&cursorInfo) && (cursorInfo.flags & CURSOR_SHOWING))
    {
        // Step 3: Get the cursor icon
        ICONINFO iconInfo = { 0 };
        if (GetIconInfo(cursorInfo.hCursor, &iconInfo))
        {
            int cursorX = cursorInfo.ptScreenPos.x - m_X - iconInfo.xHotspot;
            int cursorY = cursorInfo.ptScreenPos.y - m_Y - iconInfo.yHotspot;

            // Step 4: Draw the cursor on the capture device context
            DrawIcon(m_CaptureDC, cursorX, cursorY, cursorInfo.hCursor);

            // Clean up icon info
            DeleteObject(iconInfo.hbmMask);
            DeleteObject(iconInfo.hbmColor);
        }
    }

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth = m_Width;
    bmi.bmiHeader.biHeight = m_Height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    result = GetDIBits(
        m_CaptureDC,
        m_HBitmap,
        0,
        m_Height,
        m_Pixels,
        &bmi,
        DIB_RGB_COLORS
    );
    return (result != 0);
}

ScreenCapper::~ScreenCapper()
{
    ReleaseDC(NULL, m_DesktopDC);
    DeleteDC(m_CaptureDC);
    DeleteObject(m_HBitmap);
}