#include <PCH.h>
#include "PixelColor.h"

PixelColor::PixelColor()
	:color(0)
{}

PixelColor::PixelColor(unsigned int val_)
	: color(val_)
{}

PixelColor::PixelColor(BYTE r_, BYTE g_, BYTE b_)
	: PixelColor(r_, g_, b_, 255)
{
}

PixelColor::PixelColor(BYTE r_, BYTE g_, BYTE b_, BYTE a_)
{
	rgba[0] = r_;
	rgba[1] = g_;
	rgba[2] = b_;
	rgba[3] = a_;
}

PixelColor::PixelColor(const PixelColor& src_)
	:color(src_.color)
{}

PixelColor& PixelColor::operator=(const PixelColor& src_)
{
	color = src_.color;
	return *this;
}

bool PixelColor::operator==(const PixelColor& rhs_) const
{
	return (color == rhs_.color);
}

bool PixelColor::operator!=(const PixelColor& rhs_) const
{
	return !(*this == rhs_);
}

constexpr BYTE PixelColor::GetR() const
{
	return this->rgba[0];
}
void PixelColor::SetR(BYTE r_)
{
	rgba[0] = r_;
}

constexpr BYTE PixelColor::GetG() const
{
	return this->rgba[1];
}
void PixelColor::SetG(BYTE g_)
{
	rgba[1] = g_;
}

constexpr BYTE PixelColor::GetB() const
{
	return this->rgba[2];
}
void PixelColor::SetB(BYTE b_)
{
	rgba[2] = b_;
}

constexpr BYTE PixelColor::GetA() const
{
	return rgba[3];
}

void PixelColor::SetA(BYTE a_)
{
	rgba[3] = a_;
}