#pragma once
#ifndef LOGGER_H

#include "core/Core.h"

namespace Axel {

	class AX_API Logger
	{
	public:
		static void Init();

		// Loggers for engine and client/game code
		static std::shared_ptr<void>& CoreLogger();
		static void Info(const std::string& message);
		static void Warn(const std::string& message);
		static void Error(const std::string& message);

	};
}

#define AXLOG_INFO(msg)    Axel::Logger::Info(msg)
#define AXLOG_WARN(msg)    Axel::Logger::Warn(msg)
#define AXLOG_ERROR(msg)   Axel::Logger::Error(msg)

#endif

