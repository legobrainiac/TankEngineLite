#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

// Disable annoying fmod warnings
#pragma warning(push)
#pragma warning(disable: 4505 26812)
#include "fmod.hpp"
#include "fmod_errors.h"
#pragma warning(pop)

#include "Singleton.h"

#define SOUND SoundManager::GetInstance()

//////////////////////////////////////////////////////////////////////////
// Class: SoundManager
// Description: Singleton wrapper around FMOD::System's functionalities 
class SoundManager
	: public Singleton<SoundManager>
{
public:
	SoundManager();
	
	//////////////////////////////////////////////////////////////////////////
	// Method:    Init
	// FullName:  SoundManager::Init
	// Access:    public 
	// Returns:   bool
	// Initialize FMOD
	bool Init();

	//////////////////////////////////////////////////////////////////////////
	// Method:    Destroy
	// FullName:  SoundManager::Destroy
	// Access:    public 
	// Returns:   void
	// Description: Cleanup FMOD and all it's resources
	void Destroy();

	//////////////////////////////////////////////////////////////////////////
	// Method:    Update
	// FullName:  SoundManager::Update
	// Access:    public 
	// Returns:   void
	// Description: Update FMOD, call this one per frame
	// Parameter: float dt
	void Update(float dt);

	//////////////////////////////////////////////////////////////////////////
	// Method:    GetSystem
	// FullName:  SoundManager::GetSystem
	// Access:    public 
	// Returns:   FMOD::System*
	// Qualifier: noexcept
	inline FMOD::System* GetSystem() noexcept { return m_pFMOD; }

	//////////////////////////////////////////////////////////////////////////
	// Method:    PlaySound
	// FullName:  SoundManager::PlaySound
	// Access:    public 
	// Returns:   FMOD::Channel*
	// Qualifier: const
	// Description: Play sound without any directionality or world position
	// Parameter: FMOD::Sound * pSound
	FMOD::Channel* Play(FMOD::Sound* pSound) const;

private:
	FMOD::System* m_pFMOD = nullptr;
	FMOD::ChannelGroup* m_pChannels = nullptr;
};

#endif // !SOUND_MANAGER_H
