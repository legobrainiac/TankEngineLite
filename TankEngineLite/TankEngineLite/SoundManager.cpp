#include "pch.h"
#include "SoundManager.h"
#include "Logger.h"
#include "Profiler.h"

#include <thread>

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

void SoundManager::Update([[maybe_unused]] float dt)
{
	Profiler::GetInstance()->BeginSubSession<SessionId::SESSION_UPDATE_SOUND>();
	m_pFMOD->update();
	Profiler::GetInstance()->EndSubSession();
}

FMOD::Channel* SoundManager::PlaySound(FMOD::Sound* pSound) const
{
	FMOD::Channel* pChannel = nullptr;
	auto result = m_pFMOD->playSound(pSound, m_pChannels, false, &pChannel);

	if (result != FMOD_OK)
	{
		LOGGER->Log<LOG_ERROR>("play back of sound failed, my dude");
		return nullptr;
	}
	
	return pChannel;
}
