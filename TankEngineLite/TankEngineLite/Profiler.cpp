#include "Profiler.h"

void Profiler::BeginSession()
{
	if (m_pMainSession)
		Reset();

	// Initialize the main session with the ID of 0, time of 0, and now sub sessions
	m_pMainSession = new(Memory::New<Session>()) Session(0U);
	m_pCurrentSession = m_pMainSession;
}

void Profiler::EndSession()
{
	if (m_pCurrentSession != m_pMainSession)
		throw std::exception("Session unwinding failed, a sub session was left open");

	m_pMainSession->End();
	m_pReportSession = m_pMainSession;
}

void Profiler::Reset()
{
	m_pReportSession = m_pMainSession;
	m_pMainSession = nullptr;
	m_pCurrentSession = nullptr;
}

void Session::Report(float totalTime, int depth)
{
	const ImVec4 colours[4]
	{
		{ 1.f, 1.f, 1.f, 1.f },
		{ 0.f, 1.f, 0.f, 1.f },
		{ 1.f, 1.f, 0.f, 1.f },
		{ 1.f, 0.f, 1.f, 1.f }
	};

	const auto GetTime = [](const auto& t1, const auto& t2) -> std::chrono::duration<float> { return t2 - t1; };
	const auto PercentageOf = [totalTime](float time) { return ((time * 100) / totalTime); };

	float sessionTime = GetTime(sessionStartTime, sessionEndTime).count();
	std::stringstream stream{};
	for (int i = 0; i < depth; ++i)
		stream << "  ";

	stream << sessionNames[sessionId] << " %= ";
	int color = (depth > 3) ? 3 : depth;

	ImGui::TextColored(colours[color], stream.str().c_str());
	ImGui::SameLine();
	ImGui::Text("%.1f", PercentageOf(sessionTime));

	for (uint32_t i = 0U; i < subSessionCount; ++i)
		subSessions[i]->Report(totalTime, depth + 1);
}
