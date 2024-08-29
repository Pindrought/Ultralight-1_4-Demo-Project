#pragma once
#include <PCH.h>
#include "impl/ClipboardWin.h"
#include "impl/FileSystemWin.h"
#include "impl/FontLoaderWin.h"
#include "impl/LoggerDefault.h"
#include "GPUimpl/IGPUDriverD3D11.h"
#include "UltralightView.h"

#include "../Window/Window.h"

struct UltralightOverrides
{
	shared_ptr<IGPUDriverD3D11> GPUDriver = nullptr;
	shared_ptr<ul::FileSystem> FileSystem = nullptr;
};

class UltralightManager //Note that this is a singleton and the ultralight renderer can only be created ONCE in a process. Due to this, we store a shared ptr to the single instance that gets generated.
{
public:
	bool Initialize(UltralightOverrides* ultralightOverrides = nullptr);
	void Shutdown();
	void UpdateViews();
	ul::Renderer* GetRendererPtr();
	void RemoveViewFromWindow(int32_t viewId, int32_t windowId);
	void SetViewToWindow(int32_t viewId, int32_t windowId);
	void RegisterWindow(WeakWrapper<Window> pWindow);
	void RemoveWindowId(int32_t windowId);
	WeakWrapper<UltralightView> CreateUltralightView(UltralightViewCreationParameters parms);
	void DestroyView(WeakWrapper<UltralightView> pView);
	void DestroyAllViews();
	static UltralightManager* GetInstance();
	static shared_ptr<UltralightManager> GetInstanceShared();

	vector<WeakWrapper<UltralightView>> GetViewsForWindow(int32_t windowId);
	WeakWrapper<UltralightView> GetViewFromId(int32_t viewId);
	IGPUDriverD3D11* GetGPUDriver();
	unordered_map<int32_t, WeakWrapper<UltralightView>> GetViews();
	unordered_map<int32_t, WeakWrapper<UltralightView>> GetAcceleratedViews();

	void RefreshViewDisplaysForAnimations();
	//A return value of true = the event was processed by an ultralight view.
	bool FireKeyboardEvent(KeyboardEvent* keyboardEvent);
	bool FireMouseEvent(MouseEvent* mouseEvent);
	bool FireScrollEvent(ScrollEvent* scrollEvent);
	WeakWrapper<UltralightView> GetUltralightViewFromNativeViewPtr(ul::View* pNativeView);

	~UltralightManager();
private:
	UltralightManager();
	static shared_ptr<UltralightManager> s_Instance;
	ul::RefPtr<ul::Renderer> m_UltralightRenderer;
	unique_ptr<LoggerDefault> m_Logger;
	unique_ptr<FontLoaderWin> m_FontLoader;
	shared_ptr<ul::FileSystem> m_FileSystem;
	unique_ptr<ClipboardWin> m_Clipboard;
	shared_ptr<IGPUDriverD3D11> m_GPUDriver;

	unordered_map<int32_t, WeakWrapper<Window>> m_WindowIdToWindowPtrMap;
	unordered_map<int32_t, set<int32_t>> m_WindowIdToViewIdMap;

	unordered_map<int32_t, WeakWrapper<UltralightView>> m_WeakViewsMap;
	unordered_map<int32_t, shared_ptr<UltralightView>> m_OwnedViewsMap;
	unordered_map<int32_t, WeakWrapper<UltralightView>> m_WeakAcceleratedViewsMap;
};