#include "axelpch.h"
#include "Logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

std::shared_ptr<spdlog::logger> Axel::Logger::m_logger;

void Axel::Logger::Init()
{

	m_logger = spdlog::stdout_color_mt("CORE");


	//spdlog::set_pattern("[%T] [%^%l%$] %v");
	m_logger->set_pattern("%^[%T] %n: %v%$");
	m_logger->set_level(spdlog::level::trace);
	m_logger->flush_on(spdlog::level::trace);


	m_logger->info("Engine logging initialized");


}



