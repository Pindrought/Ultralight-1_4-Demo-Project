#pragma once
#include <PCH.h>
#include "IGPUDriverD3D11.h"
#include "../../Graphics/Shader/PixelShader.h"
#include "../../Graphics/Shader/VertexShader.h"
#include "../../Graphics/Buffer/ConstantBuffer.h"

class GPUDriverD3D11 : public IGPUDriverD3D11
{
public:
    //Inherited from GPUDriver
    GPUDriverD3D11();
    virtual void BeginSynchronize() override;
    virtual void EndSynchronize() override;
    virtual uint32_t NextTextureId() override;
    virtual uint32_t NextRenderBufferId() override;
    virtual uint32_t NextGeometryId() override;
    virtual void UpdateCommandList(const ul::CommandList& list) override;

    virtual void CreateTexture(uint32_t textureId, ul::RefPtr<ul::Bitmap> bitmap) override;
    virtual void UpdateTexture(uint32_t textureId, ul::RefPtr<ul::Bitmap> bitmap) override;
    virtual void DestroyTexture(uint32_t textureId) override;
    virtual void CreateRenderBuffer(uint32_t renderBufferId, const ul::RenderBuffer& buffer) override;
    virtual void DestroyRenderBuffer(uint32_t renderBufferId) override;
    virtual void CreateGeometry(uint32_t geometryId, const ul::VertexBuffer& vertices, const ul::IndexBuffer& indices) override;
    virtual void UpdateGeometry(uint32_t geometryId, const ul::VertexBuffer& vertices, const ul::IndexBuffer& indices) override;
    virtual void DestroyGeometry(uint32_t geometryId) override;

    void DrawCommandList() override;
    virtual void DrawGeometry(uint32_t geometryId, uint32_t indexCount, uint32_t indexOffset, const ul::GPUState& state);
    virtual void ClearRenderBuffer(uint32_t renderBufferId);
    ID3D11ShaderResourceView* GetShaderResourceView(ul::View* pView) override;
    ID3D11Texture2D* GetTexture(ul::View* view) override;

    virtual StoredEntries GetStoredResourceEntries() override;
    virtual void RegisterStoredResourceEntries(StoredEntries& entries) override;
private:
    std::vector<ul::Command> m_CommandList;

private:
    void LoadShaders();
    void InitializeSamplerState();
    void InitializeBlendStates();
    void InitializeRasterizerStates();

    void BindRenderBuffer(uint32_t renderBufferId);
    void SetViewport(uint32_t width, uint32_t height);
    void BindTexture(uint8_t textureUnit, uint32_t textureId);
    void UpdateConstantBuffer(const ul::GPUState& state);
    void BindGeometry(uint32_t geometryId);
    ul::Matrix ApplyProjection(const ul::Matrix4x4& transform, float screenWidth, float screenHeight);
    map<uint32_t, GeometryEntry> m_GeometryMap;
    map<uint32_t, RenderTargetEntry> m_RenderTargetMap;
    map<uint32_t, TextureEntry> m_TextureMap;
    VertexShader m_VertexShader_Fill;
    VertexShader m_VertexShader_FillPath;
    PixelShader m_PixelShader_Fill;
    PixelShader m_PixelShader_FillPath;
    ConstantBuffer<ConstantBufferType::CB_UltralightData> m_ConstantBuffer;
    ComPtr<ID3D11SamplerState> m_SamplerState;
    ComPtr<ID3D11BlendState> m_BlendState_Disabled;
    ComPtr<ID3D11BlendState> m_BlendState_Enabled;
    ComPtr<ID3D11RasterizerState> m_RasterizerState_Default;
    ComPtr<ID3D11RasterizerState> m_RasterizerState_Scissored;
    ComPtr<ID3D11RasterizerState> m_RasterizerState_Default_MSAA;
    ComPtr<ID3D11RasterizerState> m_RasterizerState_Scissored_MSAA;
    D3DClass* m_D3DPtr = nullptr;

    map<uint32_t, uint32_t> m_RenderBufferToViewIdMap;
    map<uint32_t, uint32_t> m_MSAARenderTargetSampleCountLookup; //Key=RenderBufferId Value=SampleCount
    uint32_t m_CurrentlyBoundRenderTargetId = 0;
    bool m_RenderTargetForViewWithMSAAIsCurrentlyBound = false;
};