#include <PCH.h>
#include "CursorManager.h"

HCURSOR CursorManager::s_CursorArrow = NULL;
HCURSOR CursorManager::s_CursorIBeam = NULL;
HCURSOR CursorManager::s_CursorHand = NULL;

HCURSOR CursorManager::s_CursorSizeNWSE = NULL;
HCURSOR CursorManager::s_CursorSizeNESW = NULL;
HCURSOR CursorManager::s_CursorSizeWE = NULL;
HCURSOR CursorManager::s_CursorSizeNS = NULL;
HCURSOR CursorManager::s_CursorSizeALL = NULL;

void CursorManager::Initialize()
{
	s_CursorHand = LoadCursor(NULL, IDC_HAND);
	s_CursorArrow = LoadCursor(NULL, IDC_ARROW);
	s_CursorIBeam = LoadCursor(NULL, IDC_IBEAM);

	s_CursorSizeNWSE = LoadCursor(NULL, IDC_SIZENWSE);
	s_CursorSizeNESW = LoadCursor(NULL, IDC_SIZENESW);
	s_CursorSizeWE = LoadCursor(NULL, IDC_SIZEWE);
	s_CursorSizeNS = LoadCursor(NULL, IDC_SIZENS);
	s_CursorSizeALL = LoadCursor(NULL, IDC_SIZEALL);
}

void CursorManager::SetCursor(CursorType cursor)
{
	switch (cursor)
	{
	case CursorType::ARROW:
		::SetCursor(s_CursorArrow);
		break;
	case CursorType::IBEAM:
		::SetCursor(s_CursorIBeam);
		break;
	case CursorType::HAND:
		::SetCursor(s_CursorHand);
		break;
	case CursorType::MIDDLEPANNING:
		::SetCursor(s_CursorSizeALL);
		break;
	case CursorType::SIZE_NWSE:
		::SetCursor(s_CursorSizeNWSE);
		break;
	case CursorType::SIZE_NESW:
		::SetCursor(s_CursorSizeNESW);
		break;
	case CursorType::SIZE_NS:
		::SetCursor(s_CursorSizeNS);
		break;
	case CursorType::SIZE_WE:
		::SetCursor(s_CursorSizeWE);
		break;
	default:
		assert(false && "Set cursor to invalid cursor type?");
	}
}

CursorType CursorManager::GetCursor()
{

	CURSORINFO cursorInfo = { 0 };
	cursorInfo.cbSize = sizeof(cursorInfo);

	if (::GetCursorInfo(&cursorInfo))
	{
		HCURSOR hCursor = cursorInfo.hCursor;
		if (hCursor == s_CursorArrow)
		{
			return CursorType::ARROW;
		}
		if (hCursor == s_CursorIBeam)
		{
			return CursorType::IBEAM;
		}
		if (hCursor == s_CursorHand)
		{
			return CursorType::HAND;
		}
		if (hCursor == s_CursorSizeNWSE ||
			hCursor == s_CursorSizeNESW ||
			hCursor == s_CursorSizeWE ||
			hCursor == s_CursorSizeNS)
		{
			return CursorType::RESIZE;
		}
		if (hCursor == s_CursorSizeALL)
		{
			return CursorType::MIDDLEPANNING;
		}
	}

	return CursorType::UNKNOWN;
}

