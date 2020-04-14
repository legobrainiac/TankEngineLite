#ifndef LOGGER_H
#define LOGGER_H

#include "imgui.h"
#include "Singleton.h"

#include <string>
#include <list>

enum LogType
{
	LOG_INFO,
	LOG_SUCCESS,
	LOG_WARNING,
	LOG_ERROR,
};

struct LogItem
{
	uint32_t type;
	std::string log;

	LogItem(const std::string& log, uint32_t type)
		: log(log)
		, type(type)
	{
	}
};

class Logger
	: public Singleton<Logger>
{
public:
	template<LogType loggerType, typename... T>
	void Log(const T& ... logs)
	{
		std::string result;

		// Unpack...
		[[maybe_unused]] int u[]{ 0, (result += logs, 0)... };
		m_Log.push_front(LogItem(result, loggerType));

		// Remove one of the old items
		if (m_Log.size() > m_MaxLogSize)
			m_Log.pop_back();
	}

	void UpdateAndDraw()
	{
		const ImVec4 colours[4]
		{
			{ 1.f, 1.f, 1.f, 1.f }, // Info
			{ 0.f, 1.f, 0.f, 1.f }, // Success
			{ 1.f, 1.f, 0.f, 1.f }, // Warning
			{ 1.f, 0.f, 0.f, 1.f }  // Error
		};

		ImGui::Begin("Log");

		static int logType = 4;
		const char* items[5] = { "INFO", "SUCCESS", "WARNING", "ERROR", "ALL" };
		ImGui::Combo("Log filter", &logType, items, IM_ARRAYSIZE(items));

		for (const auto& log : m_Log)
		{
			if (logType == 4)
			{
				ImGui::TextColored(colours[log.type], log.log.c_str());
				continue;
			}

			if (log.type == (uint32_t)logType)
				ImGui::TextColored(colours[log.type], log.log.c_str());
		}

		ImGui::End();
	}

private:
	const uint32_t m_MaxLogSize = 64;
	std::list<LogItem> m_Log;
};

#endif // !LOGGER_H