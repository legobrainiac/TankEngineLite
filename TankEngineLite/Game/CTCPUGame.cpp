#include "CTCPUGame.h"
#include <iomanip>

void CTCPUGame::Initialize()
{
	c = {};
	m_CpuClockSpeed = 10.f;
}

void CTCPUGame::Load([[maybe_unused]] ResourceManager* pResourceManager, [[maybe_unused]] TEngineRunner* pEngine)
{
}

void CTCPUGame::Update([[maybe_unused]] float dt, [[maybe_unused]] InputManager* pInputManager)
{
	uint32_t opc = c.pc;

	constexpr std::array<op, 6U> code{
		op(0x07, REG_A, 0x00),   // MOV REG_A 0x00   ; Move 0 in to Register A
		op(0x07, REG_B, 0x7F),   // MOV REG_B 0x7F   ; Move 127 in to Register B
		op(0x0A, REG_A, REG_A),  // STA REG_A REG_A  ; Store contents of Register A at address stored in Register A
		op(0x08, REG_A),         // INC REG_A        ; Increment contents of register A
		op(0x05),                // CMP              ; Compare Register A and Register B and store result in Register C
		op(0x03, 0x02),          // JNZ 0x02         ; Jump to 0x02 if Register C != 0U
	};

	if (m_RunCpu)
	{
		m_CurrentTick += dt;

		if (m_CurrentTick > (1.f / m_CpuClockSpeed))
		{
			c = step(code, c);
			m_CurrentTick -= (1.f / m_CpuClockSpeed);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// ImGui debug
	ImGui::Begin("CPU");

	//////////////////////////////////////////////////////////////////////////
	// Registers
	std::stringstream s;
	s << std::hex << +c.memory.registers[0];
	ImGui::TextColored({ 1.f, 1.f, 0.f, 1.f }, "reg a = ");
	ImGui::SameLine();
	ImGui::TextColored({ 0.f, 1.f, 1.f, 1.f }, std::string(s.str()).c_str());
	ImGui::SameLine();
	s = std::stringstream{};

	s << std::hex << +c.memory.registers[1];
	ImGui::TextColored({ 1.f, 1.f, 0.f, 1.f }, "reg b = ");
	ImGui::SameLine();
	ImGui::TextColored({ 0.f, 1.f, 1.f, 1.f }, std::string(s.str()).c_str());
	ImGui::SameLine();
	s = std::stringstream{};

	s << std::hex << +c.memory.registers[2];
	ImGui::TextColored({ 1.f, 1.f, 0.f, 1.f }, "reg c = ");
	ImGui::SameLine();
	ImGui::TextColored({ 0.f, 1.f, 1.f, 1.f }, std::string(s.str()).c_str());
	ImGui::SameLine();

	if (ImGui::Button("RESET"))
		c = cpu{};

	if (ImGui::Button("STEP"))
		c = step(code, c);

	ImGui::Checkbox("Run automatically", &m_RunCpu);
	ImGui::SameLine();
	ImGui::SliderFloat("Clock Speed", &m_CpuClockSpeed, 1.f, 1000.f);

	//////////////////////////////////////////////////////////////////////////
	// Ram
	for (uint32_t i = 0U; i < 128; ++i)
	{
		if (i % 16 != 0)
			ImGui::SameLine();
		else
			ImGui::Separator();

		std::stringstream stream;
		stream << std::hex << +c.memory.ram[i];

		ImGui::Text(std::string(stream.str()).c_str());
	}

	//////////////////////////////////////////////////////////////////////////
	// Instructions
	ImGui::Separator();

	for (uint32_t i = 0; i < code.size(); ++i)
	{
		{
			std::stringstream stream;
			stream << std::hex << i << ": ";
			ImGui::TextColored({ 0.f, 1.f, 0.f, 1.f }, std::string(stream.str()).c_str());
			ImGui::SameLine();
		}

		if (i == opc)
		{
			std::stringstream stream;

			stream << std::hex << +code[i].opCode;
			ImGui::TextColored({ 1.f, 0.f, 0.f, 1.f }, std::string(stream.str()).c_str());
			ImGui::SameLine();
			stream = std::stringstream{};

			stream << std::hex << +code[i].o1;
			ImGui::TextColored({ 1.f, 0.f, 0.f, 1.f }, std::string(stream.str()).c_str());
			ImGui::SameLine();
			stream = std::stringstream{};

			stream << std::hex << +code[i].o2;
			ImGui::TextColored({ 1.f, 0.f, 0.f, 1.f }, std::string(stream.str()).c_str());
		}
		else
		{
			std::stringstream stream;

			stream << std::hex << +code[i].opCode;
			ImGui::Text(std::string(stream.str()).c_str());
			ImGui::SameLine();
			stream = std::stringstream{};

			stream << std::hex << +code[i].o1;
			ImGui::Text(std::string(stream.str()).c_str());
			ImGui::SameLine();
			stream = std::stringstream{};

			stream << std::hex << +code[i].o2;
			ImGui::Text(std::string(stream.str()).c_str());
		}
	}

	ImGui::End();
}

void CTCPUGame::Render([[maybe_unused]] Renderer* pRenderer)
{
}

void CTCPUGame::Shutdown()
{
}
