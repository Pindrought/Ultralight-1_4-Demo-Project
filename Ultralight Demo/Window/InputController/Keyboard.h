#pragma once
#include <PCH.h>
#include "KeyboardEvent.h"

class Keyboard
{
	friend class Window;
public:
	Keyboard();
	bool KeyIsPressed(const unsigned char keycode);
	bool EventBufferIsEmpty();
	KeyboardEvent ReadEvent();
private:
	void OnWindowsKeyboardMessage(uint32_t windowId, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool m_KeyStates[256] = { false };
	std::queue<KeyboardEvent> m_EventBuffer;
};

