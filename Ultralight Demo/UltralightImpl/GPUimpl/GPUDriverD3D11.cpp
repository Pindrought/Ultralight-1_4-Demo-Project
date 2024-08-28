#include <PCH.h>
#include "GPUDriverD3D11.h"
#include "../../Misc/IDPoolManager.h"
#include "../../Engine.h"
#include "../../Graphics/InputLayoutDescriptions.h"
#include "../UltralightManager.h"

static IDPoolManager<uint32_t> s_UltralightGPUDriverGeometryIDManager(1u);
static IDPoolManager<uint32_t> s_UltralightGPUDriverRenderBufferIDManager(1u);
static IDPoolManager<uint32_t> s_UltralightGPUDriverTextureIDManager(1u);

void GPUDriverD3D11::BeginSynchronize() 
{
    //The purpose of this is to allow both MSAA views as well as views with a sample count of 1
    //I have to identify which render target textures are associated with which views and if they should be multisampled or not
    //See GPUDriverD3D11::CreateRenderBuffer()
    UltralightManager* pManager = UltralightManager::GetInstance();
    ID3D11Device* pDevice = D3DClass::GetInstance()->m_Device.Get();

    auto acceleratedViews = pManager->GetAcceleratedViews();
    if (acceleratedViews.size() == 0)
    {
        return;
    }
    //LOGINFO("BeginSynchronize");
    for (auto viewEntry : acceleratedViews)
    {
        auto pView = viewEntry.second;

        if (pView->GetSampleCount() <= 1) //Just doing this for Multisampled views
        {
            continue;
        }

        auto nativeView = pView->GetView();
        auto rt = nativeView->render_target();
        const uint32_t viewId = pView->GetId();

        auto rtToViewIter = m_RenderBufferToViewIdMap.find(rt.render_buffer_id);
        if (rtToViewIter == m_RenderBufferToViewIdMap.end())
        {
            //This technically runs before the renderbuffer gets created, so the render buffer creation method will be able to do a lookup to see if it's MSAA/its sample count
            m_RenderBufferToViewIdMap[rt.render_buffer_id] = viewId;
            m_MSAARenderTargetSampleCountLookup.insert(make_pair(rt.render_buffer_id, pView->GetSampleCount()));
        }
    }
}

void GPUDriverD3D11::EndSynchronize() {}

uint32_t GPUDriverD3D11::NextTextureId()
{
	return s_UltralightGPUDriverTextureIDManager.GetNextId();
}

uint32_t GPUDriverD3D11::NextRenderBufferId()
{
	return s_UltralightGPUDriverRenderBufferIDManager.GetNextId();
}

uint32_t GPUDriverD3D11::NextGeometryId()
{
	return s_UltralightGPUDriverGeometryIDManager.GetNextId();
}

void GPUDriverD3D11::UpdateCommandList(const ul::CommandList& list)
{
	if (list.size)
	{
		m_CommandList.resize(list.size);
		memcpy(&m_CommandList[0], list.commands, sizeof(ul::Command) * list.size);
	}
}

void GPUDriverD3D11::DrawCommandList()
{
	if (m_CommandList.empty())
		return;
    //LOGINFO("DrawCmdList");
    m_CurrentlyBoundRenderTargetId = 0;
    m_RenderTargetForViewWithMSAAIsCurrentlyBound = false;
	for (auto& cmd : m_CommandList)
	{
		if (cmd.command_type == ul::CommandType::DrawGeometry)
			DrawGeometry(cmd.geometry_id, cmd.indices_count, cmd.indices_offset, cmd.gpu_state);
		else if (cmd.command_type == ul::CommandType::ClearRenderBuffer)
			ClearRenderBuffer(cmd.gpu_state.render_buffer_id);
	}
	m_CommandList.clear();
}

////////////////////////////////

GPUDriverD3D11::GPUDriverD3D11()
{
    m_D3DPtr = D3DClass::GetInstance();
    LoadShaders();
    InitializeSamplerState();
    InitializeBlendStates();
    InitializeRasterizerStates();

    if (!m_ConstantBuffer.Initialize())
    {
        FatalError("GPUDriverD3D1::GPUDriverD3D11 failed to initialize constant buffer.");
    }
}

void GPUDriverD3D11::CreateTexture(uint32_t textureId, ul::RefPtr<ul::Bitmap> bitmap)
{
    auto i = m_TextureMap.find(textureId);
    if (i != m_TextureMap.end())
        FatalError("GPUDriverD3D11::CreateTexture, texture id already exists.");

    if (!(bitmap->format() == ul::BitmapFormat::BGRA8_UNORM_SRGB ||
          bitmap->format() == ul::BitmapFormat::A8_UNORM))
        FatalError("GPUDriverD3D11::CreateTexture, unsupported format.");

    ID3D11Device* pDevice = m_D3DPtr->m_Device.Get();

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = bitmap->width();
    desc.Height = bitmap->height();
    desc.MipLevels = desc.ArraySize = 1;
    desc.Format = bitmap->format() == ul::BitmapFormat::BGRA8_UNORM_SRGB ? DXGI_FORMAT_B8G8R8A8_UNORM : DXGI_FORMAT_A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;

    auto& textureEntry = m_TextureMap[textureId];
    HRESULT hr;

    if (bitmap->IsEmpty())
    {
        desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.CPUAccessFlags = 0;

        hr = pDevice->CreateTexture2D(&desc, NULL, &textureEntry.Texture);
    }
    else
    {
        D3D11_SUBRESOURCE_DATA textureData;
        ZeroMemory(&textureData, sizeof(textureData));
        textureData.pSysMem = bitmap->LockPixels();
        textureData.SysMemPitch = bitmap->row_bytes();
        textureData.SysMemSlicePitch = (UINT)bitmap->size();

        hr = pDevice->CreateTexture2D(&desc, &textureData, &textureEntry.Texture);
        bitmap->UnlockPixels();
    }
    FatalErrorIfFail(hr, "GPUDriverD3D11::CreateTexture, unable to create texture.");


    D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
    ZeroMemory(&srv_desc, sizeof(srv_desc));
    srv_desc.Format = desc.Format;
    srv_desc.ViewDimension = textureEntry.IsMSAARenderTarget ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MostDetailedMip = 0;
    srv_desc.Texture2D.MipLevels = 1;

    hr = pDevice->CreateShaderResourceView(textureEntry.Texture.Get(), &srv_desc, &textureEntry.TextureSRV);
    FatalErrorIfFail(hr, "GPUDriverD3D11::CreateTexture, unable to create shader resource view for texture.");

}

void GPUDriverD3D11::UpdateTexture(uint32_t textureId, ul::RefPtr<ul::Bitmap> bitmap)
{
    auto iter = m_TextureMap.find(textureId);
    if (iter == m_TextureMap.end())
    {
        FatalError("GPUDriverD3D11::UpdateTexture, texture id doesn't exist.");
    }

    auto& entry = iter->second;
    D3D11_MAPPED_SUBRESOURCE res;
    HRESULT hr = m_D3DPtr->m_Context->Map(entry.Texture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
    if (FAILED(hr))
    {
        DebugBreak();
    }

    if (res.RowPitch == bitmap->row_bytes())
    {
        memcpy(res.pData, bitmap->LockPixels(), bitmap->size());
        bitmap->UnlockPixels();
    }
    else
    {
        ul::RefPtr<ul::Bitmap> mapped_bitmap = ul::Bitmap::Create(bitmap->width(),
                                                                  bitmap->height(),
                                                                  bitmap->format(),
                                                                  res.RowPitch,
                                                                  res.pData,
                                                                  res.RowPitch * bitmap->height(),
                                                                  false);
        ul::IntRect dest_rect = { 0, 0, (int)bitmap->width(), (int)bitmap->height() };
        if (!mapped_bitmap->DrawBitmap(dest_rect, dest_rect, bitmap, false))
        {
            ErrorHandler::LogCriticalError("Failed to draw bitmap.");
        }
    }

    m_D3DPtr->m_Context->Unmap(entry.Texture.Get(), 0);
}

void GPUDriverD3D11::DestroyTexture(uint32_t textureId)
{
    s_UltralightGPUDriverTextureIDManager.StoreId(textureId);
    auto iter = m_TextureMap.find(textureId);
    if (iter != m_TextureMap.end())
    {
        m_TextureMap.erase(iter);
    }
    else
    {
        FatalError("GPUDriverD3D11::DestroyTexture");
    }
}

void GPUDriverD3D11::CreateRenderBuffer(uint32_t renderBufferId, const ul::RenderBuffer& buffer)
{
    LOGINFO("CreateRenderBuffer()\n");

    if (renderBufferId == 0)
        FatalError("GPUDriverD3D11::CreateRenderBuffer, render buffer ID 0 is reserved for default render target view.");

    auto i = m_RenderTargetMap.find(renderBufferId);
    if (i != m_RenderTargetMap.end())
        FatalError("GPUDriverD3D11::CreateRenderBuffer, render buffer id already exists.");

    auto textureIter = m_TextureMap.find(buffer.texture_id);
    if (textureIter == m_TextureMap.end())
        FatalError("GPUDriverD3D11::CreateRenderBuffer, texture id doesn't exist.");

    auto& textureEntry = textureIter->second;
    textureEntry.IsRenderBuffer = true;
    textureEntry.RenderTargetId = renderBufferId;
    
    auto& render_target_entry = m_RenderTargetMap[renderBufferId];
    render_target_entry.RenderTargetTextureId = buffer.texture_id;

    int sampleCount = 1;
    auto sampleLookupIter = m_MSAARenderTargetSampleCountLookup.find(renderBufferId);
    if (sampleLookupIter != m_MSAARenderTargetSampleCountLookup.end())
    {
        render_target_entry.IsMSAARenderTarget = true;
        sampleCount = sampleLookupIter->second;
    }

    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
    renderTargetViewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

    if (render_target_entry.IsMSAARenderTarget)
    {
        renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
        ID3D11Device* pDevice = D3DClass::GetInstance()->m_Device.Get();
        //For MSAA Render targets, this is where we need to update the texture being stored to be a MSAA texture and move the current texture to be used for the resolve texture
        assert(textureEntry.IsRenderBuffer == true);
        assert(textureEntry.IsMSAARenderTarget == false); //This should not be set yet - just sanity checking
        assert(textureEntry.ResolveTexture == nullptr && textureEntry.ResolveSRV == nullptr);

        textureEntry.IsMSAARenderTarget = true;
        //Assign resolve texture/srv to current texture/srv and create a new texture/srv for higher sample count
        textureEntry.ResolveSRV = textureEntry.TextureSRV;
        textureEntry.ResolveTexture = textureEntry.Texture;

        D3D11_TEXTURE2D_DESC desc;
        textureEntry.ResolveTexture->GetDesc(&desc);
        desc.SampleDesc.Count = sampleCount;
        desc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
        HRESULT hr = pDevice->CreateTexture2D(&desc, NULL, &textureEntry.Texture);
        FatalErrorIfFail(hr, "Failed to create the multisampled texture for a MSAA render buffer in ultralight for a view.");

        D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
        textureEntry.ResolveSRV->GetDesc(&srv_desc);

        srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;

        hr = pDevice->CreateShaderResourceView(textureEntry.Texture.Get(), &srv_desc, &textureEntry.TextureSRV);
        FatalErrorIfFail(hr, "Failed to create the multisampled texture SRV for a MSAA render buffer in ultralight for a view.");

        textureEntry.NeedsResolve = true;
    }

    ComPtr<ID3D11Texture2D> tex = textureEntry.Texture;
    
    HRESULT hr = m_D3DPtr->m_Device->CreateRenderTargetView(tex.Get(), &renderTargetViewDesc, render_target_entry.RenderTargetView.GetAddressOf());

    FatalErrorIfFail(hr, "GPUDriverD3D11::CreateRenderBuffer, unable to create render target.");
}

void GPUDriverD3D11::DestroyRenderBuffer(uint32_t renderBufferId)
{
    LOGINFO("DestroyRenderBuffer()\n");
    s_UltralightGPUDriverRenderBufferIDManager.StoreId(renderBufferId);

    auto iter = m_RenderTargetMap.find(renderBufferId);
    if (iter != m_RenderTargetMap.end())
    {
        iter->second.RenderTargetView.Reset();
        m_RenderTargetMap.erase(iter);
    }
    else
    {
        FatalError("GPUDriverD3D11::DestroyRenderBuffer");
    }

    //I think both of these should always be true but just checking anyways
    if (m_RenderBufferToViewIdMap.contains(renderBufferId))
    {
        m_RenderBufferToViewIdMap.erase(renderBufferId);
    }
    if (m_MSAARenderTargetSampleCountLookup.contains(renderBufferId))
    {
        m_MSAARenderTargetSampleCountLookup.erase(renderBufferId);
    }
}

void GPUDriverD3D11::CreateGeometry(uint32_t geometryId,
                                    const ul::VertexBuffer& vertices,
                                    const ul::IndexBuffer& indices)
{
    if (m_GeometryMap.find(geometryId) != m_GeometryMap.end())
        FatalError("GPUDriverD3D11::CreateGeometry called with a geometry id that already exists.");

    GeometryEntry geometry;
    geometry.Format = vertices.format;

    HRESULT hr;

    D3D11_BUFFER_DESC vertex_desc = {};
    vertex_desc.Usage = D3D11_USAGE_DYNAMIC;
    vertex_desc.ByteWidth = vertices.size;
    vertex_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertex_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA vertex_data = {};
    vertex_data.pSysMem = vertices.data;

    hr = m_D3DPtr->m_Device->CreateBuffer(&vertex_desc,
                                          &vertex_data,
                                          geometry.VertexBuffer.GetAddressOf());
    FatalErrorIfFail(hr, "GPUDriverD3D11::CreateGeometry CreateBuffer for vertex buffer failed.");

    D3D11_BUFFER_DESC index_desc = {};
    index_desc.Usage = D3D11_USAGE_DYNAMIC;
    index_desc.ByteWidth = indices.size;
    index_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    index_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA index_data = {};
    index_data.pSysMem = indices.data;

    hr = m_D3DPtr->m_Device->CreateBuffer(&index_desc,
                                          &index_data,
                                          geometry.IndexBuffer.GetAddressOf());
    FatalErrorIfFail(hr, "GPUDriverD3D11::CreateGeometry CreateBuffer for index buffer failed.");

    m_GeometryMap.insert({ geometryId, std::move(geometry) });
}

void GPUDriverD3D11::UpdateGeometry(uint32_t geometryId,
                                    const ul::VertexBuffer& vertices,
                                    const ul::IndexBuffer& indices)
{
    auto iter = m_GeometryMap.find(geometryId);
    if (iter == m_GeometryMap.end())
    {
        FatalError("GPUDriverD3D11::UpdateGeometry, geometry id doesn't exist.");
    }

    auto& entry = iter->second;
    D3D11_MAPPED_SUBRESOURCE res;

    HRESULT hr = m_D3DPtr->m_Context->Map(entry.VertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
    if (FAILED(hr))
    {
        DebugBreak();
    }
    memcpy(res.pData, vertices.data, vertices.size);
    m_D3DPtr->m_Context->Unmap(entry.VertexBuffer.Get(), 0);

    hr = m_D3DPtr->m_Context->Map(entry.IndexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
    if (FAILED(hr))
    {
        DebugBreak();
    }
    memcpy(res.pData, indices.data, indices.size);
    m_D3DPtr->m_Context->Unmap(entry.IndexBuffer.Get(), 0);
}

void GPUDriverD3D11::DestroyGeometry(uint32_t geometryId)
{
    s_UltralightGPUDriverGeometryIDManager.StoreId(geometryId);

    auto iter = m_GeometryMap.find(geometryId);
    if (iter != m_GeometryMap.end())
    {
        iter->second.VertexBuffer.Reset();
        iter->second.IndexBuffer.Reset();
        m_GeometryMap.erase(iter);
    }
    else
    {
        FatalError("GPUDriverD3D11::DestroyRenderBuffer");
    }
}

void GPUDriverD3D11::DrawGeometry(uint32_t geometryId,
                                  uint32_t indexCount,
                                  uint32_t indexOffset,
                                  const ul::GPUState& state)
{
    BindRenderBuffer(state.render_buffer_id);

    SetViewport(state.viewport_width, state.viewport_height);

    if (state.texture_1_id)
        BindTexture(0, state.texture_1_id);

    if (state.texture_2_id)
        BindTexture(1, state.texture_2_id);

    UpdateConstantBuffer(state);

    BindGeometry(geometryId);

    m_D3DPtr->m_Context->PSSetSamplers(0, 1, m_SamplerState.GetAddressOf());

    //Bind Vertex/Pixel Shaders
    if (state.shader_type == ul::ShaderType::Fill)
    {
        m_D3DPtr->m_Context->VSSetShader(m_VertexShader_Fill.GetShader(), nullptr, 0);
        m_D3DPtr->m_Context->PSSetShader(m_PixelShader_Fill.GetShader(), nullptr, 0);
    }
    else //fillpath
    {
        m_D3DPtr->m_Context->VSSetShader(m_VertexShader_FillPath.GetShader(), nullptr, 0);
        m_D3DPtr->m_Context->PSSetShader(m_PixelShader_FillPath.GetShader(), nullptr, 0);
    }

    if (state.enable_blend)
        m_D3DPtr->m_Context->OMSetBlendState(m_BlendState_Enabled.Get(), NULL, 0xFFFFFFFF);
    else
        m_D3DPtr->m_Context->OMSetBlendState(m_BlendState_Disabled.Get(), NULL, 0xFFFFFFFF);

    if (state.enable_scissor)
    {
        if(m_RenderTargetForViewWithMSAAIsCurrentlyBound)
        {
            m_D3DPtr->m_Context->RSSetState(m_RasterizerState_Scissored_MSAA.Get());
        }
        else
        {
            m_D3DPtr->m_Context->RSSetState(m_RasterizerState_Scissored.Get());
        }
        D3D11_RECT scissor_rect =
        {
          (LONG)(state.scissor_rect.left),
          (LONG)(state.scissor_rect.top),
          (LONG)(state.scissor_rect.right),
          (LONG)(state.scissor_rect.bottom)
        };

        m_D3DPtr->m_Context->RSSetScissorRects(1, &scissor_rect);
    }
    else
    {
        if (m_RenderTargetForViewWithMSAAIsCurrentlyBound)
        {
            m_D3DPtr->m_Context->RSSetState(m_RasterizerState_Default_MSAA.Get());
        }
        else
        {
            m_D3DPtr->m_Context->RSSetState(m_RasterizerState_Default.Get());
        }
    }

    m_D3DPtr->m_Context->VSSetConstantBuffers(0, 1, m_ConstantBuffer.GetAddressOf());
    m_D3DPtr->m_Context->PSSetConstantBuffers(0, 1, m_ConstantBuffer.GetAddressOf());
    m_D3DPtr->m_Context->DrawIndexed(indexCount, indexOffset, 0);
}

void GPUDriverD3D11::ClearRenderBuffer(uint32_t renderBufferId)
{
    assert(renderBufferId != 0);

    float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; //Clear to black

    auto renderTargetIter = m_RenderTargetMap.find(renderBufferId);
    if (renderTargetIter == m_RenderTargetMap.end())
    {
        FatalError("GPUDriverD3D11::ClearRenderBuffer, render buffer id doesn't exist.");
    }

    auto& entry = renderTargetIter->second;

    m_D3DPtr->m_Context->ClearRenderTargetView(entry.RenderTargetView.Get(), color);

    if (entry.IsMSAARenderTarget)
    {
        auto textureIter = m_TextureMap.find(renderTargetIter->second.RenderTargetTextureId);
        if (textureIter == m_TextureMap.end())
            FatalError("GPUDriverD3D11::ClearRenderBuffer, render target texture id doesn't exist.");

        // Flag the MSAA render target texture for Resolve when we bind it to a shader for reading later.
        textureIter->second.NeedsResolve = true;
    }

}

ID3D11ShaderResourceView* GPUDriverD3D11::GetShaderResourceView(ul::View* pView)
{
    auto textureId = pView->render_target().texture_id;
    auto iter = m_TextureMap.find(textureId);
    if (iter == m_TextureMap.end())
        return nullptr;
    auto& entry = iter->second;
    if (entry.IsMSAARenderTarget)
    {
        m_D3DPtr->m_Context->ResolveSubresource(entry.ResolveTexture.Get(), 0, entry.Texture.Get(), 0, DXGI_FORMAT_B8G8R8A8_UNORM);
        return entry.ResolveSRV.Get();
    }
    return entry.TextureSRV.Get();
}

ID3D11Texture2D* GPUDriverD3D11::GetTexture(ul::View* view)
{
    auto textureId = view->render_target().texture_id;
    auto iter = m_TextureMap.find(textureId);
    if (iter == m_TextureMap.end())
        return nullptr;
    auto& entry = iter->second;
    if (entry.IsMSAARenderTarget)
    {
        return entry.ResolveTexture.Get();
    }
    return entry.Texture.Get();
}

IGPUDriverD3D11::StoredEntries GPUDriverD3D11::GetStoredResourceEntries()
{
    IGPUDriverD3D11::StoredEntries entries;
    entries.Geometries = m_GeometryMap;
    entries.RenderTargets = m_RenderTargetMap;
    entries.TextureEntries = m_TextureMap;
    return entries;
}

void GPUDriverD3D11::RegisterStoredResourceEntries(StoredEntries& entries)
{
    m_GeometryMap = entries.Geometries;
    m_RenderTargetMap = entries.RenderTargets;
    m_TextureMap = entries.TextureEntries;
}

uint32_t GPUDriverD3D11::RegisterCustomTextureAndReserveId(shared_ptr<Texture> texture)
{
    uint32_t id = NextTextureId();
    TextureEntry& entry = m_TextureMap[id];
    entry.Texture = texture->GetTextureResource();
    entry.TextureSRV = texture->GetTextureResourceView();
    return id;
}

void GPUDriverD3D11::LoadShaders()
{
    bool result;
    //Fill Path Vertex Shader
    std::wstring fillPathVertexShaderPath = L"vs_fill_path.cso";
    result = m_VertexShader_FillPath.Initialize(fillPathVertexShaderPath,
                                                InputLayoutDescription::ultralight_2f_4ub_2f);
    FatalErrorIfFalse(result, "Failed to initialize fill path vertex shader.");
    //Fill Path Pixel Shader
    std::wstring fillPathPixelShaderPath = L"ps_fill_path.cso";
    result = m_PixelShader_FillPath.Initialize(fillPathPixelShaderPath);
    FatalErrorIfFalse(result, "Failed to initialize fill path pixel shader.");

    //Fill Vertex Shader
    std::wstring fillVertexShaderPath = L"vs_fill.cso";
    result = m_VertexShader_Fill.Initialize(fillVertexShaderPath,
                                            InputLayoutDescription::ultralight_2f_4ub_2f_2f_28f);
    FatalErrorIfFalse(result, "Failed to initialize fill path vertex shader.");
    //Fill Pixel Shader
    std::wstring fillPixelShaderPath = L"ps_fill.cso";
    result = m_PixelShader_Fill.Initialize(fillPixelShaderPath);
    FatalErrorIfFalse(result, "Failed to initialize fill path pixel shader.");
}

void GPUDriverD3D11::InitializeSamplerState()
{
    D3D11_SAMPLER_DESC sampler_desc = {};
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampler_desc.MinLOD = 0;
    HRESULT hr = m_D3DPtr->m_Device->CreateSamplerState(&sampler_desc, &m_SamplerState);
    FatalErrorIfFail(hr, "GPUDriverD3D11::InitializeSamplerAndBlendStates Failed to create sampler state.");
}

void GPUDriverD3D11::InitializeBlendStates()
{
    //Create enabled blend state
    CD3D11_BLEND_DESC blend_desc_enabled(D3D11_DEFAULT);
    blend_desc_enabled.RenderTarget[0].BlendEnable = TRUE; //Disabled by default
    blend_desc_enabled.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blend_desc_enabled.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
    blend_desc_enabled.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;

    HRESULT hr = m_D3DPtr->m_Device->CreateBlendState(&blend_desc_enabled, &m_BlendState_Enabled);
    FatalErrorIfFail(hr, "GPUDriverD3D11::InitializeBlendStates failed to create enabled blend state");

    //Create disabled blend state
    CD3D11_BLEND_DESC blend_desc_disabled(D3D11_DEFAULT); //disabled blend by default

    hr = m_D3DPtr->m_Device->CreateBlendState(&blend_desc_disabled, &m_BlendState_Disabled);
    FatalErrorIfFail(hr, "GPUDriverD3D11::InitializeBlendStates failed to create disabled blend state");
}

void GPUDriverD3D11::InitializeRasterizerStates()
{
    HRESULT hr;
    //Create default rasterizer state (no scissor)
    CD3D11_RASTERIZER_DESC rasterizerDesc_default(D3D11_DEFAULT);
    rasterizerDesc_default.CullMode = D3D11_CULL_NONE;
    rasterizerDesc_default.DepthClipEnable = FALSE;

    hr = m_D3DPtr->m_Device->CreateRasterizerState(&rasterizerDesc_default,
                                                   &m_RasterizerState_Default);
    FatalErrorIfFail(hr, "GPUDriverD3D11::InitializeRasterizerStates failed to create default rasterizer state.");

    CD3D11_RASTERIZER_DESC rasterizerDesc_default_msaa = rasterizerDesc_default;
    rasterizerDesc_default_msaa.MultisampleEnable = true;
    rasterizerDesc_default_msaa.AntialiasedLineEnable = true;
    hr = m_D3DPtr->m_Device->CreateRasterizerState(&rasterizerDesc_default_msaa,
                                                   &m_RasterizerState_Default_MSAA);
    FatalErrorIfFail(hr, "GPUDriverD3D11::InitializeRasterizerStates failed to create default MSAA rasterizer state.");

    CD3D11_RASTERIZER_DESC rasterizerDesc_scissor(D3D11_DEFAULT);
    rasterizerDesc_scissor.CullMode = D3D11_CULL_NONE;
    rasterizerDesc_scissor.DepthClipEnable = FALSE;
    rasterizerDesc_scissor.ScissorEnable = true;

    hr = m_D3DPtr->m_Device->CreateRasterizerState(&rasterizerDesc_scissor,
                                                   &m_RasterizerState_Scissored);
    FatalErrorIfFail(hr, "GPUDriverD3D11::InitializeRasterizerStates failed to create scissored rasterizer state.");


    CD3D11_RASTERIZER_DESC rasterizerDesc_scissor_msaa = rasterizerDesc_scissor;
    rasterizerDesc_scissor.MultisampleEnable = true;
    rasterizerDesc_scissor.AntialiasedLineEnable = true;
    hr = m_D3DPtr->m_Device->CreateRasterizerState(&rasterizerDesc_scissor,
                                                   &m_RasterizerState_Scissored_MSAA);
    FatalErrorIfFail(hr, "GPUDriverD3D11::InitializeRasterizerStates failed to create scissored MSAA rasterizer state.");

}

void GPUDriverD3D11::BindRenderBuffer(uint32_t renderBufferId)
{
    if (renderBufferId == m_CurrentlyBoundRenderTargetId)
    {
        return;
    }
    m_CurrentlyBoundRenderTargetId = renderBufferId;

    // Unbind any textures/shader resources to avoid warnings in case a render buffer that we would like to bind is already bound as an input texture.
    ID3D11ShaderResourceView* nullSRV[3] = { nullptr, nullptr, nullptr };
    m_D3DPtr->m_Context->PSSetShaderResources(0, 3, nullSRV);

    ID3D11RenderTargetView* target;
    assert(renderBufferId != 0);
    auto renderTarget = m_RenderTargetMap.find(renderBufferId);
    if (renderTarget == m_RenderTargetMap.end())
        FatalError("GPUDriverD3D11::BindRenderBuffer, render buffer id doesn't exist.");

    target = renderTarget->second.RenderTargetView.Get();

    if (m_MSAARenderTargetSampleCountLookup.contains(renderBufferId))
    {
        m_RenderTargetForViewWithMSAAIsCurrentlyBound = true;

        auto renderTargetTexture = m_TextureMap.find(renderTarget->second.RenderTargetTextureId);
        if (renderTargetTexture == m_TextureMap.end())
            FatalError("GPUDriverD3D11::BindRenderBuffer, render target texture id doesn't exist.");

        // Flag the MSAA render target texture for Resolve when we bind it to a shader for reading later.
        renderTargetTexture->second.NeedsResolve = true;
    }
    else
    {
        m_RenderTargetForViewWithMSAAIsCurrentlyBound = false;
    }

    m_D3DPtr->m_Context->OMSetRenderTargets(1, &target, nullptr);
}

void GPUDriverD3D11::SetViewport(uint32_t width, uint32_t height)
{
    D3D11_VIEWPORT vp = {};
    vp.Width = (float)width;
    vp.Height = (float)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    m_D3DPtr->m_Context->RSSetViewports(1, &vp);
}

void GPUDriverD3D11::BindTexture(uint8_t textureUnit, uint32_t textureId)
{
    auto iter = m_TextureMap.find(textureId);
    if (iter == m_TextureMap.end())
    {
        FatalError("GPUDriverD3D11::BindTexture, texture id doesn't exist.");
    }

    auto& entry = iter->second;

    if (entry.IsMSAARenderTarget)
    {
        if (entry.NeedsResolve)
        {
            m_D3DPtr->m_Context->ResolveSubresource(entry.ResolveTexture.Get(), 0, entry.Texture.Get(), 0, DXGI_FORMAT_B8G8R8A8_UNORM);
            entry.NeedsResolve = false;
        }
        m_D3DPtr->m_Context->PSSetShaderResources(textureUnit, 1, entry.ResolveSRV.GetAddressOf());
    }
    else
    {
        m_D3DPtr->m_Context->PSSetShaderResources(textureUnit, 1, entry.TextureSRV.GetAddressOf());
    }
}

void GPUDriverD3D11::UpdateConstantBuffer(const ul::GPUState& state)
{
    float screenWidth = (float)state.viewport_width;
    float screenHeight = (float)state.viewport_height;
    ul::Matrix modelViewProjectionMat = ApplyProjection(state.transform,
                                                        screenWidth,
                                                        screenHeight);

    auto& cbdata = m_ConstantBuffer.m_Data;
    cbdata.State = { 0.0f, screenWidth, screenHeight, 1.0f };

    cbdata.Transform = DirectX::XMMATRIX(modelViewProjectionMat.GetMatrix4x4().data);
    cbdata.Scalar4[0] =
    {
        state.uniform_scalar[0],
        state.uniform_scalar[1],
        state.uniform_scalar[2],
        state.uniform_scalar[3]
    };
    cbdata.Scalar4[1] =
    {
        state.uniform_scalar[4],
        state.uniform_scalar[5],
        state.uniform_scalar[6],
        state.uniform_scalar[7]
    };
    for (size_t i = 0; i < 8; ++i)
        cbdata.Vector[i] = DirectX::XMFLOAT4(state.uniform_vector[i].x,
                                             state.uniform_vector[i].y,
                                             state.uniform_vector[i].z,
                                             state.uniform_vector[i].w);
    cbdata.ClipSize = state.clip_size;
    for (size_t i = 0; i < state.clip_size; ++i)
        cbdata.Clip[i] = DirectX::XMMATRIX(state.clip[i].data);
    m_ConstantBuffer.ApplyChanges();
}

void GPUDriverD3D11::BindGeometry(uint32_t geometryId)
{
    auto iter = m_GeometryMap.find(geometryId);
    if (iter == m_GeometryMap.end())
    {
        FatalError("GPUDriverD3D11::BindGeometry geometry id does not exist in geometry map.");
    }

    auto& geometry = iter->second;

    UINT stride = geometry.Format == ul::VertexBufferFormat::_2f_4ub_2f ? sizeof(ul::Vertex_2f_4ub_2f) : sizeof(ul::Vertex_2f_4ub_2f_2f_28f);
    UINT offset = 0;
    m_D3DPtr->m_Context->IASetVertexBuffers(0, 1, geometry.VertexBuffer.GetAddressOf(), &stride, &offset);
    m_D3DPtr->m_Context->IASetIndexBuffer(geometry.IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    m_D3DPtr->m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    if (geometry.Format == ul::VertexBufferFormat::_2f_4ub_2f)
        m_D3DPtr->m_Context->IASetInputLayout(m_VertexShader_FillPath.GetInputLayout());
    else
        m_D3DPtr->m_Context->IASetInputLayout(m_VertexShader_Fill.GetInputLayout());
}

ul::Matrix GPUDriverD3D11::ApplyProjection(const ul::Matrix4x4& transform, float screenWidth, float screenHeight)
{
    ul::Matrix transformMatrix;
    transformMatrix.Set(transform);

    ul::Matrix result;
    result.SetOrthographicProjection(screenWidth, screenHeight, false);
    result.Transform(transformMatrix);

    return result;
}