#ifndef BB_LEVEL_H
#define BB_LEVEL_H

#include "ResourceManager.h"
#include "D3D.h"

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

class BBLevel
	: public IResource
{
public:
	BBLevel()
		: m_Header()
		, m_Footer()
		, m_Tiles()
	{
	}

	BBLevel(const BBLevel&) = delete;
	BBLevel& operator=(const BBLevel&) = delete;

	bool Initialize(const std::string& path);
	void SetupBatch(SpriteBatch* m_pBatch) const;
	void Shutdown();

	[[nodiscard]] bool IsOverlapping(DirectX::XMFLOAT2 tl, DirectX::XMFLOAT2 br) const noexcept;

private:
	MapHeader m_Header;
	MapFooter m_Footer;
	std::vector<Tile> m_Tiles;
};

#endif // !BB_LEVEL_H
