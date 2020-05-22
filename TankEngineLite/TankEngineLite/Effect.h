#ifndef EFFECT_H
#define EFFECT_H

#include "ResourceManager.h"
#include "d3dx11effect.h"
#include "D3D.h"

#include "SoundManager.h"
#include "MemoryTracker.h"
#include "Logger.h"

#include <vector>

class Effect
	: public IResource
{
public:
	Effect();
	Effect(const Effect&) = delete;
	Effect& operator=(const Effect&) = delete;

	bool Initialize(std::string effectPath);
	void Shutdown();

	[[nodiscard]] constexpr auto GetEffect() const noexcept -> ID3DX11Effect* { return m_pEffect; }

private:
	ID3DX11Effect* m_pEffect;
};

#endif // !EFFECT_T
