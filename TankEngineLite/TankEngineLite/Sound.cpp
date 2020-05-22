#include "Sound.h"

bool Sound::Initialize(std::string path)
{
	const auto pSoundSystem = SoundManager::GetInstance()->GetSystem();
	auto result = pSoundSystem->createSound(path.c_str(), FMOD_DEFAULT, nullptr, &m_pSound);

	// Create sound
	if (result != FMOD_OK)
	{
		LOGGER->Log<LOG_ERROR>("Failed to load sound ", path);
		return false;
	}

	LOGGER->Log<LOG_SUCCESS>("Loaded sound ", path);
	return true;
}

void Sound::Shutdown()
{
	m_pSound->release();
}
