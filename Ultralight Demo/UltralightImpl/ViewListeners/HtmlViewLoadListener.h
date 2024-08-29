#include <PCH.h>
#pragma once

class HtmlViewLoadListener : public ultralight::LoadListener
{
public:
	HtmlViewLoadListener();

	virtual ~HtmlViewLoadListener() { LOGINFO("~HtmlViewLoadListener()"); }
	void OnFinishLoading(ul::View* caller, uint64_t frame_id, bool is_main_frame, const ul::String& url) override;
	virtual void OnDOMReady(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ul::String& url) override;
	virtual void OnWindowObjectReady(ultralight::View* caller, uint64_t frame_id, bool is_main_frame, const ul::String& url) override;
	void AssignViewId(shared_ptr<int32_t> id);
private:
	shared_ptr<int32_t> m_Id = nullptr;
	static JSClassRef m_ClassRefFncCallback;
};