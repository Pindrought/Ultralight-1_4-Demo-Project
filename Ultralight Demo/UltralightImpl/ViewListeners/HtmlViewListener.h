#pragma once
#include <PCH.h>

class HtmlViewListener : public ul::ViewListener
{
public:
	HtmlViewListener();
	void OnChangeTitle(ul::View* caller, const ul::String& title) override;
	void OnChangeURL(ul::View* caller, const ul::String& url) override;
	void OnChangeTooltip(ul::View* caller, const ul::String& tooltip) override;
	void OnChangeCursor(ul::View* caller, ul::Cursor cursor) override;
	void OnAddConsoleMessage(ultralight::View* caller, const ultralight::ConsoleMessage& message) override;
	ul::RefPtr<ul::View> OnCreateInspectorView(ul::View* caller, bool is_local,
											   const ul::String& inspected_url);
private:
};