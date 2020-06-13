#include "EditorGame.h"
#include "SpriteBatch.h"

#define SCALE 2

int EditorGame::maitaSpawns[4] = {};
int EditorGame::zenChanSpawns[4] = {};

void EditorGame::Initialize()
{

}

void EditorGame::Load([[maybe_unused]] ResourceManager* pResourceManager, [[maybe_unused]] TEngineRunner* pEngine)
{
	//////////////////////////////////////////////////////////////////////////
	// Create the static sprite batch and register
	m_pSB = new(Memory::New<SpriteBatch>()) SpriteBatch("Static");
	m_pSB->InitializeBatch(RESOURCES->Get<Texture>("atlas_7"), BatchMode::BATCHMODE_DYNAMIC);
	pEngine->RegisterBatch(m_pSB);

	LoadMap("../Resources/default.bmap");
}

void EditorGame::Update([[maybe_unused]] float dt, [[maybe_unused]] InputManager* pInputManager)
{
	static int tile = 0;
	static std::string path = "../Resources/default.bmap";

	ImGui::Begin("Editor");
	ImGui::InputInt("Tile ID", &tile);
	tile = std::clamp(tile, 0, 255);

	float xt = (float)(tile % 16) * m_Header.tileW;
	float yt = (float)(tile / 16) * m_Header.tileH;

	ImGui::Image(
		(void*)m_pSB->GetTextureView(),
		ImVec2(64, 64),
		ImVec2(xt / 256.f, yt / 256.f),
		ImVec2(xt / 256.f + (1.f / 16.f), yt / 256.f + (1.f / 16.f))
	);

	ImGui::InputText("Path", const_cast<char*>(path.c_str()), path.capacity());
	if (ImGui::Button("Save map"))
		SaveMap(path);

	if (ImGui::Button("Load map"))
		LoadMap(path);

	if (ImGui::Button("Clear last row"))
	{
		for (size_t i = m_Tiles.size() - 1; i > m_Tiles.size() - 30; --i)
			m_Tiles[i].tileBehaviour = 1U;
	}

	ImGui::Text("Selected index: ");
	ImGui::SameLine();
	ImGui::Text(std::to_string(m_SelectedIndex).c_str());

	maitaSpawns[0] = m_Footer.maitaSpawns[0];
	maitaSpawns[1] = m_Footer.maitaSpawns[1];
	maitaSpawns[2] = m_Footer.maitaSpawns[2];
	maitaSpawns[3] = m_Footer.maitaSpawns[3];

	zenChanSpawns[0] = m_Footer.zenSpawns[0];
	zenChanSpawns[1] = m_Footer.zenSpawns[1];
	zenChanSpawns[2] = m_Footer.zenSpawns[2];
	zenChanSpawns[3] = m_Footer.zenSpawns[3];

	ImGui::InputInt4("Maita Spawns", maitaSpawns);
	ImGui::InputInt4("Zen Chan Spawns", zenChanSpawns); 

	for (int i = 0; i < 4; ++i)
		m_Footer.maitaSpawns[i] = (uint16_t)std::clamp(maitaSpawns[i], 0, 900);

	for (int i = 0; i < 4; ++i)
		m_Footer.zenSpawns[i] = (uint16_t)std::clamp(zenChanSpawns[i], 0, 900);

	ImGui::End();

	//////////////////////////////////////////////////////////////////////////
	// Tile placement
	auto[x, y, s] = pInputManager->GetMouseState();

	if (
		x < m_Header.mapW * m_Header.tileW * SCALE && y < m_Header.mapH * m_Header.tileH * SCALE &&
		x > 0 && y > 0
		)
	{
		m_pSB->PushSprite(
			{
				xt,
				yt,
				xt + m_Header.tileW,
				yt + m_Header.tileH
			},
			{ (float)x - (x % (16 * SCALE)), (float)y - (y % (16 * SCALE)), 0.2f },
			0,
			{ SCALE, SCALE },
			{ 0, 0 },
			{ 1.f, 1.f, 1.f, 1.f }
		);

		//////////////////////////////////////////////////////////////////////////
		// Mouse picking
		
		int xi = (x - (x % (16 * SCALE))) / m_Header.mapW;
		int yi = (y - (y % (16 * SCALE))) / m_Header.mapH;

		//////////////////////////////////////////////////////////////////////////
		// This is horrible
		// but the previous math is scuffed and I cant be bothered to think about it rn
		if (xi > 15)
			--xi;

		if (yi > 15)
			--yi;

		xi = std::clamp(xi, 0, (int)m_Header.mapW);
		yi = std::clamp(yi, 0, (int)m_Header.mapH);

		m_SelectedIndex = xi + yi * m_Header.mapH;

		// End of scuff
		//////////////////////////////////////////////////////////////////////////

		if (s & SDL_BUTTON(SDL_BUTTON_LEFT))
		{
			m_Tiles[xi + yi * m_Header.mapH].tileIndex = (uint8_t)tile;
			m_Tiles[xi + yi * m_Header.mapH].tileBehaviour = 0U;
		}
		
		if (s & SDL_BUTTON(SDL_BUTTON_RIGHT))
			m_Tiles[xi + yi * m_Header.mapH].tileBehaviour = 1U; // Wall
		else if (s & SDL_BUTTON(SDL_BUTTON_MIDDLE))
			m_Tiles[xi + yi * m_Header.mapH].tileBehaviour = 2U; // Platform
	}

	PushBatch();
}

void EditorGame::Render([[maybe_unused]] Renderer* pRenderer)
{
	m_pSB->Render();
}

void EditorGame::Shutdown()
{
	m_pSB->Destroy();
	Memory::Delete(m_pSB);
}

void EditorGame::PushBatch()
{
	for(int y = 0; y < m_Header.mapH; ++y)
	{
		for(int x = 0; x < m_Header.mapW; ++x)
		{
			const auto t = m_Tiles[x + y * m_Header.mapH];

			float xt = (float)(t.tileIndex % 16) * m_Header.tileW;
			float yt = (float)(t.tileIndex / 16) * m_Header.tileH;

			XMFLOAT4 c = (t.tileBehaviour == 2U) ? XMFLOAT4{ 1.f, 0.f, 0.f, 1.f } : XMFLOAT4{ 1.f, 1.f, 1.f, (t.tileBehaviour == 0U) ? 1.f : 0.75f };

			m_pSB->PushSprite(
				{ 
					xt, 
					yt, 
					xt + m_Header.tileW, 
					yt + m_Header.tileH
				}, 
				{ x * (float)m_Header.tileW * SCALE, y * (float)m_Header.tileH * SCALE, 0.9f },
				0, 
				{ SCALE, SCALE },
				{ 0, 0 }, 
				{ c.x, c.y, c.z, c.w }
			);
		}
	}
}

void EditorGame::LoadMap(const std::string& path)
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
}

void EditorGame::SaveMap(const std::string& path)
{
	//////////////////////////////////////////////////////////////////////////
	// Read map info in to memory
	BinaryWriter writer(path);
	writer.WriteRaw<MapHeader>(m_Header);

	for (const auto t : m_Tiles)
		writer.WriteRaw<Tile>(t);

	writer.WriteRaw<MapFooter>(m_Footer);
}
