#include "PCH.h"
#include "ScrollEvent.h"

ScrollEvent::ScrollEvent()
{
}

ScrollEvent::ScrollEvent(uint32_t windowId, int deltaX, int deltaY)
{
	m_Type = Type::ScrollByPixel;
	m_WindowId = windowId;
	m_DeltaX = deltaX;
	m_DeltaY = deltaY;
}

int ScrollEvent::GetDeltaX() const
{
	return m_DeltaX;
}

int ScrollEvent::GetDeltaY() const
{
	return m_DeltaY;
}

bool ScrollEvent::IsValid() const
{
	return (m_Type != Type::Invalid);
}

ultralight::ScrollEvent ScrollEvent::ToUltralightScrollEvent()
{
	ultralight::ScrollEvent ulEvent;
	ulEvent.type = (ultralight::ScrollEvent::Type)m_Type;
	ulEvent.delta_x = m_DeltaX;
	ulEvent.delta_y = m_DeltaY;
	return ulEvent;
}

uint32_t ScrollEvent::GetWindowId() const
{
	return m_WindowId;
}