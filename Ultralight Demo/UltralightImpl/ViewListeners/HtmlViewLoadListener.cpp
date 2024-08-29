#include "PCH.h"
#include "HtmlViewLoadListener.h"
#include "../EZJSParm.h"
#include "../../Engine.h"

JSClassRef HtmlViewLoadListener::m_ClassRefFncCallback = nullptr;

HtmlViewLoadListener::HtmlViewLoadListener()
{
}

void HtmlViewLoadListener::OnFinishLoading(ul::View* caller, uint64_t frame_id, bool is_main_frame, const ul::String& url)
{
    //auto jscontext = caller->LockJSContext();
    //JSContextRef ctxRef = jscontext->ctx();
}

void HtmlViewLoadListener::OnDOMReady(ultralight::View* caller,
                                      uint64_t frame_id,
                                      bool is_main_frame,
                                      const ul::String& url)
{
    //auto jscontext = caller->LockJSContext();
    //JSContextRef ctxRef = jscontext->ctx();
}

JSValueRef CallEvent(JSContextRef ctx, JSObjectRef function,
                     JSObjectRef thisObject, size_t argumentCount,
                     const JSValueRef arguments[], JSValueRef* exception)
{

    if (argumentCount > 0)
    {
        /*string threadInfo = GetThreadText();
        LOGINFO(threadInfo.c_str());*/
        int32_t* pViewId = (int32_t*)JSObjectGetPrivate(function);

        UltralightManager* pUltralightManager = UltralightManager::GetInstance();
        if (pUltralightManager->IsViewFlaggedForDeletion(*pViewId))
        {
            return JSValueMakeNull(ctx);
        }

        if (pViewId == nullptr)
        {
            JSRetainPtr<JSStringRef> msg = adopt(JSStringCreateWithUTF8CString("Could not resolve view id from CallEvent."));
            *exception = JSValueMakeString(ctx, msg.get());
            return JSValueMakeNull(ctx);
        }

        int32_t viewId = *pViewId;

        if (argumentCount == 0)
        {
            JSRetainPtr<JSStringRef> msg = adopt(JSStringCreateWithUTF8CString("First argument should always be the event name (string). Function was called with no arguments."));
            *exception = JSValueMakeString(ctx, msg.get());
            return JSValueMakeNull(ctx);
        }

        //first validate arguments are valid
        if (JSValueGetType(ctx, arguments[0]) != JSType::kJSTypeString) //Event name (first arg) should always be string
        {
            JSRetainPtr<JSStringRef> msg = adopt(JSStringCreateWithUTF8CString("First argument should always be the event name (string)."));
            *exception = JSValueMakeString(ctx, msg.get());
            return JSValueMakeNull(ctx);
        }

        EZJSParm eventNameParm;
        std::string outException;
        if (!EZJSParm::CreateFromJSValue(ctx, arguments[0], eventNameParm, outException))
        {
            JSRetainPtr<JSStringRef> msg = adopt(JSStringCreateWithUTF8CString(outException.c_str()));
            *exception = JSValueMakeString(ctx, msg.get());
            return JSValueMakeNull(ctx);
        }

        string eventName = eventNameParm.AsString();

        vector<EZJSParm> parms;
        parms.resize(argumentCount - 1);

        for (int i = 1; i < argumentCount; i++)
        {
            if (!EZJSParm::CreateFromJSValue(ctx, arguments[i], parms[i - 1], outException))
            {
                JSRetainPtr<JSStringRef> msg = adopt(JSStringCreateWithUTF8CString(outException.c_str()));
                *exception = JSValueMakeString(ctx, msg.get());
                return JSValueMakeNull(ctx);
            }
        }

        return Engine::GetInstance()->OnEventCallbackFromUltralight(viewId, eventName, parms).ToJSValueRef(ctx);
    }
    return JSValueMakeNull(ctx);
}

JSValueRef PrintTest(JSContextRef ctx, JSObjectRef function,
                     JSObjectRef thisObject, size_t argumentCount,
                     const JSValueRef arguments[], JSValueRef* exception)
{
    string msg = strfmt("Tick Count [%d]\n", GetTickCount());
    OutputDebugStringA(msg.c_str());
    return JSValueMakeNull(ctx);
}

void HtmlViewLoadListener::OnWindowObjectReady(ultralight::View* caller,
                                               uint64_t frame_id,
                                               bool is_main_frame,
                                               const ul::String& url)
{
    ul::RefPtr<ul::JSContext> context = caller->LockJSContext(); // Create a JavaScript String containing the name of our callback.
    JSContextRef ctx = context->ctx();
    {
        if (m_ClassRefFncCallback == nullptr)
        {
            JSClassDefinition classFncDef;
            memset(&classFncDef, 0, sizeof(classFncDef));
            classFncDef.className = "CallEvent";
            classFncDef.attributes = kJSClassAttributeNone;
            classFncDef.callAsFunction = CallEvent;
            m_ClassRefFncCallback = JSClassCreate(&classFncDef);
        }
        JSObjectRef nativeFnc = JSObjectMake(ctx, m_ClassRefFncCallback, m_Id.get());
        JSStringRef name = JSStringCreateWithUTF8CString("CallEvent");
        JSObjectRef globalObj = JSContextGetGlobalObject(ctx);
        JSObjectSetProperty(ctx, globalObj, name, nativeFnc, 0, 0);
        JSStringRelease(name);
        {
            JSStringRef name = JSStringCreateWithUTF8CString("PrintTest");
            JSObjectRef globalObj = JSContextGetGlobalObject(ctx);
            JSObjectRef fnc = JSObjectMakeFunctionWithCallback(ctx, name, PrintTest);
            JSObjectSetProperty(ctx, globalObj, name, fnc, 0, 0);
            JSStringRelease(name);
        }
    }
}

void HtmlViewLoadListener::AssignViewId(shared_ptr<int32_t> id)
{
    m_Id = id;
}
