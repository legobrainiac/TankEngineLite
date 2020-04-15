#include "Profiler.h"

void Profiler::BeginSession()
{
	if (m_pMainSession)
		Reset();

	// Initialize the main session with the ID of 0, time of 0, and now sub sessions
	m_pMainSession = Memory::New<Session>();
	new(m_pMainSession) Session(0U);

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
