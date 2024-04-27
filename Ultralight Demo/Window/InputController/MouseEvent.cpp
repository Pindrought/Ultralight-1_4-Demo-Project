#include <PCH.h>
#include "MouseEvent.h"

MouseEvent::MouseEvent()
	:
	m_Type(Type::UninitializedType),
	m_Button(Button::UninitializedButton),
	m_X(0),
	m_Y(0)
{}

MouseEvent::MouseEvent(uint32_t windowId, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_WindowId = windowId;
	m_X = LOWORD(lParam);
	m_Y = HIWORD(lParam);
	switch (uMsg)
	{
	case WM_MOUSEMOVE:
		m_Button = Button::None;
		m_Type = Type::MouseMove;
		break;
	case WM_LBUTTONDOWN:
		m_Button = Button::Left;
		m_Type = Type::MouseDown;
		break;
	case WM_RBUTTONDOWN:
		m_Button = Button::Right;
		m_Type = Type::MouseDown;
		break;
	case WM_MBUTTONDOWN:
		m_Button = Button::Middle;
		m_Type = Type::MouseDown;
		break;
	case WM_LBUTTONUP:
		m_Button = Button::Left;
		m_Type = Type::MouseUp;
		break;
	case WM_RBUTTONUP:
		m_Button = Button::Right;
		m_Type = Type::MouseUp;
		break;
	case WM_MBUTTONUP:
		m_Button = Button::Middle;
		m_Type = Type::MouseUp;
		break;
	default:
		break;
	}
}

MouseEvent::MouseEvent(uint32_t windowId, Type type, Button button, int x, int y)
	:m_WindowId(windowId), m_Type(type), m_Button(button), m_X(x), m_Y(y)
{
}

bool MouseEvent::IsValid() const
{
	return (m_Type != Type::UninitializedType && 
			m_Button != Button::UninitializedButton);
}

MouseEvent::Type MouseEvent::GetType() const
{
	return m_Type;
}

MouseEvent::Button MouseEvent::GetButton() const
{
	return m_Button;
}

MousePoint MouseEvent::GetPos() const
{
	return { m_X, m_Y };
}

int MouseEvent::GetPosX() const
{
	return m_X;
}

int MouseEvent::GetPosY() const
{
	return m_Y;
}

ultralight::MouseEvent MouseEvent::ToUltralightMouseEvent()
{
	ultralight::MouseEvent mouseEvent{};
	mouseEvent.button = static_cast<ultralight::MouseEvent::Button>(m_Button);
	mouseEvent.type = static_cast<ultralight::MouseEvent::Type>(m_Type);
	mouseEvent.x = m_X;
	mouseEvent.y = m_Y;
	return mouseEvent;
}

uint32_t MouseEvent::GetWindowId() const
{
	return m_WindowId;
}