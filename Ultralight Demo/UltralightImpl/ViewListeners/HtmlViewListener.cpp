#include <PCH.h>
#include "HtmlViewListener.h"
#include "../../Misc/CursorManager.h"
#include "../UltralightManager.h"

HtmlViewListener::HtmlViewListener()
{
}

void HtmlViewListener::OnChangeTitle(ul::View* caller, const ul::String& title)
{
}

void HtmlViewListener::OnChangeURL(ul::View* caller, const ul::String& url)
{
}

void HtmlViewListener::OnChangeTooltip(ul::View* caller, const ul::String& tooltip)
{
}

void HtmlViewListener::OnChangeCursor(ul::View* caller, ul::Cursor cursor)
{
    switch (cursor)
    {
    case ultralight::kCursor_Hand:
        CursorManager::SetCursor(CursorType::HAND);
        break;
    case ultralight::kCursor_Pointer:
        CursorManager::SetCursor(CursorType::ARROW);
        break;
    case ultralight::kCursor_IBeam:
        CursorManager::SetCursor(CursorType::IBEAM);
        break;
    case ultralight::kCursor_MiddlePanning:
    case ultralight::kCursor_EastPanning:
    case ultralight::kCursor_NorthPanning:
    case ultralight::kCursor_NorthEastPanning:
    case ultralight::kCursor_NorthWestPanning:
    case ultralight::kCursor_SouthPanning:
    case ultralight::kCursor_SouthEastPanning:
    case ultralight::kCursor_SouthWestPanning:
    case ultralight::kCursor_WestPanning:
        CursorManager::SetCursor(CursorType::MIDDLEPANNING);
        break;
    default:
        break;
    }
}

void HtmlViewListener::OnAddConsoleMessage(ul::View* caller,
                                               const ultralight::ConsoleMessage& message)
{
    std::string msg = strfmt("Javascript Error on line [%d] column [%d] in file [%s].\nError: [%s].\n",
                                   message.line_number(),
                                   message.column_number(),
                                   message.source_id().utf8().data(),
                                   message.message().utf8().data());
    LOGINFO(msg.c_str());
}

ul::RefPtr<ul::View> HtmlViewListener::OnCreateInspectorView(ul::View* caller, bool is_local, const ul::String& inspected_url)
{
    UltralightManager* pManager = UltralightManager::GetInstance();
    shared_ptr<UltralightView> targetView = pManager->GetUltralightViewFromNativeViewPtr(caller);
    return targetView->GetInspectorView()->GetViewRefPtr();
}
