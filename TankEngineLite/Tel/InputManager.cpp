#include "TelPCH.h"

#include "InputManager.h"
#include <SDL.h>

bool InputManager::ProcessInput()
{
	ZeroMemory(&m_CurrentState, sizeof(XINPUT_STATE));
	XInputGetState(0, &m_CurrentState);

	SDL_Event e;
	while (SDL_PollEvent(&e)) 
    {
		if (e.type == SDL_QUIT) 
			return true;
		if (e.type == SDL_KEYDOWN) {}
		if (e.type == SDL_MOUSEBUTTONDOWN) {}
        
        // Keyboard input
        if(e.type == SDL_KEYUP)
            KeyUp(e.key.keysym.scancode);
        
        if(e.type == SDL_KEYDOWN)
            KeyDown(e.key.keysym.scancode);
	}

	return false;
}

bool InputManager::IsPressed(ControllerButton button) const
{
	switch (button)
	{
        case ControllerButton::ButtonA:
		return m_CurrentState.Gamepad.wButtons & XINPUT_GAMEPAD_A;
        case ControllerButton::ButtonB:
		return m_CurrentState.Gamepad.wButtons & XINPUT_GAMEPAD_B;
        case ControllerButton::ButtonX:
		return m_CurrentState.Gamepad.wButtons & XINPUT_GAMEPAD_X;
        case ControllerButton::ButtonY:
		return m_CurrentState.Gamepad.wButtons & XINPUT_GAMEPAD_Y;
        default: return false;
	}
}

void InputManager::KeyDown(SDL_Scancode key)
{
	m_Keys[key] = true;
}

void InputManager::KeyUp(SDL_Scancode key)
{
	m_Keys[key] = false;
}

bool InputManager::IsKeyDown(SDL_Scancode key)
{
	return m_Keys[key];
}

std::tuple<int, int, Uint32> InputManager::GetMouseState()
{
	int x, y;
	Uint32 mouseState = SDL_GetRelativeMouseState(&x, &y);
	return std::tuple<int, int, Uint32>(x, y, mouseState);
}
