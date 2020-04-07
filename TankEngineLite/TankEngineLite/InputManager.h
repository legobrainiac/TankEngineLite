#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "SDL.h"
#include "Singleton.h"

#include <map>
#include <tuple>
#include <functional>

// TODO(tomas): currently action mappings are keyboard only

// Input processors
using ActionMappingProcessor = std::function<void()>;
using AxisMappingProcessor = std::function<void(float)>;

// Helper enums
enum Player : uint32_t
{
    PLAYER0,
    PLAYER1,
    PLAYER2,
    PLAYER3,
    PLAYER_INVALID
};

enum ActionType
{
    PRESSED,
    RELEASED
};

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

// Signals get generated when an input event happens
struct InputSignal
{
    ActionType signalType;
    uint32_t signal;
};

// Action Mappings get validated against signals
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

class InputManager final 
	: public Singleton<InputManager>
{
public:
    InputManager()
        : m_Keys()
        , m_PadKeys()
        , m_ControllerConnected()
        , m_SBIndex()
    {
    }
    
	void KeyDown(SDL_Scancode key);
	void KeyUp(SDL_Scancode key);
	bool IsKeyDown(SDL_Scancode key);
    
	bool ProcessInput();
	bool IsPressed(ControllerButton button, uint32_t controllerId = 0U) const;
	std::tuple<int, int, Uint32> GetMouseState();
    
    void inline RegisterActionMappin(const ActionMapping& am)
    {
        m_ActionMappings[am.actionType].push_back(am);
    }
    
private:
	bool m_Keys[512]; // Keyboard keys
    bool m_PadKeys[4][18]; // Gamepad keys
    bool m_ControllerConnected[4]; // Gamepad status
    
    // Signal buffer
    InputSignal m_InputSB[1024];
    uint32_t m_SBIndex;
    
    // Container for action mappings
    std::map<ActionType, std::vector<ActionMapping>> m_ActionMappings;
    void ValidateActionMapping(ActionMapping& am);    
};

#endif // !INPUT_MANAGER_H