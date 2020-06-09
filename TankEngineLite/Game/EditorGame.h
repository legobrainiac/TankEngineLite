#ifndef EDITOR_GAME_H
#define EDITOR_GAME_H

#include <Tel.h>
#include <vector>

#include "BinaryInterfaces.h"

class SpriteBatch;

struct MapHeader
{
	uint8_t mapW = 30U;
	uint8_t mapH = 30U;
	uint8_t tileW = 16U;
	uint8_t tileH = 16U;
};

struct MapFooter
{
	uint8_t playerSpawns[4]{ 0, 0, 0, 0 };
	uint8_t maitaSpawns[4]{ 0, 0, 0, 0 };
	uint8_t zenSpawns[4]{ 0, 0, 0, 0 };
};

struct Tile
{
	uint8_t tileIndex = 0U;
	uint8_t tileBehaviour = 0U;
};

class EditorGame 
	: public IGame
{
public:
	void Initialize() override;
	void Load(ResourceManager* pResourceManager, TEngineRunner* pEngine) override;
	void Update(float dt, InputManager* pInputManager) override;
	void Render(Renderer* pRenderer) override;
	void Shutdown() override;

	void PushBatch();

	void LoadMap(const std::string& path);
	void SaveMap(const std::string& path);

private:	
	SpriteBatch* m_pSB;
	
	MapHeader m_Header;
	MapFooter m_Footer;
	std::vector<Tile> m_Tiles {};
};

#endif // !EDITOR_GAME_H
