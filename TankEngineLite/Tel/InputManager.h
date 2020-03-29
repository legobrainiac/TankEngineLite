#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "SDL.h"
#include "Singleton.h"

#include <tuple>
#include <functional>
#include <vector>
#include <XInput.h>

// Double buffered input system

// Input processors
typedef std::function<void()> ActionMappingProcessor;
typedef std::function<void(float)> AxisMappingProcessor;

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

struct ActionMapping
{
    uint32_t action;
    uint32_t controllerId;
    ActionType actionType;
    ActionMappingProcessor processor;
    
    ActionMapping(uint32_t a, ActionType at, const ActionMappingProcessor& amp)
        : action(a)
        , actionType(at)
        , processor(amp)
        , controllerId((uint32_t)Player::PLAYER_INVALID)
    {
    }
    
    ActionMapping(uint32_t a, ActionType at, const ActionMappingProcessor& amp, uint32_t cId)
        : action(a)
        , actionType(at)
        , processor(amp)
        , controllerId(cId)
    {
    }
};

// Helper enums
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
        {
            m_Keys[0][i] = false;
            m_Keys[1][i] = false;
        }
    }
    
	void KeyDown(SDL_Scancode key);
	void KeyUp(SDL_Scancode key);
	bool IsKeyDown(SDL_Scancode key);
    
	bool ProcessInput();
	bool IsPressed(ControllerButton button, uint32_t controllerId = 0U) const;
	std::tuple<int, int, Uint32> GetMouseState();
    
    void inline RegisterActionMappin(const ActionMapping& am)
    {
        m_ActionMappings.push_back(am);
    }
    
private:
    uint32_t m_ActiveBuffer = 1;
	bool m_Keys[2][512];
    bool m_PadKeys[2][4][18];
    bool m_ControllerConnected[4];
     
    void ValidateActionMapping(ActionMapping& am);
    std::vector<ActionMapping> m_ActionMappings;
};

#endif // !INPUT_MANAGER_H