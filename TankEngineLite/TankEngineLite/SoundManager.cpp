#include "pch.h"
#include "SoundManager.h"
#include "Logger.h"

SoundManager::SoundManager()
{
}

bool SoundManager::Init()
{
	LOGGER->Log<LOG_INFO>("Initializing sound system");
	auto result = FMOD::System_Create(&m_pFMOD);
	
	// Create sound system
	if (result != FMOD_OK)
	{
		LOGGER->Log<LOG_ERROR>("Failed to create sound system");
		return false;
	}

	// Initialize sound system
	result = m_pFMOD->init(512, FMOD_INIT_NORMAL, nullptr);
	if (result != FMOD_OK)
	{
		LOGGER->Log<LOG_ERROR>("Failed to create sound system");
		return false;
	}

	// Channels
	result = m_pFMOD->createChannelGroup("mainGameSounds", &m_pChannels);
	if (result != FMOD_OK)
	{
		LOGGER->Log<LOG_ERROR>("Failed to create channel group >> mainGameSounds");
		return false;
	}

	return true;
}

void SoundManager::Destroy()
{
	m_pChannels->release();
	m_pFMOD->release();
}

void SoundManager::PlaySound() const
{
}
