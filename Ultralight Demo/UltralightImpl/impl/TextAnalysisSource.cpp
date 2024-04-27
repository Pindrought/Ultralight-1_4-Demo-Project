#include <PCH.h>
#include "TextAnalysisSource.h"

TextAnalysisSource::TextAnalysisSource(const wchar_t* text,
                                       UINT32 textLength,
                                       const wchar_t* localeName,
                                       DWRITE_READING_DIRECTION readingDirection)
    : m_Text(text)
    , m_TextLength(textLength)
    , m_LocaleName(localeName)
    , m_ReadingDirection(readingDirection)
    , m_RefValue(0)
{
}

TextAnalysisSource::~TextAnalysisSource()
{
}

////////////////////////////////////////////////////////////////////////////////
// IDWriteTextAnalysisSource source implementation
IFACEMETHODIMP
TextAnalysisSource::GetTextAtPosition(UINT32 textPosition,
                                      OUT WCHAR const** textString,
                                      OUT UINT32* textLength)
{
    if (textPosition >= m_TextLength) {
        // No text at this position, valid query though.
        *textString = NULL;
        *textLength = 0;
    }
    else {
        *textString = m_Text + textPosition;
        *textLength = m_TextLength - textPosition;
    }
    return S_OK;
}

IFACEMETHODIMP
TextAnalysisSource::GetTextBeforePosition(UINT32 textPosition,
                                          OUT WCHAR const** textString,
                                          OUT UINT32* textLength)
{
    if (textPosition == 0 || textPosition > m_TextLength) {
        // Either there is no text before here (== 0), or this
        // is an invalid position. The query is considered valid thouh.
        *textString = NULL;
        *textLength = 0;
    }
    else {
        *textString = m_Text;
        *textLength = textPosition;
    }
    return S_OK;
}

DWRITE_READING_DIRECTION STDMETHODCALLTYPE
TextAnalysisSource::GetParagraphReadingDirection()
{
    // We support only a single reading direction.
    return m_ReadingDirection;
}

IFACEMETHODIMP
TextAnalysisSource::GetLocaleName(UINT32 textPosition,
                                  OUT UINT32* textLength,
                                  OUT WCHAR const** localeName)
{
    // Single locale name is used, valid until the end of the string.
    *localeName = m_LocaleName;
    *textLength = m_TextLength - textPosition;
    return S_OK;
}

IFACEMETHODIMP
TextAnalysisSource::GetNumberSubstitution(UINT32 textPosition,
                                          OUT UINT32* textLength,
                                          OUT IDWriteNumberSubstitution** numberSubstitution)
{
    // We do not support number substitution.
    *numberSubstitution = NULL;
    *textLength = m_TextLength - textPosition;
    return S_OK;
}