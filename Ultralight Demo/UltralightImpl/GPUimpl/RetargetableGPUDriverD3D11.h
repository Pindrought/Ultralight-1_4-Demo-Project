//#pragma once
//#include <PCH.h>
//#include "IGPUDriverD3D11.h"
//#include "../../Graphics/Shader/PixelShader.h"
//#include "../../Graphics/Shader/VertexShader.h"
//#include "../../Graphics/Buffer/ConstantBuffer.h"
//
//class RetargetableGPUDriverD3D11 : public IGPUDriverD3D11
//{
//public:
//    //Inherited from GPUDriver
//    RetargetableGPUDriverD3D11();
//    void SetGPUImpl(shared_ptr<IGPUDriverD3D11> pImpl);
//    void BeginSynchronize() override;
//    void EndSynchronize() override;
//    uint32_t NextTextureId() override;
//    uint32_t NextRenderBufferId() override;
//    uint32_t NextGeometryId() override;
//    void UpdateCommandList(const ul::CommandList& list) override;
//
//    void CreateTexture(uint32_t textureId, ul::RefPtr<ul::Bitmap> bitmap) override;
//    void UpdateTexture(uint32_t textureId, ul::RefPtr<ul::Bitmap> bitmap) override;
//    void DestroyTexture(uint32_t textureId) override;
//    void CreateRenderBuffer(uint32_t renderBufferId, const ul::RenderBuffer& buffer) override;
//    void DestroyRenderBuffer(uint32_t renderBufferId) override;
//    void CreateGeometry(uint32_t geometryId, const ul::VertexBuffer& vertices, const ul::IndexBuffer& indices) override;
//    void UpdateGeometry(uint32_t geometryId, const ul::VertexBuffer& vertices, const ul::IndexBuffer& indices) override;
//    void DestroyGeometry(uint32_t geometryId) override;
//
//    void DrawGeometry(uint32_t geometryId, uint32_t indexCount, uint32_t indexOffset, const ul::GPUState& state);
//    void ClearRenderBuffer(uint32_t renderBufferId);
//
//    void DrawCommandList() override;
//    ID3D11ShaderResourceView* GetShaderResourceView(ul::View* pView) override;
//    ID3D11Texture2D* GetTexture(ul::View* pView) override;
//
//    virtual StoredEntries GetStoredResourceEntries() override;
//    virtual void RegisterStoredResourceEntries(StoredEntries& entries) override;
//
//    shared_ptr<IGPUDriverD3D11> m_CurrentGPUDriverImpl = nullptr;
//private:
//};