#pragma once
#include "MouseEvent.h"
#include "ScrollEvent.h"

class Mouse
{
	friend class Window;
public:
	int GetPosX();
	int GetPosY();
	MousePoint GetPos();
	bool EventBufferIsEmpty();
	bool ScrollEventBufferIsEmpty();

	MouseEvent ReadEvent();
	ScrollEvent ReadScrollEvent();
private:
	void OnWindowsMouseMessage(uint32_t windowId, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnWindowsScrollMessage(uint32_t windowId, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnMouseMoveRaw(uint32_t windowId, int x, int y);
	MouseEvent::Button m_LastPressedButton = MouseEvent::Button::None;

	std::queue<MouseEvent> m_EventBuffer;
	std::queue<ScrollEvent> m_ScrollEventBuffer;
	int m_X = 0;
	int m_Y = 0;
};

