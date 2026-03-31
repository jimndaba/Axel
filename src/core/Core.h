#pragma once
#ifndef CORE_H
#define CORE_H

#include <memory>
#include <string>
#include <map>
#include <spdlog/spdlog.h>
#include <spdlog/formatter.h>
#include <spdlog/fmt/fmt.h>

namespace Axel
{
	#if defined(_WIN32)
		#define AX_PLATFORM_WINDOWS
	#endif

	#ifdef AX_PLATFORM_WINDOWS
		// 1. ENGINE LINKAGE (Logger, EventBus, etc.)
	#ifdef AX_BUILD_ENGINE
		#define AX_API __declspec(dllexport)
	#else
		#define AX_API __declspec(dllimport)
	#endif

	// 2. GAME LINKAGE (IGame, CreateGame)
	#ifdef AX_BUILD_GAME_DLL
		#define AX_GAME_API __declspec(dllexport)
	#else
		#define AX_GAME_API __declspec(dllimport)
	#endif
	#else
	#define AX_API
	#define AX_GAME_API
	#endif

#ifdef AXEL_ENABLE_ASSERTS
	// The internal macro that does the work
#define AXEL_INTERNAL_ASSERT_IMPL(type, check, msg, ...) \
        if(!(check)) { \
            AXEL##type##ERROR("Assertion Failed: {0}", msg); \
            AXEL##type##ERROR("At: {0}:{1}", std::filesystem::path(__FILE__).filename().string(), __LINE__); \
            AXEL_DEBUGBREAK(); \
        }

	// Two versions: Core (Engine) and Client (Game)
#define AXEL_CORE_ASSERT(check, ...) AXEL_INTERNAL_ASSERT_IMPL(_CORE_, check, __VA_ARGS__)
#define AXEL_ASSERT(check, ...)      AXEL_INTERNAL_ASSERT_IMPL(_, check, __VA_ARGS__)
#else
#define AXEL_CORE_ASSERT(check, ...)
#define AXEL_ASSERT(check, ...)
#endif

#ifdef AX_DEBUG
	const bool enableValidationLayers = true;
#else
	const bool enableValidationLayers = false;
#endif



	// Ref = Reference Counted (Shared)
	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	// Scope = Unique Ownership
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	enum class ImageFormat {
		None = 0,
		R8,
		RGB8,
		RGBA8,
		RGBA16F,
		RGBA32F,
		Depth24Stencil8,
		Depth32F
	};
}


#endif 