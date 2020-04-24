#ifndef PROFILER_H
#define PROFILER_H

#include "imgui.h"
#include "Singleton.h"
#include "MemoryTracker.h"

#include <sstream>
#include <string>
#include <chrono>

using namespace std::chrono;

#define PROFILING_ON

//////////////////////////////////////////////////////////////////////////
// Enum: SessionId
// Description: All the sessions id's, this is temporary
enum SessionId
{
	SESSION_ROOT,
	SESSION_PROFILER,
	SESSION_UPDATE,
	SESSION_RENDER,
	SESSION_RENDER_BEGIN,
	SESSION_RENDER_END,
	SESSION_UPDATE_INPUT,
	SESSION_UPDATE_GAME,
	SESSION_UPDATE_SOUND,
	SESSION_UPDATE_ECS,
	SESSION_RENDER_ECS,
	SESSION_BATCH_DYNAMIC,
	SESSION_BATCH_STATIC,
	SESSION_IMGUI_NEWFRAME,
	SESSION_PROCESS_INPUT,
	SESSION_SDL_POOL_EVENT,
	SESSION_XINPUT_UPDATE,
	SESSION_ACTIONMAPPING_UPDATE,
	SESSION_COUNT
};

static std::string sessionNames[SessionId::SESSION_COUNT]
{
	"ROOT",
	"PROFILER",
	"UPDATE",
	"RENDER",
	"RENDER_BEGIN",
	"RENDER_END",
	"UPDATE_INPUT",
	"UPDATE_GAME",
	"UPDATE_SOUND",
	"UPDATE_ECS",
	"RENDER_ECS",
	"BATCH_DYNAMIC",
	"BATCH_STATIC",
	"IMGUI_NEWFRAME",
	"PROCESS_INPUT",
	"SDL_POOL_EVENT",
	"XINPUT_UPDATE",
	"ACTIONMAPPING_UPDATE"
};

#define PROFILE(ID, FUNC)do{\
Profiler::GetInstance()->BeginSubSession<ID>();\
FUNC;\
Profiler::GetInstance()->EndSubSession();} while (0)\

//////////////////////////////////////////////////////////////////////////
// Struct: Session
// Description: Represents a session or a subsession for profiling a specific thing
struct Session
{
	uint32_t sessionId;
	float sessionResult;

	std::vector<Session*> subSessions;
	Session* pRootSession;

	high_resolution_clock::time_point sessionStartTime;
	high_resolution_clock::time_point sessionEndTime;

	//////////////////////////////////////////////////////////////////////////
	// Method:    Session
	// FullName:  Session::Session
	// Access:    public 
	// Qualifier: : sessionId(id) , sessionResult(0.f) , subSessions() , pRootSession(pRoot) , sessionStartTime() , sessionEndTime()
	// Parameter: uint32_t id
	// Parameter: Session* pRoot
	// Description: Begin the profiling session
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

	//////////////////////////////////////////////////////////////////////////
	// Method:    ~Session
	// FullName:  Session::~Session
	// Access:    public 
	inline ~Session()
	{
		for (const auto s : subSessions)
			Memory::Delete<Session>(s);
	}

	//////////////////////////////////////////////////////////////////////////
	// Method:    End
	// FullName:  Session::End
	// Access:    public 
	// Returns:   void
	// Description: End the profiling session
	inline void End()
	{
		sessionEndTime = std::chrono::high_resolution_clock::now();
	}

	//////////////////////////////////////////////////////////////////////////
	// Method:    Report
	// FullName:  Session::Report
	// Access:    public 
	// Returns:   void
	// Description: Generate the ImGui report card
	// Parameter: float totalTime
	// Parameter: int depth
	void Report(float totalTime, int depth = 0)
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

		for (const auto pS : subSessions)
			pS->Report(totalTime, depth + 1);
	}
};

//////////////////////////////////////////////////////////////////////////
// Class: Profiler
// Description: Contains all the functionality to profile code and report results
class Profiler
	: public Singleton<Profiler>
{
public:
	//////////////////////////////////////////////////////////////////////////
	// Method:    Profiler
	// FullName:  Profiler::Profiler
	// Access:    public    
	constexpr Profiler()
		: m_pMainSession(nullptr)
		, m_pCurrentSession(nullptr)
		, m_pReportSession(nullptr)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	// Method:    Destroy
	// FullName:  Profiler::Destroy
	// Access:    public 
	// Returns:   void
	// Description: Cleanup profiler
	void Destroy()
	{
		Memory::Delete(m_pMainSession);
	}

	//////////////////////////////////////////////////////////////////////////
	// Method:    BeginSession
	// FullName:  Profiler::BeginSession
	// Access:    public 
	// Returns:   void
	// Description: Begin profiling session
	void BeginSession();

	//////////////////////////////////////////////////////////////////////////
	// Method:    EndSession
	// FullName:  Profiler::EndSession
	// Access:    public 
	// Returns:   void
	// Description: End the session
	void EndSession();

	//////////////////////////////////////////////////////////////////////////
	// Method:    BeginSubSession
	// FullName:  Profiler::BeginSubSession<uint32_t sessionId>
	// Access:    public 
	// Returns:   void
	// Description: Begin a sub session of the profiler, 
	//		main session needs to be open
	template<uint32_t sessionId>
	inline void BeginSubSession()
	{
#ifdef PROFILING_ON
		// Create a sub session 
		auto pSession = Memory::New<Session>();
		new(pSession) Session(sessionId, m_pCurrentSession);
		m_pCurrentSession->subSessions.push_back(pSession);

		m_pCurrentSession = pSession;
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	// Method:    EndSubSession
	// FullName:  Profiler::EndSubSession
	// Access:    public 
	// Returns:   void
	// Description: End the currently open sub session
	inline void EndSubSession()
	{
#ifdef PROFILING_ON
		// Stop the timer on this session
		m_pCurrentSession->End();

		// Bring back the pointer
		m_pCurrentSession = m_pCurrentSession->pRootSession;
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	// Method:    Report
	// FullName:  Profiler::Report
	// Access:    public 
	// Returns:   void
	// Description: Generate a report card
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
