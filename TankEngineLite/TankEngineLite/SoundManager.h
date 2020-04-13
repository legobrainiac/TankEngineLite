#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

// Disable annoying fmod warnings
#pragma warning(push)
#pragma warning(disable: 4505 26812)
#include "fmod.hpp"
#include "fmod_errors.h"
#pragma warning(pop)

#include "Singleton.h"

class SoundManager
	: public Singleton<SoundManager>
{
public:
	SoundManager();
	bool Init();
	void Destroy();

private:
	FMOD::System* m_pFMOD = nullptr;
	FMOD::ChannelGroup* m_pChannels = nullptr;
};

#endif // !SOUND_MANAGER_H
