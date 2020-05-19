#include "Effect.h"

#include "Renderer.h"

#include <codecvt>

Effect::Effect()
	: m_pEffect(nullptr)
{
}

bool Effect::Initialize(std::string effectPath)
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

void Effect::Shutdown()
{
	DXRELEASE(m_pEffect);
}
