#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "SDL.h"
#include "Singleton.h"

#include <map>
#include <tuple>
#include <vector>
#include <functional>

#include "Profiler.h"

// TODO(tomas): currently action mappings are keyboard only

//////////////////////////////////////////////////////////////////////////
// Enum: Player
// Description: PlayerController enum
enum Player : uint32_t
{
	PLAYER0,
	PLAYER1,
	PLAYER2,
	PLAYER3,
	PLAYER_INVALID
};

//////////////////////////////////////////////////////////////////////////
// Enum: ActionType
// Description: If an action is key pressed or key released
enum ActionType
{
	PRESSED,
	RELEASED
};

//////////////////////////////////////////////////////////////////////////
// Enum: ControllerButton
// Description: All the buttons in a controller
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

//////////////////////////////////////////////////////////////////////////
// Enum: ConnectionType
// Description: Type of controller connections
enum ConnectionType : uint32_t
{
	CONNECTED,
	DISCONNECTED
};

//////////////////////////////////////////////////////////////////////////
// Input processors
using ActionMappingProcessor = std::function<void()>;
using AxisMappingProcessor = std::function<void(float)>;
using ControllerConnectedCallback = std::function<void(uint32_t, ConnectionType)>;

//////////////////////////////////////////////////////////////////////////
// Struct: InputSignal
// Description: Signals get generated when an input event happens
struct InputSignal
{
	ActionType signalType;
	uint32_t signal;
};

//////////////////////////////////////////////////////////////////////////
// Struct: ActionMapping
// Description: Action Mappings get validated against signals
struct ActionMapping
{
	uint32_t action;
	ActionType actionType;
	ActionMappingProcessor processor;

	ActionMapping(uint32_t a, ActionType at, const ActionMappingProcessor& amp)
		: action(a)
		, actionType(at)
		, processor(amp)
	{
	}
};

//////////////////////////////////////////////////////////////////////////
// Struct: RumblePack
// Description: Keeps track of rumble events, used for clearing rumble once expired
struct RumblePack
{
	unsigned short speedLeft;
	unsigned short speedRight;
	float life;
	float lifeTime;

	uint32_t controller;
};

//////////////////////////////////////////////////////////////////////////
// Class: InputManager
// Description: Input management handling, force feedback
class InputManager final
	: public Singleton<InputManager>
{
public:
	InputManager()
		: m_Keys()
		, m_PadKeys()
		, m_ControllerConnected()
		, m_SBIndex()
		, m_ControllerComplained()
		, m_Initialized()
		, m_InputSB()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	// Method:    KeyDown
	// FullName:  InputManager::KeyDown
	// Access:    public 
	// Returns:   void
	// Parameter: SDL_Scancode key
	void KeyDown(SDL_Scancode key);

	//////////////////////////////////////////////////////////////////////////
	// Method:    KeyUp
	// FullName:  InputManager::KeyUp
	// Access:    public 
	// Returns:   void
	// Parameter: SDL_Scancode key
	void KeyUp(SDL_Scancode key);

	//////////////////////////////////////////////////////////////////////////
	// Method:    IsKeyDown
	// FullName:  InputManager::IsKeyDown
	// Access:    public 
	// Returns:   bool
	// Parameter: SDL_Scancode key
	bool IsKeyDown(SDL_Scancode key);

	//////////////////////////////////////////////////////////////////////////
	// Method:    ProcessInput
	// FullName:  InputManager::ProcessInput
	// Access:    public 
	// Returns:   bool
	// Description: Process all the input and generate signals for the action maps
	bool ProcessInput();

	//////////////////////////////////////////////////////////////////////////
	// Method:    CheckControllerConnection
	// FullName:  InputManager::CheckControllerConnection
	// Access:    public 
	// Returns:   void
	// Description: Check which controllers are connected, 
	//		be ware, don't call this every frame
	void CheckControllerConnection();

	//////////////////////////////////////////////////////////////////////////
	// Method:    IsPressed
	// FullName:  InputManager::IsPressed
	// Access:    public 
	// Returns:   bool
	// Description: is button on controller with id controllerId pressed
	// Parameter: ControllerButton button
	// Parameter: uint32_t controllerId
	[[nodiscard]] bool IsPressed(ControllerButton button, uint32_t controllerId = 0U) noexcept;

	//////////////////////////////////////////////////////////////////////////
	// Method:    GetMouseState
	// FullName:  InputManager::GetMouseState
	// Access:    public 
	// Returns:   std::tuple<int, int, Uint32>
	// Description: Return the mouse state in a tuple
	[[nodiscard]] std::tuple<int, int, Uint32> GetMouseState();

	//////////////////////////////////////////////////////////////////////////
	// Method:    Update
	// FullName:  InputManager::Update
	// Access:    public 
	// Returns:   void
	// Description: Process rumble pack expirations, 
	// Parameter: float dt
	void Update(float dt);

	//////////////////////////////////////////////////////////////////////////
	// Method:    RegisterActionMappin
	// FullName:  InputManager::RegisterActionMappin
	// Access:    public 
	// Returns:   void
	// Description: Register action mappings
	// Parameter: const ActionMapping& am
	void inline RegisterActionMappin(const ActionMapping& am)
	{
		LOGGER->Log<LOG_INFO>("Registered action mapping >> " + std::to_string(am.action));
		m_ActionMappings[am.actionType].push_back(am);
	}

	//////////////////////////////////////////////////////////////////////////
	// Method:    RegisterControllerConnectedCallback
	// FullName:  InputManager::RegisterControllerConnectedCallback
	// Access:    public 
	// Returns:   void
	// Description: Register a ControllerConnectedCallback
	// Parameter: const ControllerConnectedCallback& callback
	void inline RegisterControllerConnectedCallback(const ControllerConnectedCallback& callback)
	{
		LOGGER->Log<LOG_INFO>("Registered controller connected callback");
		m_ControllerConnectedCallbacks.push_back(callback);
	}

	//////////////////////////////////////////////////////////////////////////
	// Method:    RumbleController
	// FullName:  InputManager::RumbleController
	// Access:    public 
	// Returns:   void
	// Description: Create a rumble event
	// Parameter: unsigned short leftMotor
	// Parameter: unsigned short rightMotor
	// Parameter: float time
	// Parameter: uint32_t controllerId
	void RumbleController(unsigned short leftMotor, unsigned short rightMotor, float time, uint32_t controllerId = 0U);

private:
	void SetControlerRumble(unsigned short leftMotor, unsigned short rightMotor, uint32_t controllerId = 0U);

	bool m_Keys[512]; // Keyboard keys
	bool m_PadKeys[4][18]; // Gamepad keys
	bool m_ControllerConnected[4]; // Gamepad status
	bool m_ControllerComplained[4]; // Has logged a complaint
	bool m_Initialized;

	// Signal buffer
	InputSignal m_InputSB[1024];
	uint32_t m_SBIndex;

	// Container for action mappings
	std::map<ActionType, std::vector<ActionMapping>> m_ActionMappings;
	void ValidateActionMapping(ActionMapping& am);

	// Callbacks
	std::vector<ControllerConnectedCallback> m_ControllerConnectedCallbacks;

	// Rumble commands
	std::vector<RumblePack> m_RumblePacks;
};

#endif // !INPUT_MANAGER_H