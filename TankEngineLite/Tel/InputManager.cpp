#include "TelPCH.h"

#include "InputManager.h"
#include <SDL.h>

bool InputManager::ProcessInput()
{
	ZeroMemory(&m_XState, sizeof(XINPUT_STATE));
	DWORD dwResult = XInputGetState(0, &m_XState);

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
    
    // Controller input
	if (dwResult == ERROR_SUCCESS)
	{
        m_PadKeys[ControllerButton::DPAD_UP] = m_XState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
        m_PadKeys[ControllerButton::DPAD_DOWN] = m_XState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
        m_PadKeys[ControllerButton::DPAD_LEFT] = m_XState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
        m_PadKeys[ControllerButton::DPAD_RIGHT] = m_XState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
        
        m_PadKeys[ControllerButton::START] = m_XState.Gamepad.wButtons & XINPUT_GAMEPAD_START;
        m_PadKeys[ControllerButton::BACK] = m_XState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK;
        
        m_PadKeys[ControllerButton::LEFT_THUMB] = m_XState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB;
        m_PadKeys[ControllerButton::RIGHT_THUMB] = m_XState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;
        
        m_PadKeys[ControllerButton::LEFT_SHOULDER] = m_XState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
        m_PadKeys[ControllerButton::RIGHT_SHOULDER] = m_XState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
		
        m_PadKeys[ControllerButton::A] = m_XState.Gamepad.wButtons & XINPUT_GAMEPAD_A;
		m_PadKeys[ControllerButton::B] = m_XState.Gamepad.wButtons & XINPUT_GAMEPAD_B;
		m_PadKeys[ControllerButton::X] = m_XState.Gamepad.wButtons & XINPUT_GAMEPAD_X;
		m_PadKeys[ControllerButton::Y] = m_XState.Gamepad.wButtons & XINPUT_GAMEPAD_Y;
	}
	else
		std::cout << "not lmao" << std::endl;

	return false;
}

bool InputManager::IsPressed(ControllerButton button) const
{
	return m_PadKeys[button];
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
