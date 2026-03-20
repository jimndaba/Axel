#include "axelpch.h"
#include "Logger.h"

#include "spdlog/spdlog.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

static std::shared_ptr<spdlog::logger> s_CoreLogger;
static std::shared_ptr<spdlog::logger> s_ClientLogger;

void Axel::Logger::Init()
{

	s_CoreLogger = spdlog::stdout_color_mt("CORE");


	spdlog::set_pattern("[%T] [%^%l%$] %v");
	s_CoreLogger->set_level(spdlog::level::trace);
	s_CoreLogger->flush_on(spdlog::level::trace);


	s_CoreLogger->info("Engine logging initialized");


}

std::shared_ptr<void>& Axel::Logger::CoreLogger()
{
	return reinterpret_cast<std::shared_ptr<void>&>(s_CoreLogger);
}



void Axel::Logger::Info(const std::string& msg)
{
	s_CoreLogger->info(msg);
}

void Axel::Logger::Warn(const std::string& msg)
{
	s_CoreLogger->warn(msg);
}

void Axel::Logger::Error(const std::string& msg)
{
	s_CoreLogger->error(msg);
}


