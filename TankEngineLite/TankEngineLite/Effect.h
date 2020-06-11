#ifndef EFFECT_H
#define EFFECT_H

#include "ResourceManager.h"
#include "d3dx11effect.h"
#include "D3D.h"

#include "SoundManager.h"
#include "MemoryTracker.h"
#include "Logger.h"

#include <vector>
#include <stack>
#include <string>

class Effect
	: public IResource
{
public:
	Effect();
	Effect(const Effect&) = delete;
	Effect& operator=(const Effect&) = delete;

	bool Initialize(std::string effectPath);
	void CreateResources();
	void Shutdown();

	[[nodiscard]] constexpr auto GetEffect() const noexcept -> ID3DX11Effect* { return m_pEffect; }
	[[nodiscard]] constexpr auto GetTechnique() const noexcept -> ID3DX11EffectTechnique* { return m_pTechnique; }
	[[nodiscard]] constexpr auto GetInputLayout() const noexcept -> ID3D11InputLayout* { return m_pInputLayout; }
	void Register(IUnknown* pIUK) { m_Cleanup.push(pIUK); }

private:
	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pTechnique = nullptr;
	ID3D11InputLayout* m_pInputLayout = nullptr;
	std::stack<IUnknown*> m_Cleanup;
};

#endif // !EFFECT_T
