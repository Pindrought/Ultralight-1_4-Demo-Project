#include <PCH.h>
#include "WindowHelperForBorderlessResizable.h"
#include "Window.h"

void WindowHelperForBorderlessResizable::HandleCompositionChanged(Window* pWindow)
{
	BOOL enabled = FALSE;
	DwmIsCompositionEnabled(&enabled);
	pWindow->m_BRWData.CompositionEnabled = enabled;

	if (enabled) 
	{
		const HWND hwnd = pWindow->m_HWND;
		/* The window needs a frame to show a shadow, so give it the smallest
		   amount of frame possible */
		MARGINS margins = { 0, 0, 1, 0 };
		DwmExtendFrameIntoClientArea(hwnd, &margins);
		DWORD attribute = DWMNCRP_ENABLED;
		DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY,
							  &attribute, sizeof(DWORD));
	}

	UpdateRegion(pWindow);
}

void WindowHelperForBorderlessResizable::UpdateRegion(Window* pWindow)
{
	RECT old_rgn = pWindow->m_BRWData.Region;
	const HWND hwnd = pWindow->m_HWND;

	if (IsMaximized(hwnd)) {
		WINDOWINFO wi;
		wi.cbSize = sizeof(wi);
		GetWindowInfo(hwnd, &wi);

		/* For maximized windows, a region is needed to cut off the non-client
		   borders that hang over the edge of the screen */
		pWindow->m_BRWData.Region = {
			wi.rcClient.left - wi.rcWindow.left,
			wi.rcClient.top - wi.rcWindow.top,
			wi.rcClient.right - wi.rcWindow.left,
			wi.rcClient.bottom - wi.rcWindow.top,
		};
	}
	else if (!pWindow->m_BRWData.CompositionEnabled) {
		/* For ordinary themed windows when composition is disabled, a region
		   is needed to remove the rounded top corners. Make it as large as
		   possible to avoid having to change it when the window is resized. */
		pWindow->m_BRWData.Region = {
			 0,
			 0,
			32767,
			32767,
		};
	}
	else {
		/* Don't mess with the region when composition is enabled and the
		   window is not maximized, otherwise it will lose its shadow */
		pWindow->m_BRWData.Region = { 0 };
	}

	/* Avoid unnecessarily updating the region to avoid unnecessary redraws */
	if (EqualRect(&pWindow->m_BRWData.Region, &old_rgn))
		return;
	/* Treat empty regions as NULL regions */
	RECT r2 = { 0 };
	if (EqualRect(&pWindow->m_BRWData.Region, &r2))
		SetWindowRgn(hwnd, NULL, TRUE);
	else
		SetWindowRgn(hwnd, CreateRectRgnIndirect(&pWindow->m_BRWData.Region), TRUE);
}

void WindowHelperForBorderlessResizable::HandleThemeChanged(Window* pWindow)
{
	pWindow->m_BRWData.ThemeEnabled = IsThemeActive();
}

static bool has_autohide_appbar(UINT edge, RECT mon)
{
	if (IsWindows8Point1OrGreater()) {
		APPBARDATA abd;
		abd.cbSize = sizeof(abd);
		abd.uEdge = edge;
		abd.rc = mon;
		return SHAppBarMessage(ABM_GETAUTOHIDEBAREX, &abd);
	}

	/* Before Windows 8.1, it was not possible to specify a monitor when
	   checking for hidden appbars, so check only on the primary monitor */
	if (mon.left != 0 || mon.top != 0)
		return false;
	APPBARDATA abd;
	abd.cbSize = sizeof(abd);
	abd.uEdge = edge;
	return SHAppBarMessage(ABM_GETAUTOHIDEBAR, &abd);
}

void WindowHelperForBorderlessResizable::HandleNCCalcSize(Window* pWindow, WPARAM wParam, LPARAM lParam)
{
	union {
		LPARAM lparam;
		RECT* rect;
	} params = { lParam };

	/* DefWindowProc must be called in both the maximized and non-maximized
	   cases, otherwise tile/cascade windows won't work */
	RECT nonclient = *params.rect;
	HWND hwnd = pWindow->m_HWND;
	DefWindowProcA(hwnd, WM_NCCALCSIZE, wParam, params.lparam);
	RECT client = *params.rect;

	if (IsMaximized(hwnd)) 
	{
		WINDOWINFO wi = { 0 };
		wi.cbSize = sizeof(wi);
		GetWindowInfo(hwnd, &wi);

		/* Maximized windows always have a non-client border that hangs over
		   the edge of the screen, so the size proposed by WM_NCCALCSIZE is
		   fine. Just adjust the top border to remove the window title. */
		*params.rect = {
			client.left,
			nonclient.top + (long)wi.cyWindowBorders,
			client.right,
			client.bottom,
		};

		HMONITOR mon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFO mi = { 0 };
		mi.cbSize = sizeof(mi);
		GetMonitorInfoW(mon, &mi);

		/* If the client rectangle is the same as the monitor's rectangle,
		   the shell assumes that the window has gone fullscreen, so it removes
		   the topmost attribute from any auto-hide appbars, making them
		   inaccessible. To avoid this, reduce the size of the client area by
		   one pixel on a certain edge. The edge is chosen based on which side
		   of the monitor is likely to contain an auto-hide appbar, so the
		   missing client area is covered by it. */
		if (EqualRect(params.rect, &mi.rcMonitor)) {
			if (has_autohide_appbar(ABE_BOTTOM, mi.rcMonitor))
				params.rect->bottom--;
			else if (has_autohide_appbar(ABE_LEFT, mi.rcMonitor))
				params.rect->left++;
			else if (has_autohide_appbar(ABE_TOP, mi.rcMonitor))
				params.rect->top++;
			else if (has_autohide_appbar(ABE_RIGHT, mi.rcMonitor))
				params.rect->right--;
		}
	}
	else {
		/* For the non-maximized case, set the output RECT to what it was
		   before WM_NCCALCSIZE modified it. This will make the client size the
		   same as the non-client size. */
		*params.rect = nonclient;
	}
}

LRESULT WindowHelperForBorderlessResizable::HandleNCHitTest(Window* pWindow, int x, int y)
{
	HWND hwnd = pWindow->m_HWND;
	if (IsMaximized(hwnd))
		return HTCLIENT;

	POINT mouse = { x, y };
	ScreenToClient(hwnd, &mouse);

	/* The horizontal frame should be the same size as the vertical frame,
	   since the NONCLIENTMETRICS structure does not distinguish between them */
	int frame_size = GetSystemMetrics(SM_CXFRAME) +
		GetSystemMetrics(SM_CXPADDEDBORDER);
	/* The diagonal size handles are wider than the frame */
	int diagonal_width = frame_size * 2 + GetSystemMetrics(SM_CXBORDER);

	if (mouse.y < frame_size) {
		if (mouse.x < diagonal_width)
			return HTTOPLEFT;
		if (mouse.x >= pWindow->GetWidth() - diagonal_width)
			return HTTOPRIGHT;
		return HTTOP;
	}

	if (mouse.y >= pWindow->GetHeight() - frame_size) {
		if (mouse.x < diagonal_width)
			return HTBOTTOMLEFT;
		if (mouse.x >= pWindow->GetWidth() - diagonal_width)
			return HTBOTTOMRIGHT;
		return HTBOTTOM;
	}

	if (mouse.x < frame_size)
		return HTLEFT;
	if (mouse.x >= pWindow->GetWidth() - frame_size)
		return HTRIGHT;
	return HTCLIENT;
}
