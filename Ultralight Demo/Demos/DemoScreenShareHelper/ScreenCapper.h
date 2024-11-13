#pragma once
#include <PCH.h>

class ScreenCapper
{
public:
	bool Initialize();
	bool AssignRegion(int x, int y, int width, int height);
	bool CaptureRegion();
	~ScreenCapper();
	RGBQUAD* m_Pixels = nullptr;

private:
	HDC m_DesktopDC = NULL;
	HDC m_CaptureDC = NULL;
	HBITMAP m_HBitmap = NULL;
	int m_X = 0;
	int m_Y = 0;
	int m_Width = 0;
	int m_Height = 0;
};