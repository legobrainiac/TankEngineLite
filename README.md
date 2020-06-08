# Tank Engine - Lite

## Goal
Tank Engine Lite is young brother to the on going Tank Engine 3D project. 
It has 2D rendering with sprite batched and 3D rendering support with audio through FMOD.
The engine structure hinges heavily on Entity component systems as the way to manipulate information.

## ECS

The ECS(**E**ntity **C**omponent **S**ystem) allows you to construct a game world built up of entities with components.

#### Include: ecs.h

To correctly implement ecs in to your game, you can do as follows:
	* From the universe, create a world
	* You populate this world with systems of the component types you need aswell as the ammount of components preallocated and their system id
	* Then when creating an entity, push components on to it to build up behaviours

### Initialization
```c++
m_pWorld = Universe::GetInstance()->PushWorld();

m_pWorld->PushSystems<
	WorldSystem<TransformComponent2D, 256, 0, ExecutionStyle::SYNCHRONOUS>,
	WorldSystem<SpriteRenderComponent, 256, 1, ExecutionStyle::SYNCHRONOUS>,
	WorldSystem<LifeSpan, 256, 2, ExecutionStyle::SYNCHRONOUS>,
	WorldSystem<ProjectileComponent, 256, 3, ExecutionStyle::SYNCHRONOUS>,
	WorldSystem<PlayerController, 8, 4, ExecutionStyle::SYNCHRONOUS>,
	WorldSystem<ParticleEmitter, 16, 5, ExecutionStyle::SYNCHRONOUS>,
	WorldSystem<Particle, 4096, 6, ExecutionStyle::ASYNCHRONOUS>,
	WorldSystem<TransformComponent, 8, 7, ExecutionStyle::SYNCHRONOUS>,
	WorldSystem<CameraComponent, 8, 8, ExecutionStyle::SYNCHRONOUS>,
	WorldSystem<ModelRenderComponent, 8, 9, ExecutionStyle::SYNCHRONOUS>
>();
```
### Entity creation and setup, ie: Player
```c++
auto pEntity = m_pWorld->CreateEntity();
auto [pMovement, pRenderer, pTransform] = pEntity->PushComponents<PlayerController, SpriteRenderComponent, TransformComponent2D>();

// Component setup
pRenderer->SetSpriteBatch(pSpriteBatch);
pRenderer->SetAtlasTransform({ 0, 0, 16, 16 });

pTransform->position = { pos.x, pos.y, 0.f };
pTransform->scale = { 4.f, 4.f };

pMovement->SetInputController(player);
```

### Usage for a system, ie: Rendering
```c++
auto pSystem = m_pWorld->GetSystemByComponent<SpriteRenderComponent>();

pSystem->ForAll(
	[](ECS::EntityComponent* pC)
	{
		static_cast<SpriteRenderComponent*>(pC)->Render();
	});
```

### Example component, ie: ProjectileComponent
```c++
class ProjectileComponent
	: public ECS::EntityComponent
{
public:
	ProjectileComponent() = default;
	ProjectileComponent(ECS::Entity* pE)
		: ECS::EntityComponent(pE)
	{
		m_MeetsRequirements = false;
		m_pTransform = pE->GetComponent<TransformComponent2D>();

		if (m_pTransform != nullptr)
			m_MeetsRequirements = true;
	}

	void Update(float dt) override	
	{
		if (!m_MeetsRequirements)
			return;

		m_pTransform->position.x += m_Direction.x * dt * m_Speed;
		m_pTransform->position.y += m_Direction.y * dt * m_Speed;
	}

	inline void SetDirection(const XMFLOAT2& direction) { m_Direction = direction; }
	inline void SetSpeed(float speed) { m_Speed = speed; }

private:
	TransformComponent2D* m_pTransform;
	XMFLOAT2 m_Direction;
	float m_Speed;
	bool m_MeetsRequirements;
};
```

## Input Management

The input manager is not yet fully implemented but it does come with some core features as direct input manipulation as well as Action Mapping setup and a callback system for controller connections.

#### Include: InputManager.h

### Usage
```c++
// Registering a callback to be called when a controller is connected
// 	ConnectionType::CONNECTED 		-> Player created for a specific controller
// 	ConnectionType::DISCONNECTED 	-> Player for specific controller is removed
InputManager::GetInstance()->RegisterControllerConnectedCallback(
	[this](uint32_t controller, ConnectionType connection) 
	{
		if (connection == ConnectionType::CONNECTED)
			m_pPlayers[controller] = Prefabs::CreatePlayer(m_pWorld, m_pDynamic_SB, { float(rand() % 1000 + 300), 0 }, (Player)controller);
		else
			m_pWorld->DestroyEntity(m_pPlayers[controller]->GetId());
	});

// Runs a check for connected controllers, this shouldn't be called every frame. XINPUT is expensive xD
InputManager::GetInstance()->CheckControllerConnection();
```
```c++
// Action mapping for toggling a particle system when R is pressed
InputManager::GetInstance()->RegisterActionMappin(
	ActionMapping(SDL_SCANCODE_R, ActionType::PRESSED,
		[pParticleSystem]()
		{
			pParticleSystem->ToggleSpawning();
		}));
```
```c++
// Direct input
if (pInputMananager->IsPressed(ControllerButton::DPAD_LEFT, m_PlayerController))
	movement.x -= dt * movementSpeed;

if (pInputMananager->IsPressed(ControllerButton::DPAD_RIGHT, m_PlayerController))
	movement.x += dt * movementSpeed;
```

## Resource Management

The resource manager currently support the loading of the following formats:

* fx (HLSL effects file)
* wav
* temd (Tank Engine Model Descriptor for .fbx)
* jpg
* png

By default all assets of this type inside the resources folder will be automatically loaded when the engine starts.
The macro: `#define RESOURCES ResourceManager::GetInstance()` is provided.

### Usage
```c++
// Get resource with name atlas_0 and type Texture
const auto pAtlasTex = RESOURCES->Get<Texture>("atlas_0");

// Load a resource of type Sound
const auto pSound = RESOURCES->Load<Sound>("SomeCoolSound.wav", "SomeCoolSound");
```	