#ifndef PROFILER_H
#define PROFILER_H

#include "imgui.h"
#include "Singleton.h"
#include "MemoryTracker.h"

#include <sstream>
#include <string>
#include <chrono>

using namespace std::chrono;

enum SessionId
{
	SESSION_ROOT,
	SESSION_PROFILER,
	SESSION_UPDATE,
	SESSION_RENDER,
	SESSION_UPDATE_INPUT,
	SESSION_UPDATE_GAME,
	SESSION_UPDATE_SOUND,
	SESSION_UPDATE_ECS,
	SESSION_RENDER_ECS,
	SESSION_BATCH_DYNAMIC,
	SESSION_BATCH_STATIC,
};

static std::string sessionNames[11]
{
	"SESSION_ROOT",
	"SESSION_PROFILER",
	"SESSION_UPDATE",
	"SESSION_RENDER",
	"SESSION_UPDATE_INPUT",
	"SESSION_UPDATE_GAME",
	"SESSION_UPDATE_SOUND",
	"SESSION_UPDATE_ECS",
	"SESSION_RENDER_ECS",
	"SESSION_BATCH_DYNAMIC",
	"SESSION_BATCH_STATIC",
};

#define PROFILE(ID, FUNC)do{\
Profiler::GetInstance()->BeginSubSession<ID>();\
FUNC;\
Profiler::GetInstance()->EndSubSession();} while (0)\

struct Session
{
	uint32_t sessionId;
	float sessionResult;

	std::vector<Session*> subSessions;
	Session* pRootSession;

	high_resolution_clock::time_point sessionStartTime;
	high_resolution_clock::time_point sessionEndTime;

	explicit inline Session(uint32_t id, Session* pRoot = nullptr)
		: sessionId(id)
		, sessionResult(0.f)
		, subSessions()
		, pRootSession(pRoot)
		, sessionStartTime()
		, sessionEndTime()
	{
		sessionStartTime = std::chrono::high_resolution_clock::now();
	}

	inline ~Session()
	{
		for (const auto s : subSessions)
			Memory::Delete<Session>(s);
	}

	inline void End()
	{
		sessionEndTime = std::chrono::high_resolution_clock::now();
	}

	void Report(float totalTime, int depth = 0)
	{
		const auto GetTime = [](const auto& t1, const auto& t2) -> std::chrono::duration<float> { return t2 - t1; };
		const auto PercentageOf = [totalTime](float time) { return ((time * 100) / totalTime); };

		float sessionTime = GetTime(sessionStartTime, sessionEndTime).count();
		std::stringstream stream{};
		for (int i = 0; i < depth; ++i)
			stream << "\t";

		stream << sessionNames[sessionId] << " %= " << PercentageOf(sessionTime);

		ImGui::Text(stream.str().c_str());

		for (const auto pS : subSessions)
			pS->Report(totalTime, depth + 1);
	}
};

class Profiler
	: public Singleton<Profiler>
{
public:
	Profiler()
		: m_pReportSession(nullptr)
	{
	}

	~Profiler()
	{
		Reset();
	}

	void BeginSession();
	void EndSession();

	template<uint32_t sessionId>
	inline void BeginSubSession()
	{
		// Create a sub session 
		auto pSession = Memory::New<Session>();
		new(pSession) Session(sessionId, m_pCurrentSession);
		m_pCurrentSession->subSessions.push_back(pSession);

		m_pCurrentSession = pSession;
	}

	inline void EndSubSession()
	{
		// Stop the timer on this session
		m_pCurrentSession->End();

		// Bring back the pointer
		m_pCurrentSession = m_pCurrentSession->pRootSession;
	}

	void Report()
	{
		if (!m_pReportSession)
			return;

		const auto GetTime = [](const auto& t1, const auto& t2) -> std::chrono::duration<float>
		{
			return t2 - t1;
		};

		float rootSessionTime = GetTime(m_pReportSession->sessionStartTime, m_pReportSession->sessionEndTime).count();
		const auto PercentageOf = [rootSessionTime](float time) { return ((time * 100) / rootSessionTime); };

		ImGui::Begin("Profiler");
		m_pReportSession->Report(rootSessionTime);
		ImGui::End();

		Memory::Delete(m_pReportSession);
	}

private:
	void Reset();

	Session* m_pMainSession;
	Session* m_pCurrentSession;
	Session* m_pReportSession;
};

#endif // !PROFILER_H
