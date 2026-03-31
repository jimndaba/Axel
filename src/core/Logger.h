#pragma once
#ifndef LOGGER_H

#include "core/Core.h"
#include "UUID.h"

template <>
struct fmt::formatter<Axel::UUID>
{
	constexpr auto parse(fmt::format_parse_context& ctx)
	{
		return ctx.begin();
	}

	template <typename FormatContext>
	auto format(const Axel::UUID& id, FormatContext& ctx) const
	{
		return fmt::format_to(ctx.out(), "{:016x}", (uint64_t)id);
	}
};

namespace Axel {

	class AX_API Logger
	{
		static std::shared_ptr<spdlog::logger> m_logger;
	public:
		static void Init();

		// Loggers for engine and client/game code
		static spdlog::logger* GetLogger()
		{
			return m_logger.get();
		}
	};
}

#define AXLOG_INFO(...)   ::Axel::Logger::GetLogger()->info(__VA_ARGS__)
#define AXLOG_WARN(...)    ::Axel::Logger::GetLogger()->warn(__VA_ARGS__)
#define AXLOG_ERROR(...)   ::Axel::Logger::GetLogger()->error(__VA_ARGS__)
#define AXLOG_TRACE(...)   ::Axel::Logger::GetLogger()->trace(__VA_ARGS__)

#endif

