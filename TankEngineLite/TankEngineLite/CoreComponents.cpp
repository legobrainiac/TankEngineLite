#include "CoreComponents.h"

#include "SpriteBatch.h"
#include "Renderer.h"

#include "Model.h"
#include "Effect.h"

// Render Component
void SpriteRenderComponent::Render() const
{
	// Only render if all dependencies are available
	if (!m_MeetsRequirements || !m_Enabled)
		return;

	// If using a custom render callback, use that one instead of the normal one
	if (m_bShouldCustomRender)
	{
		m_CustomRenderFunction(m_pOwner);
		return;
	}

	m_pSpriteBatch->PushSprite(
		m_AtlasTransform,
		m_pTransform->position,
		m_pTransform->rotation,
		m_pTransform->scale,
		m_Pivot,
		{ 1.f, 1.f, 1.f, 1.f }
	);
}

inline void SpriteRenderComponent::SetCustomRenderFunction(const CustomRenderFunction& crf)
{
	m_CustomRenderFunction = crf;
	m_bShouldCustomRender = true;
}

SpriteRenderComponent::SpriteRenderComponent(ECS::Entity* pE)
	: ECS::EntityComponent(pE)
{
	m_pSpriteBatch = nullptr;
	m_MeetsRequirements = false;
	m_bShouldCustomRender = false;
	m_pTransform = pE->GetComponent<TransformComponent2D>();

	if (m_pTransform != nullptr)
		m_MeetsRequirements = true;
}

void CameraComponent::GenerateViewMatrix()
{
	auto V_up = XMVectorSet(0.f, 1.f, 0.f, 0.f);
	auto V_lookAt = XMVectorSet(0.f, 0.f, 1.f, 0.f);

	auto rotationMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&m_pEntityTransform->rotationQ));
	rotationMatrix = XMMatrixTranspose(rotationMatrix);

	V_lookAt = XMVector3TransformCoord(V_lookAt, rotationMatrix);
	V_up = XMVector3TransformCoord(V_up, rotationMatrix);

	const auto transformPosition = XMLoadFloat3(&m_pEntityTransform->position);

	V_lookAt = transformPosition + V_lookAt;
	XMStoreFloat4x4(&m_ViewMatrix, XMMatrixLookAtLH(transformPosition, V_lookAt, V_up));
}

ModelRenderComponent::ModelRenderComponent(ECS::Entity* pE)
	: ECS::EntityComponent(pE)
{
	m_pTransform = pE->GetComponent<TransformComponent>();

	if (m_pTransform)
		m_MeetsRequirements = true;
}

void ModelRenderComponent::Initialize(Model* pModel, Texture* pTexture)
{
	m_pModel = pModel;
	m_pTexture = pTexture;
	m_pEffect = ResourceManager::GetInstance()->Get<Effect>("FlatColor");
	m_pEffect->CreateResources();

	auto pEffect = m_pEffect->GetEffect();

	m_pWorld = pEffect->GetVariableByName("gWorld")->AsMatrix();
	m_pWorldViewProjection = pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	m_pDiffuseMap = pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
}

void ModelRenderComponent::Render()
{
	auto pRenderer = Renderer::GetInstance();
	auto pD3D = pRenderer->GetDirectX();
	auto pDeviceContext = pD3D->GetDeviceContext();

	//////////////////////////////////////////////////////////////////////////
	// Model Render
	UINT stride = sizeof(TexVertex);
	UINT offset = 0;

	auto pVB = m_pModel->GetVertexBuffer();
	auto pIB = m_pModel->GetIndexBuffer();

	// Set vertex buffer
	pDeviceContext->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);

	// Set the index buffer
	pDeviceContext->IASetIndexBuffer(pIB, DXGI_FORMAT_R32_UINT, 0);

	// Set the primitive topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//////////////////////////////////////////////////////////////////////////
	// Shader render
	pDeviceContext->IASetInputLayout(m_pEffect->GetInputLayout());

	XMFLOAT4X4 world = m_pTransform->GetWorldMatrix();
	auto worldMat = XMLoadFloat4x4(&world);

	XMFLOAT4X4 camInv = pD3D->GetCameraViewMatrix();
	auto cameraInverse = XMMatrixInverse(nullptr, XMLoadFloat4x4(&camInv));

	XMFLOAT4X4 proj = pD3D->GetProjectionMatrix();
	auto projectionMat = XMLoadFloat4x4(&proj);
	
	XMFLOAT4X4 worldViewMatrix{};
	XMStoreFloat4x4(&worldViewMatrix, worldMat * cameraInverse * projectionMat);

	m_pWorld->SetMatrix(reinterpret_cast<float*>(&world));
	m_pWorldViewProjection->SetMatrix(reinterpret_cast<float*>(&worldViewMatrix));
	m_pDiffuseMap->SetResource(m_pTexture->GetTexture());

	// Draw
	D3DX11_TECHNIQUE_DESC techDesc;
	m_pEffect->GetTechnique()->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_pModel->GetIndexCount(), 0, 0);
	}
}

