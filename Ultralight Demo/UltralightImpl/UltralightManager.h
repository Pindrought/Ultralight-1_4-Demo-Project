#pragma once
#include <PCH.h>
#include "impl/ClipboardWin.h"
#include "impl/FileSystemWin.h"
#include "impl/FontLoaderWin.h"
#include "impl/LoggerDefault.h"
#include "GPUimpl/GPUDriverD3D11.h"
#include "UltralightView.h"

#include "../Window/Window.h"

class UltralightManager
{
public:
	bool Initialize();
	void Shutdown();
	void UpdateViews();
	ul::Renderer* GetRendererPtr();
	void RemoveViewFromWindow(int32_t viewId, int32_t windowId);
	void SetViewToWindow(int32_t viewId, int32_t windowId);
	void RegisterWindow(std::shared_ptr<Window> pWindow);
	void RemoveWindowId(int32_t windowId);
	//GPUDriverD3D11* GetGPUDriverPtr();
	std::shared_ptr<UltralightView> CreateUltralightView(UltralightViewCreationParameters parms);
	void DestroyView(shared_ptr<UltralightView> pView);
	static UltralightManager* GetInstance();
	std::vector<std::shared_ptr<UltralightView>> GetViewsForWindow(int32_t windowId);
	std::shared_ptr<UltralightView> GetViewFromId(int32_t viewId);
	GPUDriverD3D11* GetGPUDriver();
	unordered_map<int32_t, shared_ptr<UltralightView>> GetViews();
	void RefreshViewDisplaysForAnimations();
	//A return value of true = the event was processed by an ultralight view.
	bool FireKeyboardEvent(KeyboardEvent* keyboardEvent);
	bool FireMouseEvent(MouseEvent* mouseEvent);
	bool FireScrollEvent(ScrollEvent* scrollEvent);
	shared_ptr<UltralightView> GetUltralightViewFromNativeViewPtr(ul::View* pNativeView);

	~UltralightManager();
private:
	static UltralightManager* s_Instance;
	ul::RefPtr<ul::Renderer> m_UltralightRenderer;
	unique_ptr<LoggerDefault> m_Logger;
	unique_ptr<FontLoaderWin> m_FontLoader;
	unique_ptr<FileSystemWin> m_FileSystem;
	unique_ptr<ClipboardWin> m_Clipboard;
	unique_ptr<GPUDriverD3D11> m_GPUDriver;

	unordered_map<int32_t, shared_ptr<Window>> m_WindowIdToWindowPtrMap;
	unordered_map<int32_t, set<int32_t>> m_WindowIdToViewIdMap;
	unordered_map<int32_t, shared_ptr<UltralightView>> m_ViewsMap;
};