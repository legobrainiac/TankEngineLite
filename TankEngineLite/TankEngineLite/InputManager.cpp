#include "pch.h"
#include "InputManager.h"

#include "imgui_impl_sdl.h"

#include <vector>
#include <XInput.h>
#include <algorithm>

#include "Profiler.h"

bool InputManager::ProcessInput()
{
	// Reset the signal buffer
	m_SBIndex = 0;

	Profiler::GetInstance()->BeginSubSession<SESSION_SDL_POOL_EVENT>();
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		ImGui_ImplSDL2_ProcessEvent(&e);

		if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard)
			continue;

		if (e.type == SDL_QUIT)
		{
			Profiler::GetInstance()->EndSubSession();
			return true;
		}
		if (e.type == SDL_KEYDOWN) {}
		if (e.type == SDL_MOUSEBUTTONDOWN) {}

		// Keyboard input
		if (e.type == SDL_KEYUP)
			KeyUp(e.key.keysym.scancode);

		if (e.type == SDL_KEYDOWN)
			KeyDown(e.key.keysym.scancode);
	}

	Profiler::GetInstance()->EndSubSession();

	Profiler::GetInstance()->BeginSubSession<SESSION_XINPUT_UPDATE>();
	for (uint32_t cId = 0; cId < 4; ++cId)
	{
		if(!m_ControllerConnected[cId])
			continue;

		XINPUT_STATE xState{};
		ZeroMemory(&xState, sizeof(XINPUT_STATE));
		XInputGetState(cId, &xState);

		// Input management
		m_PadKeys[cId][ControllerButton::DPAD_UP] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
		m_PadKeys[cId][ControllerButton::DPAD_DOWN] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
		m_PadKeys[cId][ControllerButton::DPAD_LEFT] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
		m_PadKeys[cId][ControllerButton::DPAD_RIGHT] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

		m_PadKeys[cId][ControllerButton::START] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_START;
		m_PadKeys[cId][ControllerButton::BACK] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_BACK;

		m_PadKeys[cId][ControllerButton::LEFT_THUMB] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB;
		m_PadKeys[cId][ControllerButton::RIGHT_THUMB] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB;

		m_PadKeys[cId][ControllerButton::LEFT_SHOULDER] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
		m_PadKeys[cId][ControllerButton::RIGHT_SHOULDER] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;

		m_PadKeys[cId][ControllerButton::A] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_A;
		m_PadKeys[cId][ControllerButton::B] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_B;
		m_PadKeys[cId][ControllerButton::X] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_X;
		m_PadKeys[cId][ControllerButton::Y] = xState.Gamepad.wButtons & XINPUT_GAMEPAD_Y;
	}

	Profiler::GetInstance()->EndSubSession();

	Profiler::GetInstance()->BeginSubSession<SESSION_ACTIONMAPPING_UPDATE>();
	// Process Action mappings
	for (const auto& amType : m_ActionMappings)
	{
		for (auto am : amType.second)
			ValidateActionMapping(am);
	}
	
	Profiler::GetInstance()->EndSubSession();

	return false;
}

void InputManager::CheckControllerConnection()
{
	for (uint32_t cId = 0; cId < 4; ++cId)
	{
		XINPUT_STATE xState{};
		ZeroMemory(&xState, sizeof(XINPUT_STATE));
		DWORD dwResult = XInputGetState(cId, &xState);

		// Check if it's connected and log it
		if (m_ControllerConnected[cId] && dwResult != ERROR_SUCCESS)
		{
			m_ControllerConnected[cId] = false;
			Logger::GetInstance()->Log<LOG_WARNING>("Controller disconnected [ cid = " + std::to_string(cId) + " ]");

			// Call to all event listeners
			for (const auto& callback : m_ControllerConnectedCallbacks)
				callback(cId, ConnectionType::DISCONNECTED);

			continue;
		}
		else if (!m_ControllerConnected[cId] && dwResult == ERROR_SUCCESS)
		{
			m_ControllerConnected[cId] = true;
			m_ControllerComplained[cId] = true;

			Logger::GetInstance()->Log<LOG_SUCCESS>("Controller connected [ cid = " + std::to_string(cId) + " ]");

			// Call to all event listeners
			for (const auto& callback : m_ControllerConnectedCallbacks)
				callback(cId, ConnectionType::CONNECTED);
		}

		if (dwResult != ERROR_SUCCESS)
			continue;
	}
}

void InputManager::ValidateActionMapping(ActionMapping& am)
{
	for (uint32_t i = 0; i < m_SBIndex; ++i)
	{
		InputSignal s = m_InputSB[i];

		if (am.actionType == s.signalType &&
			am.action == s.signal)
			am.processor();
	}
}

bool InputManager::IsPressed(ControllerButton button, uint32_t controllerId)
{
	if (m_ControllerConnected[controllerId])
		return m_PadKeys[controllerId][button];
	else if (!m_ControllerComplained[controllerId])
	{
		Logger::GetInstance()->Log<LOG_WARNING>("Attempt of input on disconnected controller [ cid = " + std::to_string(controllerId) + " ]");
		m_ControllerComplained[controllerId] = true;
	}

	return false;
}

void InputManager::KeyDown(SDL_Scancode key)
{
	m_InputSB[m_SBIndex++] = InputSignal{ ActionType::PRESSED, (uint32_t)key };
	m_Keys[key] = true;
}

void InputManager::KeyUp(SDL_Scancode key)
{
	m_InputSB[m_SBIndex++] = InputSignal{ ActionType::RELEASED, (uint32_t)key };
	m_Keys[key] = false;
}

bool InputManager::IsKeyDown(SDL_Scancode key)
{
	return m_Keys[key];
}

std::tuple<int, int, Uint32> InputManager::GetMouseState()
{
	int x, y;
	Uint32 mouseState = SDL_GetMouseState(&x, &y);
	return std::tuple<int, int, Uint32>(x, y, mouseState);
}

void InputManager::Update(float dt)
{
	Profiler::GetInstance()->BeginSubSession<SessionId::SESSION_UPDATE_INPUT>();

	for (auto& rumble : m_RumblePacks)
		rumble.lifeTime += dt;

	auto it = std::remove_if(m_RumblePacks.begin(), m_RumblePacks.end(), [this](RumblePack& pack)
		{
			if (pack.lifeTime > pack.life)
			{
				SetControlerRumble(0, 0, pack.controller);
				return true;
			}

			return false;
		});

	if (it != m_RumblePacks.end())
		m_RumblePacks.erase(it);

	Profiler::GetInstance()->EndSubSession();
}

void InputManager::RumbleController(unsigned short leftMotor, unsigned short rightMotor, float time, uint32_t controllerId)
{
	RumblePack pack;
	pack.speedLeft = leftMotor;
	pack.speedRight = rightMotor;
	pack.controller = controllerId;
	pack.life = time;
	pack.lifeTime = 0.f;
	m_RumblePacks.push_back(pack);

	SetControlerRumble(leftMotor, rightMotor, controllerId);
}

void InputManager::SetControlerRumble(unsigned short leftMotor, unsigned short rightMotor, uint32_t controllerId)
{
	if (m_ControllerConnected[controllerId])
	{
		XINPUT_VIBRATION rumbler;
		rumbler.wLeftMotorSpeed = leftMotor;
		rumbler.wRightMotorSpeed = rightMotor;

		XInputSetState(controllerId, &rumbler);
	}
	else if (!m_ControllerComplained[controllerId])
	{
		Logger::GetInstance()->Log<LOG_WARNING>("Attempt of rumble on disconnected controller [ cid = " + std::to_string(controllerId) + " ]");
		m_ControllerComplained[controllerId] = true;
	}
}

