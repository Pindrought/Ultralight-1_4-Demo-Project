#pragma once
#include <PCH.h>
#include "D3DClass.h"
#include "PipelineState.h"
#include "Buffer/ConstantBuffer.h"
#include "Renderable/Mesh2D.h"

class Renderer
{
public:
	static Renderer* GetInstance();
	bool Initialize();
	D3DClass* GetD3D();
	void ActivateRenderTarget(RenderTargetContainer* pRenderTargetContainer);
	void ClearRenderTarget(RenderTargetContainer* pRenderTargetContainer);
	void PrepareFor2DRendering(RenderTargetContainer* pRenderTargetContainer);
	void ActivatePipelineState(shared_ptr<PipelineState> pipelineState);
	shared_ptr<PipelineState> GetPipelineState(string name);
	void RegisterPipelineState(std::shared_ptr<PipelineState> pipelineState);
	void RenderUltralightView(UltralightView* pUltralightView);
	bool RenderSceneInRenderTargetContainer(RenderTargetContainer* pRenderTargetContainer);
	void DrawSprite(Texture* pTexture, float x, float y, float z, float width, float height);
	bool RenderEntity(Entity* pEntity);
	~Renderer();
private:
	bool BuildQuadMeshForUltralightView();
	static Renderer* s_Instance; //Only ever one renderer instance
	shared_ptr<D3DClass> m_D3D;
	shared_ptr<PipelineState> m_ActivePipelineState = nullptr;
	unordered_map<string, shared_ptr<PipelineState>> m_PipelineStatesMap;
	ConstantBuffer<ConstantBufferType::CB_PerFrameData_2D> m_CB_PerFrameData_2D;
	ConstantBuffer<ConstantBufferType::CB_PerDrawData_2D> m_CB_PerDrawData_2D;
	ConstantBuffer<ConstantBufferType::CB_UltralightData> m_CB_UltralightData;
	ConstantBuffer<ConstantBufferType::CB_PerFrameData_3D> m_CB_PerFrameData_3D;
	ConstantBuffer<ConstantBufferType::CB_PerDrawData_3D> m_CB_PerDrawData_3D;
	ConstantBuffer<ConstantBufferType::CB_Material> m_CB_Material;

	float m_DeltaTime = 0; //DeltaTime for animation processing for RenderEntity calls -- Assigned at start of RenderSceneInRenderTargetContainer

	Mesh2D m_QuadMeshForUltralightView; //This is also used for sprite rendering in DrawSprite()
};