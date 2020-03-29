#include "TelPCH.h"

#include "InputManager.h"

#include <SDL.h>

bool InputManager::ProcessInput()
{
    // Swap active buffers
    m_ActiveBuffer = (m_ActiveBuffer == 0) ? 1 : 0;
    
    // Clear key buffer
    for (int i = 0; i < 512; ++i)
        m_Keys[m_ActiveBuffer][i] = false;
    
    // Clear pad buffer
    for(int j = 0; j < 4; ++j)
    {
        for (int i = 0; i < 18; ++i)
            m_PadKeys[m_ActiveBuffer][j][i] = false;
    }
    
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
    
    for(uint32_t cId = 0; cId < 4; ++cId)
    {
        // Read controller state
        XINPUT_STATE xState{};
        ZeroMemory(&xState, sizeof(XINPUT_STATE));
        DWORD dwResult = XInputGetState(cId, &xState);
        
        // Check if it's connected and log it
        if(m_ControllerConnected[cId] && dwResult != ERROR_SUCCESS)
        {
            m_ControllerConnected[cId] = false;
            LOGINFO("Controller disconnected [ cid = " << cId << " ]" << std::endl);
            continue;
        }
        else if(!m_ControllerConnected[cId] && dwResult == ERROR_SUCCESS)
        {
            m_ControllerConnected[cId] = true;
            LOGINFO("Controller connected [ cid = " << cId << " ]" << std::endl);
        }
        
        if(dwResult != ERROR_SUCCESS)
            continue;
        
        // Input management
        m_PadKeys[m_ActiveBuffer][cId][ControllerButton::DPAD_UP] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
        m_PadKeys[m_ActiveBuffer][cId][ControllerButton::DPAD_DOWN] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
        m_PadKeys[m_ActiveBuffer][cId][ControllerButton::DPAD_LEFT] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
        m_PadKeys[m_ActiveBuffer][cId][ControllerButton::DPAD_RIGHT] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
        
        m_PadKeys[m_ActiveBuffer][cId][ControllerButton::START] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_START;
        m_PadKeys[m_ActiveBuffer][cId][ControllerButton::BACK] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK;
        
        m_PadKeys[m_ActiveBuffer][cId][ControllerButton::LEFT_THUMB] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB;
        m_PadKeys[m_ActiveBuffer][cId][ControllerButton::RIGHT_THUMB] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;
        
        m_PadKeys[m_ActiveBuffer][cId][ControllerButton::LEFT_SHOULDER] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
        m_PadKeys[m_ActiveBuffer][cId][ControllerButton::RIGHT_SHOULDER] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
        
        m_PadKeys[m_ActiveBuffer][cId][ControllerButton::A] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_A;
        m_PadKeys[m_ActiveBuffer][cId][ControllerButton::B] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_B;
        m_PadKeys[m_ActiveBuffer][cId][ControllerButton::X] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_X;
        m_PadKeys[m_ActiveBuffer][cId][ControllerButton::Y] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_Y;    
    }
    
    // Process Action mappings
    for(auto am : m_ActionMappings)
        ValidateActionMapping(am);
    
	return false;
}

void InputManager::ValidateActionMapping(ActionMapping& am)
{
    uint32_t inactiveBuffer = uint32_t(!(bool)m_ActiveBuffer);
    
    // TODO(tomas): not fully implemented
    
    if(am.controllerId == (uint32_t)Player::PLAYER_INVALID) 
    { // If this is a keyboard action map
        switch(am.actionType)
        {
            case ActionType::PRESSED:
            if(m_Keys[m_ActiveBuffer][am.action] && !m_Keys[inactiveBuffer][am.action])
                am.processor();
            break;
            case ActionType::RELEASED:
            if(m_Keys[inactiveBuffer][am.action] && !m_Keys[m_ActiveBuffer][am.action])
                am.processor();
            break;
        }
    }
    else
    { // If this is a controller action map
        switch(am.actionType)
        {
            case ActionType::PRESSED:
            if(
                   m_PadKeys[(uint32_t)am.controllerId][m_ActiveBuffer][am.action] &&
                   !m_PadKeys[(uint32_t)am.controllerId][inactiveBuffer][am.action]
                   )
            {
                am.processor();
            }
            break;
            case ActionType::RELEASED:
            if(
                   m_PadKeys[(uint32_t)am.controllerId][inactiveBuffer][am.action] &&
                   !m_PadKeys[(uint32_t)am.controllerId][m_ActiveBuffer][am.action]
                   )
            {   
                am.processor();
            }
            break;
        }
    }
}

bool InputManager::IsPressed(ControllerButton button, uint32_t controllerId) const
{
    if(m_ControllerConnected[controllerId])
        return m_PadKeys[m_ActiveBuffer][controllerId][button];
    else
    {
        LOGINFO("Attempt of input on disconnected controller [ cid = " << controllerId << " ]" << std::endl);
        return false;
    }
}

void InputManager::KeyDown(SDL_Scancode key)
{
	m_Keys[m_ActiveBuffer][key] = true;
}

void InputManager::KeyUp(SDL_Scancode key)
{
	m_Keys[m_ActiveBuffer][key] = false;
}

bool InputManager::IsKeyDown(SDL_Scancode key)
{
	return m_Keys[m_ActiveBuffer][key];
}

std::tuple<int, int, Uint32> InputManager::GetMouseState()
{
	int x, y;
	Uint32 mouseState = SDL_GetRelativeMouseState(&x, &y);
	return std::tuple<int, int, Uint32>(x, y, mouseState);
}

