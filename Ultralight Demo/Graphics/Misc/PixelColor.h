#pragma once
#include <PCH.h>

class PixelColor
{
public:
	PixelColor();
	PixelColor(unsigned int val);
	PixelColor(BYTE r, BYTE g, BYTE b);
	PixelColor(BYTE r, BYTE g, BYTE b, BYTE a);
	PixelColor(const PixelColor& src);

	PixelColor& operator=(const PixelColor& src);
	bool operator==(const PixelColor& rhs) const;
	bool operator!=(const PixelColor& rhs) const;

	constexpr BYTE GetR() const;
	void SetR(BYTE r);

	constexpr BYTE GetG() const;
	void SetG(BYTE g);

	constexpr BYTE GetB() const;
	void SetB(BYTE b);

	constexpr BYTE GetA() const;
	void SetA(BYTE a);
	union
	{
		BYTE rgba[4];
		unsigned int color;
	};
private:

};