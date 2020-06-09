#ifndef SOUND_H
#define SOUND_H

#include "ResourceManager.h"
#include "SoundManager.h"

#include <string>

class Sound :
	public IResource
{
public:
	Sound()
		: m_pSound(nullptr)
	{
	}

	Sound(const Sound&) = delete;
	Sound& operator=(const Sound&) = delete;

	bool Initialize(std::string path);
	void Shutdown();

	[[nodiscard]] constexpr auto GetSound() const noexcept -> FMOD::Sound* { return m_pSound; }

private:
	FMOD::Sound* m_pSound;
};

#endif

