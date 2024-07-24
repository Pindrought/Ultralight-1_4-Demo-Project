#pragma once
#include <PCH.h>
#include "../UltralightImpl/GPUimpl/IGPUDriverD3D11.h"
#include "../../../Graphics/Shader/PixelShader.h"
#include "../../../Graphics/Shader/VertexShader.h"
#include "../../../Graphics/Buffer/ConstantBuffer.h"
#include "../../../Graphics/Texture.h"

struct CustomTextureEntry : public TextureEntry
{
    bool IsCPPTexture = false;
    CustomTextureEntry() :TextureEntry() { };
    CustomTextureEntry(TextureEntry other)
    {
        this->IsMSAARenderTarget = other.IsMSAARenderTarget;
        this->IsRenderBuffer = other.IsRenderBuffer;
        this->NeedsResolve = other.NeedsResolve;
        this->ResolveSRV = other.ResolveSRV;
        this->ResolveTexture = other.ResolveTexture;
        this->Texture = other.Texture;
        this->TextureSRV = other.TextureSRV;
    }
    TextureEntry ToTextureEntry() //For converting back to original texture entry and losing custom fields for when switching gpu driver impl
    {
        TextureEntry entry;

        entry.IsMSAARenderTarget = this->IsMSAARenderTarget;
        entry.IsRenderBuffer = this->IsRenderBuffer;
        entry.NeedsResolve = this->NeedsResolve;
        entry.ResolveSRV = this->ResolveSRV;
        entry.ResolveTexture = this->ResolveTexture;
        entry.Texture = this->Texture;
        entry.TextureSRV = this->TextureSRV;

        return entry;
    }
};

struct TextureOverwriteData
{
    uint32_t PixelUID = 0;
    shared_ptr<Texture> Texture = nullptr;
    string Alias = "";
};

class CustomGPUImpl : public IGPUDriverD3D11
{
public:
    //Inherited from GPUDriver
    CustomGPUImpl();
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

    void QueueCPPTextureOverwrite(TextureOverwriteData data);

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
    std::map<uint32_t, GeometryEntry> m_GeometryMap;
    std::map<uint32_t, RenderTargetEntry> m_RenderTargetMap;
    std::map<uint32_t, CustomTextureEntry> m_TextureMap;
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
    //map<uint32_t, uint32_t> m_ViewToTextureIdMap;
    map<uint32_t, TextureOverwriteData> m_TextureOverwriteData; //This is for textures to be pulled in from outside ultralight ex. rendertarget from d3d11
};