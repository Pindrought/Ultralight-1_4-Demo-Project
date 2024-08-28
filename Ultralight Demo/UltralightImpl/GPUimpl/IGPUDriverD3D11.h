#pragma once
#include <PCH.h>
#include "../Graphics/Texture.h"

struct GeometryEntry
{
    ul::VertexBufferFormat Format;
    ComPtr<ID3D11Buffer> VertexBuffer;
    ComPtr<ID3D11Buffer> IndexBuffer;
};

struct TextureEntry
{
    ComPtr<ID3D11Texture2D> Texture;
    ComPtr<ID3D11ShaderResourceView> TextureSRV;
    bool IsMSAARenderTarget = false;
    bool NeedsResolve = false;
    ComPtr<ID3D11Texture2D> ResolveTexture;
    ComPtr<ID3D11ShaderResourceView> ResolveSRV;
    bool IsRenderBuffer = false; //This isn't really necessary since I have the RenderTargetId variable but just keeping it for now
    uint32_t RenderTargetId = 0; //This is only to be used when this is a texture for a render buffer. This is used for allowing diff MSAA levels
};

struct RenderTargetEntry
{
    ComPtr<ID3D11RenderTargetView> RenderTargetView;
    uint32_t RenderTargetTextureId = 0;
    bool IsMSAARenderTarget = false;
};

class IGPUDriverD3D11 : public ul::GPUDriver
{
public:
    virtual void BeginSynchronize() override = 0;
    virtual void EndSynchronize() override = 0;
    virtual uint32_t NextTextureId() override = 0;
    virtual uint32_t NextRenderBufferId() override = 0;
    virtual uint32_t NextGeometryId() override = 0;
    virtual void UpdateCommandList(const ul::CommandList& list) override = 0;

    virtual void CreateTexture(uint32_t textureId, ul::RefPtr<ul::Bitmap> bitmap) override = 0;
    virtual void UpdateTexture(uint32_t textureId, ul::RefPtr<ul::Bitmap> bitmap) override = 0;
    virtual void DestroyTexture(uint32_t textureId) override = 0;
    virtual void CreateRenderBuffer(uint32_t renderBufferId, const ul::RenderBuffer& buffer) override = 0;
    virtual void DestroyRenderBuffer(uint32_t renderBufferId) override = 0;
    virtual void CreateGeometry(uint32_t geometryId, const ul::VertexBuffer& vertices, const ul::IndexBuffer& indices) override = 0;
    virtual void UpdateGeometry(uint32_t geometryId, const ul::VertexBuffer& vertices, const ul::IndexBuffer& indices) override = 0;
    virtual void DestroyGeometry(uint32_t geometryId) override = 0;

    virtual void DrawGeometry(uint32_t geometryId, uint32_t indexCount, uint32_t indexOffset, const ul::GPUState& state) = 0;
    virtual void ClearRenderBuffer(uint32_t renderBufferId) = 0;

    //Below are the added methods for my interface for the gpu driver
    //The reason I set up an interface is so a different gpu driver impl could be passed into ultralight when starting up instead of using my default impl
    //Ex. for the demo when using the cpp textures in ultralight the functionality is different
    virtual void DrawCommandList() = 0;
    virtual ID3D11ShaderResourceView* GetShaderResourceView(ul::View* pView) = 0;
    virtual ID3D11Texture2D* GetTexture(ul::View* pView) = 0;
    virtual uint32_t RegisterCustomTextureAndReserveId(shared_ptr<Texture> texture) = 0;

    //I wanted the ability to swap between gpu drivers when closing/restarting the engine class for the different demos
    //This introduced an issue though where sometimes geometry or render targets or textures were not being "destroyed" according to ultralight
    //Even though the gpu driver was technically a completely different instance and those assets had already actually been deallocated
    //Because of this, i'm storing the id's it thinks are still out there so I can validate against them for error checking when destroying a texture/renderarget/geometry that doesn't exist
    //I hate this solution and if I didn't need the option of switching between different gpu driver impl's during runtime I wouldn't have done this
    struct StoredEntries {
        std::map<uint32_t, GeometryEntry> Geometries;
        std::map<uint32_t, RenderTargetEntry> RenderTargets;
        std::map<uint32_t, TextureEntry> TextureEntries;
    };

    virtual StoredEntries GetStoredResourceEntries() = 0;
    virtual void RegisterStoredResourceEntries(StoredEntries& entries) = 0;
};