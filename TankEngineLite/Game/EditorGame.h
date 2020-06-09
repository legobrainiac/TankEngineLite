#ifndef EDITOR_GAME_H
#define EDITOR_GAME_H

#include <Tel.h>
#include <vector>

#include "BinaryInterfaces.h"
#include "BBLevel.h"

class SpriteBatch;

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
