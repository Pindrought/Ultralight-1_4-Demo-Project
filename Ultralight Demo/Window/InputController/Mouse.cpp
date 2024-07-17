#include <PCH.h>
#include "Mouse.h"

int Mouse::GetPosX()
{
	return m_X;
}

int Mouse::GetPosY()
{
	return m_Y;
}

MousePoint Mouse::GetPos()
{
	return { m_X, m_Y };
}

bool Mouse::EventBufferIsEmpty()
{
	return m_EventBuffer.empty();
}

bool Mouse::ScrollEventBufferIsEmpty()
{
	return m_ScrollEventBuffer.empty();
}

MouseEvent Mouse::ReadEvent()
{
	if (m_EventBuffer.empty())
	{
		return MouseEvent();
	}
	else
	{
		MouseEvent e = m_EventBuffer.front(); //Get first event from buffer
		m_EventBuffer.pop(); //Remove first event from buffer
		return e;
	}
}

ScrollEvent Mouse::ReadScrollEvent()
{
	if (m_ScrollEventBuffer.empty())
	{
		return ScrollEvent();
	}
	else
	{
		ScrollEvent e = m_ScrollEventBuffer.front(); //Get first event from buffer
		m_ScrollEventBuffer.pop(); //Remove first event from buffer
		return e;
	}
}

void Mouse::OnWindowsMouseMessage(uint32_t windowId, 
								  UINT uMsg,
								  WPARAM wParam, 
								  LPARAM lParam)
{
	//LOGINFO(strfmt("MouseMessage Window: %d", (int)windowId).c_str());
	MouseEvent mouseEvent(windowId, uMsg, wParam, lParam);
	switch (mouseEvent.m_Type)
	{
	case MouseEvent::Type::MouseDown:
		m_LastPressedButton = mouseEvent.m_Button;
		break;
	case MouseEvent::Type::MouseUp:
		m_LastPressedButton = MouseEvent::Button::None;
		break;
	case MouseEvent::Type::MouseMove:
		mouseEvent.m_Button = m_LastPressedButton;
		break;
	}
	m_EventBuffer.push(mouseEvent);
}

void Mouse::OnWindowsScrollMessage(uint32_t windowId,
								  UINT uMsg,
								  WPARAM wParam,
								  LPARAM lParam)
{
	//LOGINFO(strfmt("MouseScroll Window: %d", (int)windowId).c_str());
	int16_t x = LOWORD(wParam);
	int16_t y = HIWORD(wParam);
	ScrollEvent scrollEvent(windowId, x, y);
	
	m_ScrollEventBuffer.push(scrollEvent);
}

void Mouse::OnMouseMoveRaw(uint32_t windowId, int x, int y)
{
	//LOGINFO(strfmt("MouseMoveRaw Window: %d", (int)windowId).c_str());
	MouseEvent ev;
	ev.m_Type = MouseEvent::Type::MouseMoveRaw;
	ev.m_Button = m_LastPressedButton;
	ev.m_X = x;
	ev.m_Y = y;
	ev.m_WindowId = windowId;
	m_EventBuffer.push(ev);
}