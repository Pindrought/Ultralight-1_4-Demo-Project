#pragma once
#include <PCH.h>
#include "MousePoint.h"



class MouseEvent
{
	friend class Mouse;
public:
	enum class Type
	{
		MouseMove = ultralight::MouseEvent::Type::kType_MouseMoved,
		MouseDown = ultralight::MouseEvent::Type::kType_MouseDown,
		MouseUp = ultralight::MouseEvent::Type::kType_MouseUp,
		MouseMoveRaw,
		UninitializedType
	};
	enum class Button
	{
		None = 0,
		Left = ultralight::MouseEvent::Button::kButton_Left,
		Middle = ultralight::MouseEvent::Button::kButton_Middle,
		Right = ultralight::MouseEvent::Button::kButton_Right,
		UninitializedButton
	};
public:
	MouseEvent();
	MouseEvent(uint32_t windowId, UINT uMsg, WPARAM wParam, LPARAM lParam);
	MouseEvent(uint32_t windowId, Type type, Button button, int x, int y);
	bool IsValid() const;
	Type GetType() const;
	Button GetButton() const;
	MousePoint GetPos() const;
	int GetPosX() const;
	int GetPosY() const;
	ultralight::MouseEvent ToUltralightMouseEvent();
	uint32_t GetWindowId() const;
private:
	Type m_Type = Type::UninitializedType;
	Button m_Button = Button::UninitializedButton;
	int m_X;
	int m_Y;
	int32_t m_WindowId = -1;
};

