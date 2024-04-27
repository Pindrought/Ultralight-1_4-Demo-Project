#include <PCH.h>
#include "KeyboardEvent.h"

KeyboardEvent::KeyboardEvent()
	:
	m_Type(Type::Invalid),
	m_Key(0u),
	m_IsAutoRepeat(false)
{
}

KeyboardEvent::KeyboardEvent(uint32_t windowId, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_WindowId = windowId;
	m_WParam = wParam;
	m_LParam = lParam;
	unsigned char keycode = static_cast<unsigned char>(m_WParam);
	m_Key = keycode;
	switch (uMsg)
	{
	case WM_KEYDOWN:
		m_Type = KeyboardEvent::Type::KeyDown;
		m_IsAutoRepeat = lParam & 0x40000000;
		break;
	case WM_KEYUP:
		m_Type = KeyboardEvent::Type::KeyUp;
		m_IsAutoRepeat = false;
		break;
	case WM_SYSKEYDOWN:
		m_Type = KeyboardEvent::Type::KeyDown;
		m_IsAutoRepeat = lParam & 0x40000000;
		m_IsSystemKey = true;
		break;
	case WM_SYSKEYUP:
		m_Type = KeyboardEvent::Type::KeyUp;
		m_IsAutoRepeat = false;
		m_IsSystemKey = true;
		break;
	case WM_CHAR:
		m_Type = KeyboardEvent::Type::Char;
		m_IsAutoRepeat = lParam & 0x40000000;
		break;
	default:
		m_Type = KeyboardEvent::Type::Invalid;
		break;
	}
}

KeyboardEvent::Type KeyboardEvent::GetType()
{
	return m_Type;
}

bool KeyboardEvent::IsKeyDown() const
{
	return m_Type == Type::KeyDown;
}

bool KeyboardEvent::IsKeyUp() const
{
	return m_Type == Type::KeyUp;
}

bool KeyboardEvent::IsAutoRepeat() const
{
	return m_IsAutoRepeat;
}

bool KeyboardEvent::IsValid() const
{
	return m_Type != Type::Invalid;
}

bool KeyboardEvent::IsSystemKey() const
{
	return m_IsSystemKey;
}

unsigned char KeyboardEvent::GetKeyCode() const
{
	return m_Key;
}

uint32_t KeyboardEvent::GetWindowId() const
{
	return m_WindowId;
}

ultralight::KeyEvent KeyboardEvent::ToUltralightKeyboardEvent()
{
	return ultralight::KeyEvent(static_cast<ultralight::KeyEvent::Type>(m_Type), 
								m_WParam, 
								m_LParam, 
								m_IsSystemKey);
}