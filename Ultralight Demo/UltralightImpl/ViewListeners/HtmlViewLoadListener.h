#include <PCH.h>
#pragma once

class HtmlViewLoadListener : public ultralight::LoadListener
{
public:
	HtmlViewLoadListener();

	virtual ~HtmlViewLoadListener() {}
	void OnFinishLoading(ul::View* caller, uint64_t frame_id, bool is_main_frame, const ul::String& url) override;
	virtual void OnDOMReady(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ul::String& url) override;
	virtual void OnWindowObjectReady(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ul::String& url) override;
	void AssignViewId(int32_t id);
private:
	int m_Id = -1;
	static JSClassRef m_ClassRefFncCallback;
};