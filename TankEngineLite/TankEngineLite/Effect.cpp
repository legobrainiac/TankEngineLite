#include "Effect.h"

#include "Renderer.h"

#include <codecvt>

Effect::Effect()
	: m_pEffect(nullptr)
{
}

bool Effect::Initialize(const std::string& effectPath)
{
	HRESULT hr = S_OK;
	ID3D10Blob* pErrorBlob = nullptr;
	ID3DX11Effect* pEffect;

	DWORD shaderFlags = 0;
#if defined( _DEBUG )
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

#pragma warning(push)
#pragma warning(disable: 4996)

	// Convert to wchar_t*
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring fullPath = converter.from_bytes(effectPath);

#pragma warning(pop)

	hr = D3DX11CompileEffectFromFile(
		fullPath.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		Renderer::GetInstance()->GetDirectX()->GetDevice(),
		&pEffect,
		&pErrorBlob);

	if (FAILED(hr))
	{
		if (pErrorBlob != nullptr)
		{
			auto* errors = (char*)pErrorBlob->GetBufferPointer();

			std::wstringstream ss;
			for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
				ss << errors[i];

			OutputDebugStringW(ss.str().c_str());
			DXRELEASE(pErrorBlob);

			std::wcout << ss.str() << std::endl;
		}
		else
		{
			std::wcout << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << fullPath << std::endl;
			return false;
		}
	}

	LOGGER->Log<LOG_SUCCESS>("Loaded effect " + effectPath);

	DXRELEASE(pErrorBlob);
	m_pEffect = pEffect;

	return true;
}

void Effect::CreateResources()
{
	if (!m_pInputLayout)
	{
		auto pRenderer = Renderer::GetInstance();
		auto pD3D = pRenderer->GetDirectX();
		auto pDevice = pD3D->GetDevice();

		// IL
		HRESULT result = S_OK;
		static const uint32_t numElements = 3;
		D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = 0;
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[1].SemanticName = "NORMAL";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[1].AlignedByteOffset = 12;
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[2].SemanticName = "TEXCOORD";
		vertexDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertexDesc[2].AlignedByteOffset = 24;
		vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		// Get Technique
		m_pTechnique = m_pEffect->GetTechniqueByIndex(0);

		// Create input layout
		D3DX11_PASS_DESC passDesc{};
		auto pPass = m_pTechnique->GetPassByIndex(0);
		pPass->GetDesc(&passDesc);

		result = pDevice->CreateInputLayout(vertexDesc, numElements, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_pInputLayout);
	}
}

void Effect::Shutdown()
{
	DXRELEASE(m_pInputLayout);
	DXRELEASE(m_pEffect);
}
