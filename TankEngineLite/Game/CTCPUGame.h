#ifndef CTCPU_GAME_H
#define CTCPU_GAME_H

#include <Tel.h>

#include "ctcpu.h"

class CTCPUGame 
	: public IGame
{
public:
	void Initialize() override;
	void Load(ResourceManager* pResourceManager, TEngineRunner* pEngine) override;
	void Update(float dt, InputManager* pInputManager) override;
	void Render(Renderer* pRenderer) override;
	void Shutdown() override;

private:
	cpu c;
	bool m_RunCpu;
	float m_CpuClockSpeed = 10.f;
	float m_CurrentTick = 0.f;
};

#endif // !CTCPU_GAME_H
