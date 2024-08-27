#include <PCH.h>
#include "UltralightView.h"
#include "UltralightManager.h"
#include "../Misc/IDPoolManager.h"
#include "../Engine.h"

static IDPoolManager<int32_t> s_UltralightViewIDPoolManager;

ul::View* UltralightView::GetView()
{
    assert(m_NativeView.get() != nullptr);
    return m_NativeView.get();
}

ul::RefPtr<ul::View> UltralightView::GetViewRefPtr()
{
	return m_NativeView;
}

void UltralightView::LoadHTML(std::string html)
{
	m_NativeView->LoadHTML(ul::String8(html.c_str()));
}

void UltralightView::LoadURL(std::string url)
{
	m_NativeView->LoadURL(ul::String8(url.c_str()));
}

int32_t UltralightView::GetId()
{
    return m_Id;
}

bool UltralightView::UpdateStorageTexture()
{
	assert(m_NativeView.get() != nullptr);
	if (m_IsAccelerated) //GPU Renderer
	{
		
	}
	else //CPU Renderer
	{
		ul::BitmapSurface* pSurface = (ul::BitmapSurface*)m_NativeView->surface();
		bool dirtyBoundsEmpty = pSurface->dirty_bounds().IsEmpty();
		if (dirtyBoundsEmpty == false)
		{
			D3DClass* pD3D = D3DClass::GetInstance();
			ID3D11DeviceContext* pContext = pD3D->m_Context.Get();

			ID3D11Resource* pTexture = m_StorageTexture->GetTextureResource();

			D3D11_MAPPED_SUBRESOURCE resource;
			HRESULT hr = pContext->Map(pTexture, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &resource);
			ReturnFalseIfFail(hr, "Failed to map UltralightView storage texture.");

			if (resource.RowPitch == pSurface->row_bytes())
			{
				memcpy(resource.pData, pSurface->LockPixels(), pSurface->size());
				pSurface->UnlockPixels();
			}
			else
			{
				auto bitmap = pSurface->bitmap();
				ul::RefPtr<ul::Bitmap> mapped_bitmap = ul::Bitmap::Create(bitmap->width(),
																		  bitmap->height(),
																		  bitmap->format(),
																		  resource.RowPitch,
																		  resource.pData,
																		  resource.RowPitch * bitmap->height(),
																		  false);
				ul::IntRect dest_rect = { 0, 0, (int)bitmap->width(), (int)bitmap->height() };
				mapped_bitmap->DrawBitmap(dest_rect, dest_rect, bitmap, false);
			}

			pContext->Unmap(pTexture, 0);

			pSurface->ClearDirtyBounds();
		}
	}
	return true;
}

ID3D11ShaderResourceView* UltralightView::GetTextureSRV()
{
	if (m_IsAccelerated) //GPU Renderer
	{
		IGPUDriverD3D11* pGPUDriver = UltralightManager::GetInstance()->GetGPUDriver();
		ID3D11ShaderResourceView* pSRV = pGPUDriver->GetShaderResourceView(m_NativeView.get());
		if (pSRV == nullptr)
		{
			return m_TempSRV.Get();
		}
		else
		{
			if (m_TempSRV != nullptr)
			{
				m_TempSRV = nullptr;
				m_TempTexture = nullptr;
			}
		}
		return pGPUDriver->GetShaderResourceView(m_NativeView.get());
	}
	else //CPU Renderer
	{
		return m_StorageTexture->GetTextureResourceView();
	}
}

shared_ptr<UltralightView> UltralightView::GetInspectorView()
{
	return m_InspectorView;
}

uint32_t UltralightView::GetWidth() const
{
	return m_Width;
}

uint32_t UltralightView::GetHeight() const
{
	return m_Height;
}

uint32_t UltralightView::GetSampleCount() const
{
	return m_SampleCount;
}

PixelColor UltralightView::GetPixelColor(int x, int y)
{
	assert(x >= 0 && x < m_Width);
	assert(y >= 0 && y < m_Height);
	
	if (m_IsAccelerated) //GPU Driver
	{
		D3DClass* pD3D = D3DClass::GetInstance();
		//TODO: Maybe move this somewhere else
		static ComPtr<ID3D11Texture2D> s_StagingTextureForPixelColorGrab = nullptr;
		if (s_StagingTextureForPixelColorGrab == nullptr)
		{
			D3D11_TEXTURE2D_DESC stagingTextureDesc = { 0 };

			stagingTextureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			stagingTextureDesc.BindFlags = 0;
			stagingTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_READ;
			stagingTextureDesc.Usage = D3D11_USAGE_STAGING;
			stagingTextureDesc.Width = 1;
			stagingTextureDesc.Height = 1;
			stagingTextureDesc.ArraySize = 1;
			stagingTextureDesc.MipLevels = 1;
			stagingTextureDesc.SampleDesc.Count = 1;

			ID3D11Device* pDevice = pD3D->m_Device.Get();
			HRESULT hr = pDevice->CreateTexture2D(&stagingTextureDesc, nullptr, &s_StagingTextureForPixelColorGrab);
			FatalErrorIfFail(hr, "Failed to generate 1x1 staging texture for Ultralight pixel color validation.");
		}
		
		ID3D11DeviceContext* pContext = pD3D->m_Context.Get();

		IGPUDriverD3D11* pGPUDriver = UltralightManager::GetInstance()->GetGPUDriver();
		ID3D11Texture2D* pViewTexture = pGPUDriver->GetTexture(m_NativeView.get());

		if (pViewTexture == nullptr)
		{
			return PixelColor(0, 0, 0, 0);
		}

		D3D11_BOX box;
		box.left = x;
		box.right = x + 1;
		box.top = y;
		box.bottom = y + 1;
		box.front = 0;
		box.back = 1;
		pContext->CopySubresourceRegion(s_StagingTextureForPixelColorGrab.Get(), 0, 0, 0, 0,
										pViewTexture, 0, &box);

		D3D11_MAPPED_SUBRESOURCE res = { 0 };
		HRESULT hr = pContext->Map(s_StagingTextureForPixelColorGrab.Get(), 0, D3D11_MAP_READ, 0, &res);
		FatalErrorIfFail(hr, "Failed to map staging texture for Ultralight pixel color validation.");

		PixelColor pc;
		memcpy(&pc, res.pData, sizeof(pc));

		pContext->Unmap(s_StagingTextureForPixelColorGrab.Get(), 0);

		return pc;
	}
	else //CPU Driver
	{
		auto pSurface = m_NativeView->surface();
		PixelColor* pPixels = (PixelColor*)pSurface->LockPixels();
		assert(pPixels != nullptr);

		PixelColor pc;
		memcpy(&pc,
			   pPixels + m_Width * y + x,
			   sizeof(pc));
		pSurface->UnlockPixels();
		return pc;
	}
}

int32_t UltralightView::GetWindowId()
{
	return m_WindowId;
}

void UltralightView::FireKeyboardEvent(ul::KeyEvent keyboardEvent)
{
	m_NativeView->FireKeyEvent(keyboardEvent);
}

void UltralightView::FireMouseEvent(ul::MouseEvent mouseEvent)
{
	mouseEvent.x -= m_Position.x;
	mouseEvent.y -= m_Position.y;
	m_NativeView->FireMouseEvent(mouseEvent);
}

void UltralightView::FireScrollEvent(ul::ScrollEvent scrollEvent)
{
	m_NativeView->FireScrollEvent(scrollEvent);
}

bool UltralightView::IsAccelerated() const
{
	return m_IsAccelerated;
}

bool UltralightView::IsInputEnabled() const
{
	return m_InputEnabled;
}

bool UltralightView::IsInspectorView() const
{
	return m_IsInspectorView;
}

bool UltralightView::IsVisible() const
{
	return m_IsVisible;
}

bool UltralightView::HasInspectorView() const
{
	return m_HasInspectorView;
}

void UltralightView::SetInputEnabled(bool enabled)
{
	m_InputEnabled = enabled;
}

void UltralightView::SetVisibility(bool isVisible)
{
	m_IsVisible = isVisible;
}

bool UltralightView::ShouldMatchWindowDimensions()
{
	return m_ForceMatchWindowDimensions;
}

DirectX::XMFLOAT3 UltralightView::GetPosition()
{
	return m_Position;
}

bool UltralightView::Resize(uint32_t width, uint32_t height)
{
	m_Width = width;
	m_Height = height;
	D3DClass* pD3D = D3DClass::GetInstance();
	ID3D11Device* pDevice = pD3D->m_Device.Get();
	ID3D11DeviceContext* pContext = pD3D->m_Context.Get();

	if (m_IsAccelerated)
	{
		IGPUDriverD3D11* pGPUDriver = UltralightManager::GetInstance()->GetGPUDriver();
		assert(pGPUDriver != nullptr);

		auto pTexture = pGPUDriver->GetTexture(m_NativeView.get());
		auto pSRV = pGPUDriver->GetShaderResourceView(m_NativeView.get());

		if (pTexture != nullptr)
		{
			D3D11_TEXTURE2D_DESC texDesc;
			pTexture->GetDesc(&texDesc);

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			pSRV->GetDesc(&srvDesc);

			if (FAILED(pDevice->CreateTexture2D(&texDesc, nullptr, &m_TempTexture)))
			{
				ErrorHandler::LogCriticalError("Failed to create temporary texture for accelerated resized Ultralight View.");
				return false;
			}

			if (FAILED(pDevice->CreateShaderResourceView(m_TempTexture.Get(), &srvDesc, &m_TempSRV)))
			{
				ErrorHandler::LogCriticalError("Failed to create temporary texture shader resource view for accelerated resized Ultralight View.");
				return false;
			}
			pContext->CopyResource(m_TempTexture.Get(), pTexture);
		}
	}
	else
	{
		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		textureDesc.Width = m_Width;
		textureDesc.Height = m_Height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DYNAMIC;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		textureDesc.MiscFlags = 0;

		m_StorageTexture = std::make_shared<Texture>();
		if (!m_StorageTexture->Initialize(textureDesc, false))
		{
			return false;
		}
	}
	m_NativeView->Resize(width, height);
	return true;
}

void UltralightView::SetPosition(DirectX::XMFLOAT3 pos)
{
	m_Position = pos;
}

bool UltralightView::CallJSFnc(std::string inFunctionName, std::initializer_list<EZJSParm> inParmList, EZJSParm& outReturnValue, std::string& outException)
{
	vector<EZJSParm> vecArgs = inParmList;
	return CallJSFnc(inFunctionName, vecArgs, outReturnValue, outException);
}

bool UltralightView::CallJSFnc(std::string inFunctionName, 
							   vector<EZJSParm>& inParmList,
							   EZJSParm& outReturnValue,
							   std::string& outException)
{
	outReturnValue = EZJSParm();

	ul::View* pULView = m_NativeView.get();
	auto context = pULView->LockJSContext();
	JSContextRef ctx = context->ctx();
	JSStringRef fncNameStringRef = JSStringCreateWithUTF8CString(inFunctionName.c_str());

	std::vector<JSStringRef> cleanupStrings;
	cleanupStrings.push_back(fncNameStringRef);

	JSValueRef func = JSEvaluateScript(ctx, fncNameStringRef, 0, 0, 0, 0);
	if (JSValueIsObject(ctx, func))
	{
		JSObjectRef funcObj = JSValueToObject(ctx, func, 0);

		if (funcObj && JSObjectIsFunction(ctx, funcObj))
		{
			vector<JSValueRef> args = BuildJSValueRefParms(ctx, inParmList);

			JSValueRef exception = 0;
			JSValueRef result = JSObjectCallAsFunction(ctx, funcObj, 0, args.size(), args.data(), &exception);
			if (exception)
			{
				JSStringRef msgArgumentJSRef = JSValueToStringCopy(ctx, exception, NULL);
				size_t length = JSStringGetLength(msgArgumentJSRef) + 1;
				std::unique_ptr<char[]> stringBuffer = std::make_unique<char[]>(length);
				JSStringGetUTF8CString(msgArgumentJSRef, stringBuffer.get(), length);
				for (auto& s : cleanupStrings)
				{
					JSStringRelease(s);
				}
				outException = stringBuffer.get();
				return false;
			}

			if (!EZJSParm::CreateFromJSValue(ctx, result, outReturnValue, outException))
			{
				return false;
			}

			return true;
		}
		else
		{
			for (auto& s : cleanupStrings)
			{
				JSStringRelease(s);
			}
			outException = strfmt("JS function [%s] does not exist, but an object exists with this name instead.", inFunctionName.c_str());
			return false;
		}
	}
	else
	{
		for (auto& s : cleanupStrings)
		{
			JSStringRelease(s);
		}
		outException = strfmt("JS function [%s] does not exist.", inFunctionName.c_str());
		return false;
	}


	return false;
}

void UltralightView::SetToWindow(int32_t windowId)
{
	if (windowId == -1) //-1 = clearing view from window
	{
		m_WindowId = windowId;
		return;
	}

	Engine* pEngine = Engine::GetInstance();
	Window* pWindow = pEngine->GetWindowFromId(windowId);
	assert(pWindow != nullptr);
	if (m_ForceMatchWindowDimensions)
	{
		if (pWindow->GetWidth() != m_Width ||
			pWindow->GetHeight() != m_Height)
		{
			Resize(pWindow->GetWidth(), pWindow->GetHeight());
		}
	}
	m_WindowId = windowId;
	
}

UltralightView::~UltralightView()
{
	if (m_Id != -1)
	{
		s_UltralightViewIDPoolManager.StoreId(m_Id);
	}

	if (m_InspectorView != nullptr)
	{
	}
	int refCount = m_NativeView->ref_count();
	if (refCount > 1)
		m_NativeView->Release(); //This is necessary for when an inspector view is holding a ref to this view or else this will never be cleaned up
	//Note that in some cases there is still a crash when it comes to releasing the Ultralight views. Haven't figured out what I am doing to cause that yet.
	
	LOGINFO("~UltralightView() --");
	LOGINFO(m_Name.c_str());
	LOGINFO("\n");
}

bool UltralightView::Initialize(UltralightViewCreationParameters params)
{
	if (m_Id != -1)
	{
		ErrorHandler::LogCriticalError("Attempted to initialize an Ultralight View that has already been initialized.");
		return false;
	}
	m_Name = params.Name;
	m_Width = params.Width;
	m_Height = params.Height;
	m_SampleCount = params.SampleCount;
	m_IsAccelerated = params.IsAccelerated;
	m_Position = params.Position;
	m_IsTransparent = params.IsTransparent;
	m_ForceMatchWindowDimensions = params.ForceMatchWindowDimensions;
	m_Id = s_UltralightViewIDPoolManager.GetNextId();
	m_InspectionTarget = params.InspectionTarget;
	if (m_InspectionTarget != nullptr)
	{
		m_IsInspectorView = true;
		m_InspectionTarget->m_HasInspectorView = true;
	}

	ultralight::ViewConfig config;

	config.display_id = m_Id;
	config.is_transparent = m_IsTransparent;
	config.is_accelerated = m_IsAccelerated;
	UltralightManager* pUltralightManager = UltralightManager::GetInstance();
	assert(pUltralightManager != nullptr);

	ul::Renderer* pUltralightRenderer = pUltralightManager->GetRendererPtr();
	m_NativeView = pUltralightRenderer->CreateView(m_Width, m_Height, config, nullptr);

	if (m_NativeView.get() == nullptr)
	{
		ErrorHandler::LogCriticalError("Ultralight::Renderer failed to create ultralight View.");
		return false;
	}

	if (m_IsAccelerated)
	{
		auto rt = m_NativeView->render_target();
		int textureId = rt.texture_id;
	}

	if (m_IsAccelerated == false) //For CPU renderer, need a storage texture
	{
		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		textureDesc.Width = m_Width;
		textureDesc.Height = m_Height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DYNAMIC;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		textureDesc.MiscFlags = 0;

		m_StorageTexture = std::make_shared<Texture>();
		if (!m_StorageTexture->Initialize(textureDesc, false))
		{
			return false;
		}
	}

	m_ViewListener = std::make_unique<HtmlViewListener>();
	m_NativeView->set_view_listener(m_ViewListener.get());

	m_LoadListener = std::make_unique<HtmlViewLoadListener>();
	m_LoadListener->AssignViewId(m_Id);
	m_NativeView->set_load_listener(m_LoadListener.get());

	return true;
}
