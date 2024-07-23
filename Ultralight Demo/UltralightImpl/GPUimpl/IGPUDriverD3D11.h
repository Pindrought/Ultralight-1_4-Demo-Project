#pragma once
#include <PCH.h>

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
};