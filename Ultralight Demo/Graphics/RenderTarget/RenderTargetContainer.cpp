#include <PCH.h>
#include "RenderTargetContainer.h"
#include "../../Engine.h"
#include "../../Window/Window.h"

bool RenderTargetContainer::InitializeWindowRenderTarget(Window* pWindow,
                                                         UINT sampleCount)
{
    assert(pWindow != nullptr);

    m_IsWindowRenderTarget = true;
    m_WindowPtr = pWindow;
    UpdateViewportDimensions(pWindow->GetWidth(), pWindow->GetHeight());

    m_RenderTarget = std::shared_ptr<RenderTarget>(new RenderTarget());
    if (!m_RenderTarget->InitializeWindowRenderTarget(m_Width,
                                                      m_Height,
                                                      pWindow,
                                                      nullptr))
    {
        return false;
    }

    if (sampleCount > 1)
    {
        m_MSAARenderTarget = std::shared_ptr<RenderTarget>(new RenderTarget());
        if (!m_MSAARenderTarget->Initialize(m_Width,
                                            m_Height,
                                            nullptr,
                                            nullptr,
                                            sampleCount))
        {
            return false;
        }
    }

    return true;
}

bool RenderTargetContainer::InitializeRenderTarget(UINT width,
                                                   UINT height,
                                                   std::shared_ptr<Texture> renderTargetTexture,
                                                   std::shared_ptr<Texture> depthBufferTexture,
                                                   UINT sampleCount)
{
    m_IsWindowRenderTarget = false;

    UpdateViewportDimensions(width, height);

    m_RenderTarget = std::shared_ptr<RenderTarget>(new RenderTarget());
    if (!m_RenderTarget->Initialize(m_Width,
                                    m_Height,
                                    renderTargetTexture,
                                    depthBufferTexture,
                                    1))
    {
        return false;
    }


    if (sampleCount > 1)
    {
        m_MSAARenderTarget = std::shared_ptr<RenderTarget>(new RenderTarget());
        if (!m_MSAARenderTarget->Initialize(m_Width,
                                            m_Height,
                                            nullptr,
                                            nullptr,
                                            sampleCount))
        {
            return false;
        }
    }
}

ID3D11RenderTargetView* RenderTargetContainer::GetRenderTargetView()
{
    if (m_MSAARenderTarget != nullptr)
    {
        return m_MSAARenderTarget->GetRTV();
    }
    if (m_RenderTarget != nullptr)
    {
        return m_RenderTarget->GetRTV();
    }
    return nullptr;
}


ID3D11RenderTargetView* const* RenderTargetContainer::GetRenderTargetViewAddressOf()
{
    if (m_MSAARenderTarget != nullptr)
    {
        return m_MSAARenderTarget->GetRTVAddressOf();
    }
    if (m_RenderTarget != nullptr)
    {
        return m_RenderTarget->GetRTVAddressOf();
    }
    return nullptr;
}

ID3D11Resource* RenderTargetContainer::GetTextureResource()
{
    if (m_MSAARenderTarget != nullptr)
    {
        return m_MSAARenderTarget->GetRTVTextureResourcePtr();
    }
    if (m_RenderTarget != nullptr)
    {
        return m_RenderTarget->GetRTVTextureResourcePtr();
    }
    return nullptr;
}

shared_ptr<Texture> RenderTargetContainer::GetTextureSharedPtr()
{
    if (m_MSAARenderTarget != nullptr)
    {
        return m_MSAARenderTarget->GetSharedTexturePtr();
    }
    if (m_RenderTarget != nullptr)
    {
        return m_RenderTarget->GetSharedTexturePtr();
    }
    return nullptr;
}

ID3D11DepthStencilView* RenderTargetContainer::GetDepthStencilView()
{
    if (m_MSAARenderTarget != nullptr)
    {
        return m_MSAARenderTarget->GetDepthStencilView();
    }

    if (m_RenderTarget != nullptr)
    {
        return m_RenderTarget->GetDepthStencilView();
    }
    return nullptr;
}


bool RenderTargetContainer::IsActive()
{
    return m_IsActive;
}

void RenderTargetContainer::SetActive(bool activeStatus)
{
    m_IsActive = activeStatus;
}

bool RenderTargetContainer::Resize(UINT width, UINT height)
{
    UpdateViewportDimensions(width, height);

    if (m_RenderTarget == nullptr)
    {
        ErrorHandler::LogCriticalError("Resize attempted on render target container that has not been initialized.");
        return false;
    }

    if (!m_RenderTarget->Resize(width, height))
        return false;

    if (m_MSAARenderTarget != nullptr)
    {
        if (!m_MSAARenderTarget->Resize(width, height))
            return false;
    }

    return true;
}

D3D11_VIEWPORT& RenderTargetContainer::GetViewport()
{
    return m_Viewport;
}


float* RenderTargetContainer::GetBackgroundColor()
{
    return m_BGColor;
}

void RenderTargetContainer::ResolveIfNecessary()
{
    if (m_MSAARenderTarget == nullptr)
        return;

    D3DClass* pD3D = D3DClass::GetInstance();
    ID3D11DeviceContext* pContext = pD3D->m_Context.Get();

    pContext->ResolveSubresource(m_RenderTarget->GetRTVTextureResourcePtr(),
                                 0,
                                 m_MSAARenderTarget->GetRTVTextureResourcePtr(),
                                 0,
                                 m_TextureFormat);
}

void RenderTargetContainer::ResetRenderTargetsForResize()
{
    if (m_RenderTarget)
    {
        m_RenderTarget->m_RenderTargetView.Reset();
        if (m_RenderTarget->m_RenderTargetTexture == nullptr &&
            m_RenderTarget->m_TexturePtr != nullptr)
        {
            m_RenderTarget->m_TexturePtr->AddRef();
            auto refcount = m_RenderTarget->m_TexturePtr->Release();
            m_RenderTarget->m_TexturePtr.Reset();
        }
    }
    if (m_MSAARenderTarget)
    {
        m_MSAARenderTarget->m_RenderTargetView.Reset();
    }
}

void RenderTargetContainer::UpdateViewportDimensions(UINT width, UINT height)
{
    m_Width = width;
    m_Height = height;
    m_Viewport.Width = m_Width;
    m_Viewport.Height = m_Height;
    m_Viewport.TopLeftX = 0;
    m_Viewport.TopLeftY = 0;
    m_Viewport.MinDepth = 0;
    m_Viewport.MaxDepth = 1.0f;
}

UINT RenderTargetContainer::GetWidth()
{
    return m_Width;
}

UINT RenderTargetContainer::GetHeight()
{
    return m_Height;
}

bool RenderTargetContainer::IsWindowRenderTarget()
{
    return m_IsWindowRenderTarget;
}

Window* RenderTargetContainer::GetWindowPtr()
{
    return m_WindowPtr;
}

bool RenderTargetContainer::IsReadyForRender()
{
    if (m_UpdateInterval <= m_UpdateIntervalProgress)
    {
        return true;
    }
    return false;
}

void RenderTargetContainer::AdvanceUpdateInterval(float ms)
{
    if (m_UpdateInterval == 0)
    {
        m_UpdateIntervalProgress = 0;
    }
    else
    {
        m_UpdateIntervalProgress += ms;
    }
}

void RenderTargetContainer::ResetReadyForRender()
{
    m_UpdateIntervalProgress = 0;
}

void RenderTargetContainer::SetUpdateInterval(float ms)
{
    m_UpdateInterval = ms;
    m_UpdateIntervalProgress = ms; //Do this to force an update automatically when we update the interval
}

RenderTargetContainer::~RenderTargetContainer()
{
}

void RenderTargetContainer::SetBackgroundColor(float r, float g, float b, float a)
{
    m_BGColor[0] = r;
    m_BGColor[1] = g;
    m_BGColor[2] = b;
    m_BGColor[3] = a;
}