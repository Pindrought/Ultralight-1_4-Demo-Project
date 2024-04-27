#pragma once
#include <PCH.h>

class KeyboardEvent
{
public:
	enum class Type
	{
		Invalid = 0,
		KeyDown = ultralight::KeyEvent::kType_RawKeyDown,
		KeyUp = ultralight::KeyEvent::kType_KeyUp,
		Char = ultralight::KeyEvent::kType_Char,
	};

	KeyboardEvent();
	KeyboardEvent(uint32_t windowId, UINT uMsg, WPARAM wParam, LPARAM lParam);
	Type GetType();
	bool IsKeyDown() const;
	bool IsKeyUp() const;
	bool IsAutoRepeat() const;
	bool IsValid() const;
	bool IsSystemKey() const;
	unsigned char GetKeyCode() const;
	uint32_t GetWindowId() const;
	ultralight::KeyEvent ToUltralightKeyboardEvent();

private:
	WPARAM m_WParam = NULL;
	LPARAM m_LParam = NULL;
	Type m_Type = Type::Invalid;
	wchar_t m_Key = 0;
	bool m_IsAutoRepeat = false;
	bool m_IsSystemKey = false;
	int32_t m_WindowId = -1;
};

