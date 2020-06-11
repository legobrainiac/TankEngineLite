#include "BBLevel.h"

#include "SpriteBatch.h"
#include "BinaryInterfaces.h"

bool BBLevel::Initialize(const std::string& path)
{
	BinaryReader reader(path);

	m_Header = reader.ReadRaw<MapHeader>();
	LOGGER->Log<LOG_INFO>(
		"mapW: " + std::to_string(m_Header.mapW),
		" mapH: " + std::to_string(m_Header.mapH),
		" tileW: " + std::to_string(m_Header.tileW),
		" tileH: " + std::to_string(m_Header.tileW)
		);

	m_Tiles.resize((int)m_Header.mapW * (int)m_Header.mapH);

	for (int y = 0; y < (int)m_Header.mapH; ++y)
	{
		for (int x = 0; x < m_Header.mapW; ++x)
			m_Tiles[x + y * (int)m_Header.mapH] = reader.ReadRaw<Tile>();
	}

	m_Footer = reader.ReadRaw<MapFooter>();

	return true;
}

void BBLevel::SetupBatch(SpriteBatch* m_pBatch) const
{
	const auto scale = 2.f;

	for (int y = 0; y < m_Header.mapH; ++y)
	{
		for (int x = 0; x < m_Header.mapW; ++x)
		{
			const auto t = m_Tiles[x + y * m_Header.mapH];

			float xt = (float)(t.tileIndex % 16) * m_Header.tileW;
			float yt = (float)(t.tileIndex / 16) * m_Header.tileH;

			float alpha = (t.tileBehaviour == 0U || t.tileBehaviour == 2U) ? 1.f : 0.75f;

			if (!m_pBatch)
				return;

			m_pBatch->PushSprite(
				{
					xt,
					yt,
					xt + m_Header.tileW,
					yt + m_Header.tileH
				},
				{ x * (float)m_Header.tileW * scale, y * (float)m_Header.tileH * scale, 0.9f },
				0,
				{ scale, scale },
				{ 0, 0 },
				{ 1.f, 1.f, 1.f, alpha }
			);
		}
	}
}

void BBLevel::Shutdown()
{
}

bool BBLevel::IsOverlapping(XMFLOAT2 tl, XMFLOAT2 br, uint8_t* behaviour) const noexcept
{
	const auto scale = 2.f;

	for (int y = 0; y < m_Header.mapH; ++y)
	{
		for (int x = 0; x < m_Header.mapW; ++x)
		{
			const auto t = m_Tiles[x + y * m_Header.mapH];

			if (t.tileBehaviour == 1U)
				continue;

			float x1 = x * m_Header.tileW * scale;
			float y1 = y * m_Header.tileH * scale;
			float x2 = x1 + m_Header.tileW * scale;
			float y2 = y1 + m_Header.tileH * scale;

			if (tl.x > x2 || x1 > br.x || tl.y > y2 || y1 > br.y)
				continue;
			else
			{
				if (behaviour)
					*behaviour = t.tileBehaviour;

				return true;
			}
		}
	}

	return false;
}
