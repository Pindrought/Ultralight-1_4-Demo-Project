#include <PCH.h>
#include "Renderer.h"
#include "D3DClass.h"
#include "RenderTarget/RenderTargetContainer.h"
#include "PipelineStateBuilder.h"
#include "../UltralightImpl/UltralightView.h"
#include "Renderable/Camera.h"

Renderer* Renderer::s_Instance = nullptr; //Only ever one renderer instance

Renderer* Renderer::GetInstance()
{
	assert(s_Instance != nullptr);
	return s_Instance;
}

bool Renderer::Initialize()
{
	if (s_Instance != nullptr)
	{
		ErrorHandler::LogCriticalError("Attempted to initialize renderer more than once.");
		return false;
	}

	s_Instance = this;

	m_D3D = D3DClass::GetInstanceShared();

	if (!m_D3D->Initialize())
	{
		ErrorHandler::LogCriticalError("Failed to initialize DirectX11.");
		return false;
	}

	if (!m_CB_PerDrawData_2D.Initialize())
	{
		ErrorHandler::LogCriticalError("Failed to initialize PerDrawData_2D Constant Buffer.");
		return false;
	}

	if (!m_CB_PerFrameData_2D.Initialize())
	{
		ErrorHandler::LogCriticalError("Failed to initialize PerFrameData_2D Constant Buffer.");
		return false;
	}
	
	if (!m_CB_UltralightData.Initialize())
	{
		ErrorHandler::LogCriticalError("Failed to initialize UltralightData Constant Buffer.");
		return false;
	}

	if (!m_CB_PerFrameData_3D.Initialize())
	{
		ErrorHandler::LogCriticalError("Failed to initialize PerFrameData 3D Constant Buffer.");
		return false;
	}

	if (!m_CB_PerDrawData_3D.Initialize())
	{
		ErrorHandler::LogCriticalError("Failed to initialize PerDrawData 3D Constant Buffer.");
		return false;
	}

	if (!m_CB_Material.Initialize())
	{
		ErrorHandler::LogCriticalError("Failed to initialize Material Constant Buffer.");
		return false;
	}

	if (!m_CB_BoneTransforms.Initialize())
	{
		ErrorHandler::LogCriticalError("Failed to initialize bone data constant buffer.");
		return false;
	}

	if (!PipelineStateBuilder::BuildPipelineStatesForRenderer())
	{
		ErrorHandler::LogCriticalError("Failed to build default pipeline states for renderer.");
		return false;
	}

	if (!BuildQuadMeshForUltralightView())
	{
		ErrorHandler::LogCriticalError("Failed to build quad mesh for Ultralight View rendering.");
		return false;
	}

	return true;
}

D3DClass* Renderer::GetD3D()
{
	return m_D3D.get();
}

void Renderer::ActivateRenderTarget(RenderTargetContainer* pRenderTargetContainer)
{
	m_D3D->m_Context->OMSetRenderTargets(1, 
										 pRenderTargetContainer->GetRenderTargetViewAddressOf(), 
										 pRenderTargetContainer->GetDepthStencilView());
}

void Renderer::ClearRenderTarget(RenderTargetContainer* pRenderTargetContainer)
{
	ActivatePipelineState(nullptr);
	ID3D11DeviceContext* pContext = m_D3D->m_Context.Get();
	pContext->ClearState();
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pContext->RSSetViewports(1, &pRenderTargetContainer->GetViewport());

	pContext->ClearRenderTargetView(pRenderTargetContainer->GetRenderTargetView(),
										   pRenderTargetContainer->GetBackgroundColor());
	pContext->ClearDepthStencilView(pRenderTargetContainer->GetDepthStencilView(),
										   D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
										   1.0f, //Clear depth to 1.0f
										   0);   //Clear stencil to 0
	pContext->VSSetConstantBuffers(0, 1, m_CB_PerFrameData_2D.GetAddressOf());
	pContext->VSSetConstantBuffers(1, 1, m_CB_PerDrawData_2D.GetAddressOf());

	DirectX::XMMATRIX orthoMatrix = DirectX::XMMatrixOrthographicOffCenterLH(0,
																			pRenderTargetContainer->GetWidth(),
																			pRenderTargetContainer->GetHeight(),
																			0,
																			0.0f,
																			1000.0f);

	DirectX::XMStoreFloat4x4(&m_CB_PerFrameData_2D.m_Data.OrthoMatrix, orthoMatrix);
	if (!m_CB_PerFrameData_2D.ApplyChanges())
	{
		ErrorHandler::LogCriticalError("Failed to update the per frame data constant buffer for 2D rendering.");
	}

	pContext->OMSetRenderTargets(1,
								 pRenderTargetContainer->GetRenderTargetViewAddressOf(),
								 pRenderTargetContainer->GetDepthStencilView());
}

void Renderer::PrepareFor2DRendering(RenderTargetContainer* pRenderTargetContainer)
{
	ActivatePipelineState(nullptr);
	ID3D11DeviceContext* pContext = m_D3D->m_Context.Get();
	pContext->ClearState();
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pContext->RSSetViewports(1, &pRenderTargetContainer->GetViewport());

	pContext->ClearDepthStencilView(pRenderTargetContainer->GetDepthStencilView(),
									D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
									1.0f, //Clear depth to 1.0f
									0);   //Clear stencil to 0

	pContext->VSSetConstantBuffers(0, 1, m_CB_PerFrameData_2D.GetAddressOf());
	pContext->VSSetConstantBuffers(1, 1, m_CB_PerDrawData_2D.GetAddressOf());

	DirectX::XMMATRIX orthoMatrix = DirectX::XMMatrixOrthographicOffCenterLH(0,
																			 pRenderTargetContainer->GetWidth(),
																			 pRenderTargetContainer->GetHeight(),
																			 0,
																			 0.0f,
																			 1000.0f);

	DirectX::XMStoreFloat4x4(&m_CB_PerFrameData_2D.m_Data.OrthoMatrix, orthoMatrix);
	if (!m_CB_PerFrameData_2D.ApplyChanges())
	{
		ErrorHandler::LogCriticalError("Failed to update the per frame data constant buffer for 2D rendering.");
	}

	pContext->OMSetRenderTargets(1,
								 pRenderTargetContainer->GetRenderTargetViewAddressOf(),
								 pRenderTargetContainer->GetDepthStencilView());
}

void Renderer::ActivatePipelineState(std::shared_ptr<PipelineState> pipelineState)
{
	ID3D11DeviceContext* pContext = m_D3D->m_Context.Get();
	if (m_ActivePipelineState == pipelineState)
	{
		return;
	}

	if (pipelineState == nullptr)
	{
		m_ActivePipelineState = nullptr;
		return;
	}

	if (m_ActivePipelineState == nullptr)
	{
		m_ActivePipelineState = pipelineState;
		pContext->RSSetState(m_ActivePipelineState->RasterizerState.Get());
		pContext->PSSetSamplers(0, 1, m_ActivePipelineState->SamplerState.GetAddressOf());
		pContext->VSSetSamplers(0, 1, m_ActivePipelineState->SamplerState.GetAddressOf());
		pContext->OMSetDepthStencilState(m_ActivePipelineState->DepthStencilState.Get(),
											m_ActivePipelineState->StencilRef);
		pContext->OMSetBlendState(m_ActivePipelineState->BlendState.Get(), NULL, 0xFFFFFFFF);
		pContext->IASetInputLayout(m_ActivePipelineState->VertexShader->GetInputLayout());
		pContext->VSSetShader(m_ActivePipelineState->VertexShader->GetShader(), nullptr, 0);
		if (m_ActivePipelineState->PixelShader == nullptr)
		{
			pContext->PSSetShader(nullptr, nullptr, 0);
		}
		else
		{
			pContext->PSSetShader(m_ActivePipelineState->PixelShader->GetShader(), nullptr, 0);
		}
		return;
	}

	if (m_ActivePipelineState->RasterizerState != pipelineState->RasterizerState)
	{
		pContext->RSSetState(pipelineState->RasterizerState.Get());
	}
	if (m_ActivePipelineState->SamplerState != pipelineState->SamplerState)
	{
		pContext->PSSetSamplers(0, 1, pipelineState->SamplerState.GetAddressOf());
		pContext->VSSetSamplers(0, 1, pipelineState->SamplerState.GetAddressOf());
	}
	if (m_ActivePipelineState->DepthStencilState != pipelineState->DepthStencilState)
	{
		pContext->OMSetDepthStencilState(pipelineState->DepthStencilState.Get(),
											pipelineState->StencilRef);
	}
	if (m_ActivePipelineState->BlendState != pipelineState->BlendState)
	{
		pContext->OMSetBlendState(pipelineState->BlendState.Get(), NULL, 0xFFFFFFFF);
	}
	if (m_ActivePipelineState->VertexShader != pipelineState->VertexShader)
	{
		pContext->IASetInputLayout(pipelineState->VertexShader->GetInputLayout());
		pContext->VSSetShader(pipelineState->VertexShader->GetShader(), nullptr, 0);
	}
	if (m_ActivePipelineState->PixelShader != pipelineState->PixelShader)
	{
		if (pipelineState->PixelShader == nullptr)
		{
			pContext->PSSetShader(nullptr, nullptr, 0);
		}
		else
		{
			pContext->PSSetShader(pipelineState->PixelShader->GetShader(), nullptr, 0);
		}
	}
	m_ActivePipelineState = pipelineState;
	
}

shared_ptr<PipelineState> Renderer::GetPipelineState(std::string name)
{
	auto iter = m_PipelineStatesMap.find(name);
	if (iter == m_PipelineStatesMap.end())
	{
		return nullptr;
	}
	return iter->second;
}

void Renderer::RegisterPipelineState(std::shared_ptr<PipelineState> pipelineState)
{
	m_PipelineStatesMap[pipelineState->Name] = pipelineState;
}

void Renderer::RenderUltralightView(UltralightView* pUltralightView)
{
	//TODO: Modify this to only happen once per render target.
	std::shared_ptr<PipelineState> render2D = GetPipelineState("2D");
	assert(render2D != nullptr);
	ActivatePipelineState(render2D);

	if (pUltralightView->GetView()->is_loading() == false)
	{
		pUltralightView->UpdateStorageTexture();
	}

	auto pos = pUltralightView->GetPosition();

	DirectX::XMMATRIX modelMatrix = DirectX::XMMatrixScaling(pUltralightView->GetWidth(), pUltralightView->GetHeight(), 1.0f) *
									DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
	//modelMatrix = DirectX::XMMatrixTranspose(modelMatrix);
	DirectX::XMStoreFloat4x4(&m_CB_PerDrawData_2D.m_Data.ModelMatrix, modelMatrix);

	if (!m_CB_PerDrawData_2D.ApplyChanges())
	{
		ErrorHandler::LogCriticalError("Failed to update the per draw data 2d constant buffer.");
	}

	ID3D11DeviceContext* pContext = m_D3D->m_Context.Get();

	ID3D11ShaderResourceView* pResourceView = pUltralightView->GetTextureSRV();
	pContext->PSSetShaderResources(0, 1, &pResourceView);

	pContext->IASetIndexBuffer(m_QuadMeshForUltralightView.GetIndexBuffer(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	ID3D11Buffer* pVertexBuffer = m_QuadMeshForUltralightView.GetVertexBuffer();
	const uint32_t stride = m_QuadMeshForUltralightView.GetStride();
	const uint32_t offset = 0;
	pContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
	pContext->DrawIndexed(m_QuadMeshForUltralightView.GetIndexCount(), 0, 0);
}

bool Renderer::RenderSceneInRenderTargetContainer(RenderTargetContainer* pRenderTargetContainer)
{
	Camera* pCamera = pRenderTargetContainer->GetCamera();
	if (pCamera == nullptr)
	{
		return true;
	}
	Scene* pScene = pCamera->GetScene();
	if (pScene == nullptr)
	{
		return true;
	}
	ID3D11DeviceContext* pContext = m_D3D->m_Context.Get();

	pContext->OMSetRenderTargets(1,
								 pRenderTargetContainer->GetRenderTargetViewAddressOf(),
								 pRenderTargetContainer->GetDepthStencilView());
	pContext->RSSetViewports(1, &pRenderTargetContainer->GetViewport());

	m_DeltaTime = pRenderTargetContainer->GetFrameDeltaTime();

	m_CB_PerFrameData_3D.m_Data.Projection = pCamera->GetProjectionMatrix();
	m_CB_PerFrameData_3D.m_Data.View = pCamera->GetViewMatrix();
	m_CB_PerFrameData_3D.m_Data.ViewProjection = m_CB_PerFrameData_3D.m_Data.View *
												 m_CB_PerFrameData_3D.m_Data.Projection;
	if (!m_CB_PerFrameData_3D.ApplyChanges())
	{
		return false;
	}

	ID3D11Buffer* constantBuffers[] = { m_CB_PerFrameData_3D.Get(), m_CB_PerDrawData_3D.Get() };
	int constantBuffersCount = ARRAYSIZE(constantBuffers);
	pContext->VSSetConstantBuffers(0, ARRAYSIZE(constantBuffers), constantBuffers);

	pContext->PSSetConstantBuffers(2, 1, m_CB_Material.GetAddressOf());

	for (auto& entity : pScene->Entities)
	{
		if (!RenderEntity(entity.get()))
		{
			return false;
		}
	}

	return true;
}

void Renderer::DrawSprite(Texture* pTexture, float x, float y, float z, float width, float height)
{
	std::shared_ptr<PipelineState> render2D = GetPipelineState("2D");
	assert(render2D != nullptr);
	ActivatePipelineState(render2D);

	DirectX::XMMATRIX modelMatrix = DirectX::XMMatrixScaling(width, height, 1.0f) *
									DirectX::XMMatrixTranslation(x, y, z);
	//modelMatrix = DirectX::XMMatrixTranspose(modelMatrix);
	DirectX::XMStoreFloat4x4(&m_CB_PerDrawData_2D.m_Data.ModelMatrix, modelMatrix);

	if (!m_CB_PerDrawData_2D.ApplyChanges())
	{
		ErrorHandler::LogCriticalError("Failed to update the per draw data 2d constant buffer.");
	}

	ID3D11DeviceContext* pContext = m_D3D->m_Context.Get();

	ID3D11ShaderResourceView* pResourceView = pTexture->GetTextureResourceView();
	pContext->PSSetShaderResources(0, 1, &pResourceView);

	//Just going to reuse the same quad used for Ultralight views here since it's same setup
	pContext->IASetIndexBuffer(m_QuadMeshForUltralightView.GetIndexBuffer(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
	ID3D11Buffer* pVertexBuffer = m_QuadMeshForUltralightView.GetVertexBuffer();
	const uint32_t stride = m_QuadMeshForUltralightView.GetStride();
	const uint32_t offset = 0;
	pContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
	pContext->DrawIndexed(m_QuadMeshForUltralightView.GetIndexCount(), 0, 0);
}

bool Renderer::RenderEntity(Entity* pEntity)
{
	if (pEntity->IsVisible() == false)
	{
		return true;
	}

	Model* pModel = pEntity->GetModel();
	if (pModel == nullptr)
	{
		return true;
	}

	ID3D11DeviceContext* pContext = m_D3D->m_Context.Get();

	pEntity->AdvanceAnimation(m_DeltaTime);
	pEntity->UpdatePose();
	EZGLTF::Skeleton skeleton = pModel->GetSkeleton();

	for (int i = 0; i < skeleton.m_Nodes.size(); i++)
	{
		auto& node = skeleton.m_Nodes[i];
		if (node.MeshIndices.size() == 0)
		{
			continue;
		}

		for(auto& meshIndex : node.MeshIndices)
		{
			auto& mesh = pModel->m_Meshes[meshIndex];
			
			for (auto primRef : mesh->m_PrimitiveRefs)
			{
				auto material = primRef->m_Material;
				auto prim = primRef->m_Primitive;
				if (material->PipelineState == nullptr)
				{
					//Note: This lookup is bad - this is just for demonstration purposes.
					shared_ptr<PipelineState> pPipelineState = GetPipelineState("3D"); //use default 3d pipeline state
					ActivatePipelineState(pPipelineState);
				}
				else
				{
					ActivatePipelineState(material->PipelineState);
				}

				auto& matData = m_CB_Material.m_Data;
				matData.BaseColor = material->Properties.BaseColor;
				matData.MetallicFactor = material->Properties.MetallicFactor;
				matData.RoughnessFactor = material->Properties.RoughnessFactor;

				if (prim->m_PrimaryPrimitiveAttributeSet->m_Normals.VertexCount() > 0)
				{
					matData.HasNormals = TRUE;
				}
				else
				{
					matData.HasNormals = FALSE;
				}

				if (prim->m_PrimaryPrimitiveAttributeSet->m_Colors.VertexCount() > 0)
				{
					matData.HasColoredVertices = TRUE;
				}
				else
				{
					matData.HasColoredVertices = FALSE;
				}

				auto& perDrawData = m_CB_PerDrawData_3D.m_Data;
				if (prim->m_PrimaryPrimitiveAttributeSet->m_JointIndices.VertexCount() > 0)
				{
					perDrawData.HasBones = TRUE; //for now ignoring skeleton
					EZGLTF::SkeletonPose pose = pEntity->GetPose();
					for (int i = 0; i < pose.BoneTransforms.size(); i++)
					{
						m_CB_BoneTransforms.m_Data.Bones[i] = pose.BoneTransforms[i];
					}
					if (m_CB_BoneTransforms.ApplyChanges())
					{
						pContext->VSSetConstantBuffers(3, 1, m_CB_BoneTransforms.GetAddressOf());
					}
					else
					{
						ErrorHandler::LogCriticalError("Failed to update bone data constant buffer.");
					}
				}
				else
				{
					perDrawData.HasBones = FALSE;
				}

				perDrawData.ModelMatrix = node.GlobalTransform * pEntity->GetMatrix();
				if (!m_CB_PerDrawData_3D.ApplyChanges())
				{
					return false;
				}

				if (material->BaseColorTexture != nullptr)
				{
					matData.HasBaseColorTexture = TRUE;
					pContext->PSSetShaderResources(0, 1, material->BaseColorTexture->GetTextureResourceViewAddress());
				}
				else
				{
					matData.HasBaseColorTexture = FALSE;
				}

				if (!m_CB_Material.ApplyChanges())
				{
					return false;
				}

				pContext->IASetPrimitiveTopology(prim->m_Topology);
				auto primAttributes = prim->m_PrimaryPrimitiveAttributeSet;
				pContext->IASetVertexBuffers(0,
											 primAttributes->GetVertexBufferCount(),
											 primAttributes->GetVertexBuffersVector().data(),
											 primAttributes->GetStridesVector().data(),
											 primAttributes->GetOffsetsVector().data());

				if (prim->m_IndexBuffer.IndexCount() == 0)
				{
					pContext->Draw(primAttributes->m_Positions.VertexCount(), 0);
				}
				else
				{
					pContext->IASetIndexBuffer(prim->m_IndexBuffer.Get(), DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
					pContext->DrawIndexed(prim->m_IndexBuffer.IndexCount(), 0, 0);
				}
			}
		}
	}

	return true;
}

Renderer::~Renderer()
{
	if (s_Instance == this)
	{
		s_Instance = nullptr;
	}
}

bool Renderer::BuildQuadMeshForUltralightView()
{
	vector<Vertex_2D> vertices =
	{
		Vertex_2D(0.0, 0.0, 0.0, 0, 0), //[0] TOPLEFT
		Vertex_2D(1.0, 0.0, 0.0, 1, 0), //[1] TOPRIGHT
		Vertex_2D(0.0, 1.0, 0.0, 0, 1), //[2] BOTTOMLEFT
		Vertex_2D(1.0, 1.0, 0.0, 1, 1), //[3] BOTTOMRIGHT
	};
	vector<uint32_t> indices = {
		2, 0, 1,
		2, 1, 3
	};

	if (!m_QuadMeshForUltralightView.Initialize(vertices, indices))
	{
		return false;
	}
	return true;
}