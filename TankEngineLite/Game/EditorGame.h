#ifndef EDITOR_GAME_H
#define EDITOR_GAME_H

#include <Tel.h>
#include <vector>
#include <array>

#include "BinaryInterfaces.h"
#include "BBLevel.h"

class SpriteBatch;

class EditorGame 
	: public IGame
{
public:

	EditorGame()
		: m_Tiles()
	{
	}

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
	
	static int maitaSpawns[4];
	static int zenChanSpawns[4];

	MapHeader m_Header;
	MapFooter m_Footer;
	std::array<Tile, 900> m_Tiles;

	int m_SelectedIndex = 0;
};

#endif // !EDITOR_GAME_H
