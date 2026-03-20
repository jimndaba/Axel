#pragma once
#ifndef IPLATFORM_H
#define IPLATFORM_H

#include "core/Core.h"
#include "core/InputState.h"
#include "events/EventBus.h"

namespace Axel
{
	struct PlatformDesc {
		const char* WindowTitle;
		uint32_t DefaultWidth;
		uint32_t DefaultHeight;
		bool Fullscreen;
	};

	struct WindowResizeEvent : IEvent
	{
		uint32_t Width;
		uint32_t Height;
		WindowResizeEvent(int w, int h) : Width(w), Height(h) {}
	};

	class AxPlatform
	{
	public:
		~AxPlatform() = default;

		bool Init(const PlatformDesc& desc);
		void Shutdown();
		bool ProcessMessages(); // Returns false if the app should quit

		void PollInput(InputState& state);

		void* GetNativeWindow();
		void GetDisplayMetrics(int& outWidth, int& outHeight) const;
		std::string GetPlatformName() const;
		void OnWindowResize(std::shared_ptr<WindowResizeEvent>& evnt);

		void* LoadSharedLibrary(const char* path);
		void* GetSymbol(void* handle, const char* symbol);
		void UnloadSharedLibrary(void* handle);

	private:
		void* mNativeWindowHandle;
		uint32_t mCurrentWidth;
		uint32_t mCurrentHeight;
		PlatformDesc mDescription;
	};

}

#endif