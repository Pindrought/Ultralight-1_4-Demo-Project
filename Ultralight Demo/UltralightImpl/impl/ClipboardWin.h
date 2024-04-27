//this is straight copy&paste
#pragma once
#include <PCH.h>

class ClipboardWin : public ultralight::Clipboard
{

public:
	ClipboardWin() {}

	virtual void Clear() override;

	virtual ultralight::String ReadPlainText() override;

	virtual void WritePlainText(const ultralight::String& text) override;
};