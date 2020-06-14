#include "pch.h"
#include "Model.h"
#include "Renderer.h"

#include <fstream>

#include <string>

Model::Model()
	: m_pVertexBuffer(nullptr)
	, m_pIndexBuffer(nullptr)
	, m_VertexCount{}
	, m_IndexCount{}
	, m_LoadStatus(false)
{
	//m_LoadStatus = InitializeBuffers(pDevice, modelFilepath, modelName, uvChannel) && LoadTexture(pDevice, textureResourceName.c_str());
}

bool Model::Initialize(std::string modelDescriptorPath)
{
	const auto pDevice = Renderer::GetInstance()->GetDirectX()->GetDevice();

	// Read in the tank engine model descriptor, TEMD
	std::ifstream stream(modelDescriptorPath);
	std::string modelName;
	std::string filePath;
	std::string uvChannelName;

	if (stream.is_open())
	{
		std::getline(stream, modelName);
		std::getline(stream, filePath);
		std::getline(stream, uvChannelName);
	}

	std::string modelPath = modelDescriptorPath.substr(0, modelDescriptorPath.find_last_of('/') + 1) + filePath;
	return InitializeBuffers(pDevice, modelPath, modelName, uvChannelName);
}

void Model::Shutdown()
{
	RELEASE(m_pIndexBuffer);
	RELEASE(m_pVertexBuffer);
}

bool Model::InitializeBuffers(ID3D11Device* pDevice, std::string modelFilepath, std::string modelName, std::string uvChannel)
{
	HRESULT result;
	TexVertex* pVertices{ nullptr };
	ULONG* pIndices{ nullptr };

	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_BUFFER_DESC indexBufferDesc;

	D3D11_SUBRESOURCE_DATA vertexData;
	D3D11_SUBRESOURCE_DATA indexData;

	int errorCounter{ 0 };
	Model::LoadFBX(modelFilepath.c_str(), modelName.c_str(), m_VertexCount, m_IndexCount, pVertices, pIndices, errorCounter, (LPCSTR)uvChannel.c_str());

	// Setup the description of the static vertex buffer
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(TexVertex) * m_VertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Set sub resource pointer
	vertexData.pSysMem = pVertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	result = pDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &m_pVertexBuffer);
	ASSERT_RESULT();

	// Setup the description of the static index buffer
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(ULONG) * m_IndexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	
	// Set sub resource pointer
	indexData.pSysMem = pIndices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	result = pDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer);
	ASSERT_RESULT();

	delete[] pVertices;
	delete[] pIndices;

	return !(errorCounter > 0);
}