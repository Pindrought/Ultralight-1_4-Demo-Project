#pragma once
#include <PCH.h>
//For whatever reason, making a borderless resizable window is a lot of work and the documentation is lacking.
//This implementation was created by referencing https://github.com/rossy/borderless-window/blob/master/borderless-window.c
//This helper will only be called from the WindowProc in Window class when a window is both borderless AND resizable

#ifndef WM_NCUAHDRAWCAPTION
#define WM_NCUAHDRAWCAPTION (0x00AE)
#endif
#ifndef WM_NCUAHDRAWFRAME
#define WM_NCUAHDRAWFRAME (0x00AF)
#endif

struct BorderlessResizableWindowData
{
	HWND Hwnd = NULL;
	uint32_t Width = 0;
	uint32_t Height = 0;
	RECT Region = { 0 };
	bool ThemeEnabled = false;
	bool CompositionEnabled = false;
};

class WindowHelperForBorderlessResizable
{
public:
	static void HandleCompositionChanged(Window* pWindow);
	static void UpdateRegion(Window* pWindow);
	static void HandleThemeChanged(Window* pWindow);
	static void HandleNCCalcSize(Window* pWindow, WPARAM wParam, LPARAM lParam);
	static LRESULT HandleNCHitTest(Window* pWindow, int x, int y);
};