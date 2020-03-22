#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "SDL.h"
#include "Singleton.h"

#include <tuple>
#include <XInput.h>

enum ControllerButton
{
    DPAD_UP,
    DPAD_DOWN,
    DPAD_LEFT,
    DPAD_RIGHT,
    START,
    BACK,
    LEFT_THUMB,
    RIGHT_THUMB,
    LEFT_SHOULDER,
    RIGHT_SHOULDER,
    A,
    B,
    X,
    Y
};

class InputManager final : public Singleton<InputManager>
{
public:
    InputManager()
    {
        for (int i = 0; i < 512; ++i)
            m_Keys[i] = false;
    }
    
	void KeyDown(SDL_Scancode key);
	void KeyUp(SDL_Scancode key);
	bool IsKeyDown(SDL_Scancode key);
    
	bool ProcessInput();
	bool IsPressed(ControllerButton button) const;
	std::tuple<int, int, Uint32> GetMouseState();
    
private:
	XINPUT_STATE m_XState{};
	bool m_Keys[512];
    bool m_PadKeys[18];
};

#endif // !INPUT_MANAGER_H