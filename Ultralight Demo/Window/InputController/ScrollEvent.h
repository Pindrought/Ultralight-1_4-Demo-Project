#pragma once
#include <PCH.h>
#include "MousePoint.h"

class ScrollEvent
{
	friend class Mouse;
public:
	enum class Type
	{
		ScrollByPixel = ultralight::ScrollEvent::kType_ScrollByPixel,
		ScrollByPage = ultralight::ScrollEvent::kType_ScrollByPage, //Idk when you'd ever use this
		Invalid
	};
public:
	ScrollEvent();
	ScrollEvent(uint32_t windowId, int deltaX, int deltaY);
	int GetDeltaX() const;
	int GetDeltaY() const;
	bool IsValid() const;
	ultralight::ScrollEvent ToUltralightScrollEvent();
	uint32_t GetWindowId() const;
private:
	Type m_Type = Type::Invalid;
	int m_DeltaX = 0;
	int m_DeltaY = 0;
	uint32_t m_WindowId = 0;
};

