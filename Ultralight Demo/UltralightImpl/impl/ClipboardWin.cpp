//this is all copy and paste from ultralight examples
#include <PCH.h>
#include "ClipboardWin.h"

void ClipboardWin::Clear()
{
    EmptyClipboard();
}

ultralight::String ClipboardWin::ReadPlainText()
{
    if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
        return ultralight::String();

    if (!OpenClipboard(0))
        return ultralight::String();

    ultralight::String result;

    HGLOBAL globalMem = GetClipboardData(CF_UNICODETEXT);
    if (globalMem)
    {
        LPWSTR strMem = (LPWSTR)GlobalLock(globalMem);
        if (strMem)
        {
            result = ultralight::String(strMem, lstrlenW(strMem));
            GlobalUnlock(globalMem);
        }
    }

    CloseClipboard();

    return result;
}

void ClipboardWin::WritePlainText(const ultralight::String& text)
{
    if (text.empty())
        return;

    if (!OpenClipboard(0))
        return;

    EmptyClipboard();

    HGLOBAL globalMem = GlobalAlloc(GMEM_MOVEABLE, (text.utf16().size() + 1) * sizeof(ultralight::Char16));
    LPWSTR strMem = (LPWSTR)GlobalLock(globalMem);
    memcpy(strMem, text.utf16().data(), text.utf16().size() * sizeof(ultralight::Char16));
    strMem[text.utf16().size()] = 0; // null character
    GlobalUnlock(globalMem);

    SetClipboardData(CF_UNICODETEXT, globalMem);

    CloseClipboard();
}