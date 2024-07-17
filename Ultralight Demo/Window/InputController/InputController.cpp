#include "PCH.h"
#include "InputController.h"

void InputController::ClearEventsForWindow(int32_t windowId)
{
	auto tempMouseEvents = m_Mouse.m_EventBuffer;
	auto tempMouseScrollEvents = m_Mouse.m_ScrollEventBuffer;
	auto tempKeyboardEvents = m_Keyboard.m_EventBuffer;

	m_Mouse.m_EventBuffer = std::queue<MouseEvent>();
	m_Mouse.m_ScrollEventBuffer = std::queue<ScrollEvent>();
	m_Keyboard.m_EventBuffer = std::queue<KeyboardEvent>();

	while (tempMouseEvents.empty() == false)
	{
		MouseEvent evt = tempMouseEvents.front();
		tempMouseEvents.pop();
		if (evt.GetWindowId() != windowId)
		{
			m_Mouse.m_EventBuffer.push(evt);
		}
		else
		{
			LOGINFO("Cleaned up mouse event.");
		}
	}

	while (tempMouseScrollEvents.empty() == false)
	{
		ScrollEvent evt = tempMouseScrollEvents.front();
		tempMouseScrollEvents.pop();
		if (evt.GetWindowId() != windowId)
		{
			m_Mouse.m_ScrollEventBuffer.push(evt);
		}
		else
		{
			LOGINFO("Cleaned up scroll event.");

		}
	}

	while (tempKeyboardEvents.empty() == false)
	{
		KeyboardEvent evt = tempKeyboardEvents.front();
		tempKeyboardEvents.pop();
		if (evt.GetWindowId() != windowId)
		{
			m_Keyboard.m_EventBuffer.push(evt);
		}
		else
		{
			LOGINFO("Cleaned up keyboard event.");
		}
	}

}
