#pragma once
#include <PCH.h>

class TextAnalysisSource
    : public IDWriteTextAnalysisSource
{
public:
    // IUnknown interface
    IFACEMETHOD(QueryInterface)(IID const& iid, OUT void** ppObject)
    {
        if (iid == __uuidof(IDWriteTextAnalysisSource)) {
            *ppObject = static_cast<IDWriteTextAnalysisSource*>(this);
            return S_OK;
        }
        else if (iid == __uuidof(IUnknown)) {
            *ppObject =
                static_cast<IUnknown*>(static_cast<IDWriteTextAnalysisSource*>(this));
            return S_OK;
        }
        else {
            return E_NOINTERFACE;
        }
    }

    IFACEMETHOD_(ULONG, AddRef)()
    {
        return InterlockedIncrement(&m_RefValue);
    }

    IFACEMETHOD_(ULONG, Release)()
    {
        ULONG newCount = InterlockedDecrement(&m_RefValue);
        if (newCount == 0)
            delete this;

        return newCount;
    }

public:
    TextAnalysisSource(const wchar_t* text,
                       UINT32 textLength,
                       const wchar_t* localeName,
                       DWRITE_READING_DIRECTION readingDirection);

    ~TextAnalysisSource();

    // IDWriteTextAnalysisSource implementation
    IFACEMETHODIMP GetTextAtPosition(UINT32 textPosition,
                                     OUT WCHAR const** textString,
                                     OUT UINT32* textLength);

    IFACEMETHODIMP GetTextBeforePosition(UINT32 textPosition,
                                         OUT WCHAR const** textString,
                                         OUT UINT32* textLength);

    IFACEMETHODIMP_(DWRITE_READING_DIRECTION)
        GetParagraphReadingDirection() throw();

    IFACEMETHODIMP GetLocaleName(UINT32 textPosition,
                                 OUT UINT32* textLength,
                                 OUT WCHAR const** localeName);

    IFACEMETHODIMP
        GetNumberSubstitution(UINT32 textPosition,
                              OUT UINT32* textLength,
                              OUT IDWriteNumberSubstitution** numberSubstitution);

protected:
    UINT32 m_TextLength;
    const wchar_t* m_Text;
    const wchar_t* m_LocaleName;
    DWRITE_READING_DIRECTION m_ReadingDirection;
    ULONG m_RefValue;

private:
    // No copy construction allowed.
    TextAnalysisSource(const TextAnalysisSource& b) = delete;
    TextAnalysisSource& operator=(TextAnalysisSource const&) = delete;
};