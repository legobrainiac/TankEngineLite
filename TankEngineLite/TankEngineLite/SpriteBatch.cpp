#include "pch.h"
#include "SpriteBatch.h"
#include "ResourceManager.h"

#include "Renderer.h"
#include "Texture.h"

#include "Effect.h"

void SpriteBatch::InitializeBatch(Texture* atlas, BatchMode mode)
{
	m_Mode = mode;

	// Device
	const auto pRenderer = Renderer::GetInstance();
	const auto pDevice = pRenderer->GetDirectX()->GetDevice();

	// Load sprite batch effect
	m_pEffect = ResourceManager::GetInstance()->Get<Effect>("SpriteBatch")->GetEffect();

	// Texture atlas setup
	m_Atlas = atlas;

	// Get the technique and variables
	m_pTechnique = m_pEffect->GetTechniqueByIndex(0);

	m_pTransfromMatrixV = m_pEffect->GetVariableByName("gTransform")->AsMatrix();
	m_pTextureSizeV = m_pEffect->GetVariableByName("gTextureSize")->AsVector();
	m_pCameraPos = m_pEffect->GetVariableByName("gCameraPos")->AsVector();
	m_pScale = m_pEffect->GetVariableByName("gScale")->AsScalar();
	m_pTextureSRV = m_pEffect->GetVariableByName("gSpriteTexture")->AsShaderResource();

	// Input layout
	D3D11_INPUT_ELEMENT_DESC vertexDesc[4]{};

	// TODO(tomas): input layout builder?
	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "TANGENT";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 16;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 32;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "COLOR";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 48;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// Pass description
	D3DX11_PASS_DESC passDesc{};
	m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	
	const auto result = pDevice->CreateInputLayout(vertexDesc, 4, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_pInputLayout);
	if (FAILED(result))
		Logger::GetInstance()->Log<LOG_ERROR>("Failed to create input layout for sprite batch, halp");

	// TODO(tomas): remove this, replace with proper matrix for a 2D camera
	const auto [screenW, screenH] = pRenderer->GetScreenSize();

	const float scaleX = (screenW > 0) ? 2.0f / screenW : 0;
	const float scaleY = (screenH > 0) ? 2.0f / screenH : 0;

	m_Transform._11 = scaleX;
	m_Transform._12 = 0;
	m_Transform._13 = 0;
	m_Transform._14 = 0;
	m_Transform._21 = 0;
	m_Transform._22 = -scaleY;
	m_Transform._23 = 0;
	m_Transform._24 = 0;
	m_Transform._31 = 0;
	m_Transform._32 = 0;
	m_Transform._33 = 1;
	m_Transform._34 = 0;
	m_Transform._41 = -1;
	m_Transform._42 = 1;
	m_Transform._43 = 0;
	m_Transform._44 = 1;

	Logger::GetInstance()->Log<LOG_SUCCESS>(((m_Mode == BatchMode::BATCHMODE_DYNAMIC) ? std::string("Dynamic") : "Static") + " sprite batch created");
}

void SpriteBatch::Destroy()
{
	DXRELEASE(m_pTechnique);
	DXRELEASE(m_pVertexBuffer);

	DXRELEASE(m_pTransfromMatrixV);
	DXRELEASE(m_pTextureSizeV);
	DXRELEASE(m_pTextureSRV);

	DXRELEASE(m_pInputLayout);
	m_Batch.clear();
}

void SpriteBatch::PushSprite(
	XMFLOAT4 atlasTransform,
	XMFLOAT3 position,
	float rotation,
	XMFLOAT2 scale,
	XMFLOAT2 pivot,
	XMFLOAT4 colour)
{
	if (!m_IsRendering)
		return;

	BatchItem item{};
	item.atlasTrasform = atlasTransform;
	item.transformData0 = XMFLOAT4{ position.x, position.y, position.z, rotation };
	item.transformData1 = XMFLOAT4{ pivot.x, pivot.y, scale.x, scale.y };
	item.color = colour;

	m_Batch.push_back(item);

	m_Dirty = true;
}

void SpriteBatch::Render()
{
	if (!m_Atlas || !m_IsRendering || (m_Mode == BatchMode::BATCHMODE_DYNAMIC && m_Batch.empty()))
		return;

	// Device stuff
	const auto pRenderer = Renderer::GetInstance();
	const auto pDeviceContext = pRenderer->GetDirectX()->GetDeviceContext();

	if (m_Mode == BatchMode::BATCHMODE_DYNAMIC || m_Dirty)
		UpdateBuffer();

	// Setup render pipeline
	pDeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	uint32_t stride = sizeof(BatchItem);
	uint32_t offset = 0;

	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	// Set texture
	m_pTextureSRV->SetResource(m_Atlas->GetTexture());

	// Set shader globals
	auto texSize = m_Atlas->GetTextureSize();
	m_pTextureSizeV->SetFloatVector(reinterpret_cast<float*>(&texSize));
	m_pCameraPos->SetFloatVector(reinterpret_cast<float*>(&m_CameraPos));
	m_pScale->SetFloat(m_Scale);

	//Set Transform
	m_pTransfromMatrixV->SetMatrix(reinterpret_cast<float*>(&m_Transform));

	// Draw the batch
	D3DX11_TECHNIQUE_DESC techDesc;
	m_pTechnique->GetDesc(&techDesc);
	for (unsigned int j = 0; j < techDesc.Passes; ++j)
	{
		m_pTechnique->GetPassByIndex(j)->Apply(0, pDeviceContext);
		pDeviceContext->Draw(m_BatchSize, 0);
	}

	// Clear it as setup for next frame
	m_Batch.clear();
}

void SpriteBatch::ImGuiDebug()
{
	ImGui::Image((void*)m_Atlas->GetTexture(), ImVec2(256, 256));

	ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "BatchMode::m_Atlas::size::x = ");
	ImGui::SameLine();
	ImGui::Text(std::to_string((int)m_Atlas->GetTextureSize().x).c_str());

	ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "BatchMode::m_Atlas::size::y = ");
	ImGui::SameLine();
	ImGui::Text(std::to_string((int)m_Atlas->GetTextureSize().y).c_str());

	ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "BatchMode::m_Mode = ");
	ImGui::SameLine();
	ImGui::Text((m_Mode == BatchMode::BATCHMODE_DYNAMIC) ? "Dynamic" : "Static");

	ImGui::TextColored(ImVec4(1.f, 1.f, 0.f, 1.f), "BatchMode::m_BatchSize = ");
	ImGui::SameLine();
	ImGui::Text(std::to_string(m_BatchSize).c_str());
}

void SpriteBatch::UpdateBuffer()
{
	// Device stuff
	const auto pRenderer = Renderer::GetInstance();
	const auto pDevice = pRenderer->GetDirectX()->GetDevice();
	const auto pDeviceContext = pRenderer->GetDirectX()->GetDeviceContext();

	// Reallocate, maybe don't do this every fucking frame
	DXRELEASE(m_pVertexBuffer);

	m_BatchSize = (int)m_Batch.size();

	D3D11_BUFFER_DESC desc;

	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = sizeof(BatchItem) * (uint32_t)m_Batch.size();
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	const auto result = pDevice->CreateBuffer(&desc, nullptr, &m_pVertexBuffer);
	if (FAILED(result))
		return;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	pDeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedResource);
	memcpy(mappedResource.pData, &m_Batch[0], sizeof(BatchItem) * m_BatchSize);
	pDeviceContext->Unmap(m_pVertexBuffer, 0);
	
	m_Dirty = false;
}
