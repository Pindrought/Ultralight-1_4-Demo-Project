#pragma once
#include <PCH.h>

enum WindowStyle : DWORD
{
	None = 0,
	Resizable = (1 << 0),
	ExitButton = (1 << 1),
	MinimizeAvailable = (1 << 2),
	MaximizeAvailable = (1 << 3),
	NoBorder = (1 << 4),
	TransparencyAllowed = (1 << 5),
	Topmost = (1 << 6)
};

inline WindowStyle operator|(WindowStyle a, WindowStyle b)
{
	return static_cast<WindowStyle>(static_cast<int>(a) | static_cast<int>(b));
}