#include "PCH.h"
#include "RetargetableGPUDriverD3D11.h"

RetargetableGPUDriverD3D11::RetargetableGPUDriverD3D11()
{
}

void RetargetableGPUDriverD3D11::SetGPUImpl(shared_ptr<IGPUDriverD3D11> pImpl)
{
	assert(pImpl != nullptr);
	m_CurrentGPUDriverImpl = pImpl;
}

void RetargetableGPUDriverD3D11::BeginSynchronize()
{
	assert(m_CurrentGPUDriverImpl != nullptr);
	m_CurrentGPUDriverImpl->BeginSynchronize();
}

void RetargetableGPUDriverD3D11::EndSynchronize()
{
	assert(m_CurrentGPUDriverImpl != nullptr);
	m_CurrentGPUDriverImpl->EndSynchronize();
}

uint32_t RetargetableGPUDriverD3D11::NextTextureId()
{
	assert(m_CurrentGPUDriverImpl != nullptr);
	return m_CurrentGPUDriverImpl->NextTextureId();
}

uint32_t RetargetableGPUDriverD3D11::NextRenderBufferId()
{
	assert(m_CurrentGPUDriverImpl != nullptr);
	return m_CurrentGPUDriverImpl->NextRenderBufferId();
}

uint32_t RetargetableGPUDriverD3D11::NextGeometryId()
{
	assert(m_CurrentGPUDriverImpl != nullptr);
	return m_CurrentGPUDriverImpl->NextGeometryId();
}

void RetargetableGPUDriverD3D11::UpdateCommandList(const ul::CommandList& list)
{
	assert(m_CurrentGPUDriverImpl != nullptr);
	m_CurrentGPUDriverImpl->UpdateCommandList(list);
}

void RetargetableGPUDriverD3D11::CreateTexture(uint32_t textureId, ul::RefPtr<ul::Bitmap> bitmap)
{
	assert(m_CurrentGPUDriverImpl != nullptr);
	m_CurrentGPUDriverImpl->CreateTexture(textureId, bitmap);
}

void RetargetableGPUDriverD3D11::UpdateTexture(uint32_t textureId, ul::RefPtr<ul::Bitmap> bitmap)
{
	assert(m_CurrentGPUDriverImpl != nullptr);
	m_CurrentGPUDriverImpl->UpdateTexture(textureId, bitmap);
}

void RetargetableGPUDriverD3D11::DestroyTexture(uint32_t textureId)
{
	assert(m_CurrentGPUDriverImpl != nullptr);
	m_CurrentGPUDriverImpl->DestroyTexture(textureId);
}

void RetargetableGPUDriverD3D11::CreateRenderBuffer(uint32_t renderBufferId, const ul::RenderBuffer& buffer)
{
	assert(m_CurrentGPUDriverImpl != nullptr);
	m_CurrentGPUDriverImpl->CreateRenderBuffer(renderBufferId, buffer);
}

void RetargetableGPUDriverD3D11::DestroyRenderBuffer(uint32_t renderBufferId)
{
	assert(m_CurrentGPUDriverImpl != nullptr);
	m_CurrentGPUDriverImpl->DestroyRenderBuffer(renderBufferId);
}

void RetargetableGPUDriverD3D11::CreateGeometry(uint32_t geometryId, const ul::VertexBuffer& vertices, const ul::IndexBuffer& indices)
{
	assert(m_CurrentGPUDriverImpl != nullptr);
	m_CurrentGPUDriverImpl->CreateGeometry(geometryId, vertices, indices);
}

void RetargetableGPUDriverD3D11::UpdateGeometry(uint32_t geometryId, const ul::VertexBuffer& vertices, const ul::IndexBuffer& indices)
{
	assert(m_CurrentGPUDriverImpl != nullptr);
	m_CurrentGPUDriverImpl->UpdateGeometry(geometryId, vertices, indices);
}

void RetargetableGPUDriverD3D11::DestroyGeometry(uint32_t geometryId)
{
	assert(m_CurrentGPUDriverImpl != nullptr);
	m_CurrentGPUDriverImpl->DestroyGeometry(geometryId);
}

void RetargetableGPUDriverD3D11::DrawGeometry(uint32_t geometryId, uint32_t indexCount, uint32_t indexOffset, const ul::GPUState& state)
{
	assert(m_CurrentGPUDriverImpl != nullptr);
	m_CurrentGPUDriverImpl->DrawGeometry(geometryId, indexCount, indexOffset, state);
}

void RetargetableGPUDriverD3D11::ClearRenderBuffer(uint32_t renderBufferId)
{
	assert(m_CurrentGPUDriverImpl != nullptr);
	m_CurrentGPUDriverImpl->ClearRenderBuffer(renderBufferId);
}

void RetargetableGPUDriverD3D11::DrawCommandList()
{
	assert(m_CurrentGPUDriverImpl != nullptr);
	m_CurrentGPUDriverImpl->DrawCommandList();
}

ID3D11ShaderResourceView* RetargetableGPUDriverD3D11::GetShaderResourceView(ul::View* pView)
{
	assert(m_CurrentGPUDriverImpl != nullptr);
	return m_CurrentGPUDriverImpl->GetShaderResourceView(pView);
}

ID3D11Texture2D* RetargetableGPUDriverD3D11::GetTexture(ul::View* pView)
{
	assert(m_CurrentGPUDriverImpl != nullptr);
	return m_CurrentGPUDriverImpl->GetTexture(pView);
}
