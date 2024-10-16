#pragma once
#include <PCH.h>
#include "../Graphics/Texture.h"
#include "../Graphics/Misc/PixelColor.h"
#include "ViewListeners/HtmlViewListener.h"
#include "ViewListeners/HtmlViewLoadListener.h"
#include "EZJSParm.h"
#include "../Window/InputController/MousePoint.h"

struct UltralightViewCreationParameters
{
	string Name = ""; //For debugging purposes
	uint32_t Width = 400;
	uint32_t Height = 400;
	uint32_t SampleCount = 1;
	DirectX::XMFLOAT3 Position = { 0,0,0 };
	bool ForceMatchWindowDimensions = false;
	bool IsAccelerated = false;
	bool IsTransparent = false;
	WeakWrapper<UltralightView> InspectionTarget; //This is only for when creating an inspector view. For normal views, this should always be null.
};

class UltralightView
{
	friend class UltralightManager;
public:
	ul::View* GetView();
	ul::RefPtr<ul::View> GetViewRefPtr();
	void LoadHTML(std::string html);
	void LoadURL(std::string url);
	int32_t GetId();
	bool UpdateStorageTexture();
	ID3D11ShaderResourceView* GetTextureSRV();
	WeakWrapper<UltralightView> GetInspectorView();
	uint32_t GetWidth() const;
	uint32_t GetHeight() const;
	uint32_t GetSampleCount() const;
	PixelColor GetPixelColor(int x, int y);
	int32_t GetWindowId();
	bool IsMouseEventInsideThisView(MouseEvent* mouseEvent, MousePoint& outPixelCoords);
	void FireKeyboardEvent(ul::KeyEvent keyboardEvent);
	void FireMouseEvent(ul::MouseEvent mouseEvent);
	void FireScrollEvent(ul::ScrollEvent scrollEvent);
	bool IsAccelerated() const;
	bool IsInputEnabled() const;
	bool IsInspectorView() const;
	bool IsVisible() const;
	bool HasInspectorView() const;
	void SetInputEnabled(bool enabled);
	void SetVisibility(bool isVisible);
	bool ShouldMatchWindowDimensions();
	DirectX::XMFLOAT3 GetPosition();
	bool Resize(uint32_t width, uint32_t height);
	void SetPosition(DirectX::XMFLOAT3 pos);
	bool CallJSFnc(std::string inFunctionName, std::initializer_list<EZJSParm> inParmList, EZJSParm& outReturnValue, std::string& outException);
	bool CallJSFnc(std::string inFunctionName, vector<EZJSParm>& inParmList, EZJSParm& outReturnValue, std::string& outException);
	~UltralightView();
private:
	UltralightView() {} //This should only be instantiated by the UltralightManager but since i'm using shared_ptr was running into issues. 
	void SetToWindow(int32_t windowId); //To remove confusion, this is all managed by the UltralightManager AddViewToWindow/RemoveViewFromWindow functions
	bool Initialize(UltralightViewCreationParameters parms); //This should only be initialized by the UltralightManager
	ul::RefPtr<ul::View> m_NativeView = nullptr;
	bool m_IsAccelerated = true;
	bool m_IsTransparent = false;
	string m_Name = ""; //For debugging purposes
	int32_t m_Id = -1;
	int32_t m_WindowId = -1; //If assigned to a window, this will contain the id of that window.
	DirectX::XMFLOAT3 m_Position = { 0,0, 0 };
	uint32_t m_Width = 0;
	uint32_t m_Height = 0;
	uint32_t m_SampleCount = 1;
	bool m_InputEnabled = true;
	bool m_IsVisible = true;
	bool m_DestructionInitiated = false;
	WeakWrapper<UltralightView> m_InspectionTarget; //Only time this is not null is when this is an inspector view.
	WeakWrapper<UltralightView> m_InspectorView; //If this view is being inspected by another view, this will reference the inspector view.
	bool m_ForceMatchWindowDimensions = false;
	std::shared_ptr<Texture> m_StorageTexture = nullptr; //This is the storage texture used only by the CPU renderer
	ComPtr<ID3D11Texture2D> m_TempTexture = nullptr; //This is a temporary texture used to store last img during a resize
	ComPtr<ID3D11ShaderResourceView> m_TempSRV = nullptr; //This is a temporary texture SRV used to store last img during a resize
	std::unique_ptr<HtmlViewListener> m_ViewListener;
	std::unique_ptr<HtmlViewLoadListener> m_LoadListener;
};