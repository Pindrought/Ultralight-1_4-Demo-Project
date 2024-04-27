#pragma once
#include <PCH.h>

class MimeTypeHelper
{
public:
	static const char* FileExtensionToMimeTypeA(const char* ext);
	static const wchar_t* FileExtensionToMimeType(const wchar_t* ext);
};