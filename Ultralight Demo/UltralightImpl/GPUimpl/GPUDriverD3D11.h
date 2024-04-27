#pragma once
#include <PCH.h>
#include "../../Graphics/Shader/PixelShader.h"
#include "../../Graphics/Shader/VertexShader.h"
#include "../../Graphics/Buffer/ConstantBuffer.h"

struct GeometryEntry
{
    ul::VertexBufferFormat format;
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> indexBuffer;
};

struct TextureEntry
{
    ComPtr<ID3D11Texture2D> texture;
    ComPtr<ID3D11ShaderResourceView> textureSRV;
    bool isMSAARenderTarget = false;
    bool needsResolve = false;
    ComPtr<ID3D11Texture2D> resolveTexture;
    ComPtr<ID3D11ShaderResourceView> resolveSRV;
    bool isRenderBuffer = false;
};

struct RenderTargetEntry
{
    ComPtr<ID3D11RenderTargetView> renderTargetView;
    uint32_t renderTargetTextureId;
};

class GPUDriverD3D11 : public ul::GPUDriver
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

    void DrawCommandList();
    virtual void DrawGeometry(uint32_t geometryId, uint32_t indexCount, uint32_t indexOffset, const ul::GPUState& state);
    virtual void ClearRenderBuffer(uint32_t renderBufferId);
    ID3D11ShaderResourceView* GetShaderResourceView(ul::View* pView);
    ID3D11Texture2D* GetTexture(ul::View* view);

private:
    uint32_t m_NextTextureId = 1;
    uint32_t m_NextRenderBufferId = 1;
    uint32_t m_NextGeometryId = 1;
    std::vector<ul::Command> m_CommandList;


    std::vector<ID3D11ShaderResourceView*> GetSRVs();
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
    std::map<uint32_t, GeometryEntry> m_GeometryMap;
    std::map<uint32_t, RenderTargetEntry> m_RenderTargetMap;
    std::map<uint32_t, TextureEntry> m_TextureMap;
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
    D3DClass* m_D3DPtr = nullptr;
    map<uint32_t, uint32_t> m_ViewToTextureIdMap;
};