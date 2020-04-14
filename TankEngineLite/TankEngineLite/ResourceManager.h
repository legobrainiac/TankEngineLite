#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "Singleton.h"
#include "d3dx11effect.h"

#include <unordered_map>

#include "SoundManager.h"

class Texture;
class Font;

class ResourceManager final 
	: public Singleton<ResourceManager>
{
public:
	void Init(const std::string& data, const std::wstring& dataW);
	void Destroy();

	// TODO(tomas): abstract this whole thing more? Load<Effect>, Load<Texture>

	//////////////////////////////////////////////////////////////////////////
	// Textures
	Texture* LoadTexture(const std::string& file, const std::string& name);
	Texture* GetTexture(const std::string& name) const;

	//////////////////////////////////////////////////////////////////////////
	// Effects
	ID3DX11Effect* LoadEffect(const std::wstring& file, const std::string& name);
	ID3DX11Effect* GetEffect(const std::string& name) const;

	//////////////////////////////////////////////////////////////////////////
	// Sound
	FMOD::Sound* LoadSound(const std::string& file, const std::string& name);
	FMOD::Sound* GetSound(const std::string& name) const;

private:
	friend class Singleton<ResourceManager>;
	ResourceManager() = default;

	std::string m_DataPath;
	std::wstring m_DataPathW;

	// Resources
	std::unordered_map<std::string, Texture*> m_pTextures;
	std::unordered_map<std::string, ID3DX11Effect*> m_pEffects;
	std::unordered_map<std::string, FMOD::Sound*> m_pSounds;
};
#endif // !RESOURCE_MANAGER_H
