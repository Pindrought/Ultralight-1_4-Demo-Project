#pragma once
#include <PCH.h>
#include "Keyboard.h"
#include "Mouse.h"

class InputController
{
public:
	void ClearEventsForWindow(int32_t windowId);
	Mouse m_Mouse;
	Keyboard m_Keyboard;
};