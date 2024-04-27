#pragma once
#include <PCH.h>

enum class CursorType
{
	ARROW,
	IBEAM,
	HAND,
	RESIZE,
	SIZE_NWSE,
	SIZE_NESW,
	SIZE_WE,
	SIZE_NS,
	MIDDLEPANNING,
	UNKNOWN
};

class CursorManager
{
public:
	static void Initialize();
	static void SetCursor(CursorType cursor);
	static CursorType GetCursor();
private:
	static HCURSOR s_CursorArrow;
	static HCURSOR s_CursorIBeam;
	static HCURSOR s_CursorHand;

	static HCURSOR s_CursorSizeNWSE;
	static HCURSOR s_CursorSizeNESW;
	static HCURSOR s_CursorSizeWE;
	static HCURSOR s_CursorSizeNS;
	static HCURSOR s_CursorSizeALL;
};