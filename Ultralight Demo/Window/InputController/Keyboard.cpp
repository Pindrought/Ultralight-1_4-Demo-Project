#include <PCH.h>
#include "Keyboard.h"

Keyboard::Keyboard()
{
}

bool Keyboard::KeyIsPressed(const unsigned char keycode)
{
	bool isPressed = m_KeyStates[keycode];
	return isPressed;
}

bool Keyboard::EventBufferIsEmpty()
{
	return m_EventBuffer.empty();
}

KeyboardEvent Keyboard::ReadEvent()
{
	if (m_EventBuffer.empty()) //If no keys to be read?
	{
		return KeyboardEvent(); //return empty keyboard event
	}
	else
	{
		KeyboardEvent e = m_EventBuffer.front(); //Get first Keyboard Event from queue
		m_EventBuffer.pop(); //Remove first item from queue
		return e; //Returns keyboard event
	}
}

void Keyboard::OnWindowsKeyboardMessage(uint32_t windowId,
										UINT uMsg, 
										WPARAM wParam, 
										LPARAM lParam)
{
	KeyboardEvent kbe(windowId, uMsg, wParam, lParam);
	switch (kbe.GetType())
	{
	case KeyboardEvent::Type::KeyDown:
		m_KeyStates[kbe.GetKeyCode()] = true;
		break;
	case KeyboardEvent::Type::KeyUp:
		m_KeyStates[kbe.GetKeyCode()] = false;
		break;
	case KeyboardEvent::Type::Char:
		break;
	}
	m_EventBuffer.push(kbe);

}