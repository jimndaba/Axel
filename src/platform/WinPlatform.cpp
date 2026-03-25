#include "axelpch.h"
#include "core/Core.h"
#include "Platform.h"

#if defined(AX_PLATFORM_WINDOWS)


#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h> // Include Windows first
#include <GLFW/glfw3.h>
#include <libloaderapi.h>



namespace Axel 
{
	void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
		// Retrieve the pointer to our Platform class stored in the window user pointer
		auto* platform = static_cast<AxPlatform*>(glfwGetWindowUserPointer(window));

		// 2. Publish to your Fracture Eventbus
		// This notifies the Renderer, Camera, and UI systems simultaneously
		Axel::Eventbus::Publish<WindowResizeEvent>(width, height);
	}

	bool AxPlatform::Init(const PlatformDesc& desc)
	{
		mCurrentHeight = desc.DefaultHeight;
		mCurrentWidth = desc.DefaultWidth;

		if (!glfwInit())
			return 0;

		/* Create a windowed mode window and its OpenGL context */
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		mNativeWindowHandle = glfwCreateWindow(mCurrentWidth, mCurrentHeight, desc.WindowTitle, NULL, NULL);
		if (!mNativeWindowHandle)
		{
			glfwTerminate();
			return false;
		}
		glfwSetFramebufferSizeCallback((GLFWwindow*)mNativeWindowHandle, FramebufferSizeCallback);
		/* Make the window's context current */
		glfwMakeContextCurrent((GLFWwindow*)mNativeWindowHandle);
		return 1;
	}

	void AxPlatform::Shutdown()
	{
		glfwTerminate();
	}

	bool AxPlatform::ProcessMessages()
	{
		glfwPollEvents();
		bool shouldClose = glfwWindowShouldClose(static_cast<GLFWwindow*>(mNativeWindowHandle));
		return !shouldClose;
	}

	void AxPlatform::PollInput(InputState& currentstate)
	{
		InputState previousInput = currentstate;

		// 2. Get Windows Raw Input (Simplified)
		currentstate.ButtonsDown = 0;
	}

	void* AxPlatform::GetNativeWindow()
	{
		return  mNativeWindowHandle;
	}

	void AxPlatform::GetDisplayMetrics(int& outWidth, int& outHeight) const
	{
		outHeight = mCurrentHeight;
		outWidth = mCurrentWidth;
	}

	std::string AxPlatform::GetPlatformName() const
	{
		return "Win32";
	}

	void AxPlatform::OnWindowResize(std::shared_ptr<WindowResizeEvent>& evnt)
	{
		mCurrentWidth = evnt->Width;
		mCurrentHeight = evnt->Height;
	}

	void* AxPlatform::LoadSharedLibrary(const char* path)
	{
		return static_cast<void*>(LoadLibraryA(path));
	}

	void* AxPlatform::GetSymbol(void* handle, const char* symbol)
	{
		if (!handle || !symbol) {
			// m_Logger->Error("Invalid handle or symbol name passed to GetSymbol");
			return nullptr;
		}

		// Cast the opaque handle back to a Windows HMODULE
		HMODULE module = static_cast<HMODULE>(handle);

		// GetProcAddress returns a FARPROC (function pointer)
		// We cast it to void* to keep it platform-agnostic for the interface
		void* address = reinterpret_cast<void*>(GetProcAddress(module, symbol));

		if (!address) {
			// Log the specific Windows error code for debugging
			// DWORD error = GetLastError();
		}

		return address;
	}

	void AxPlatform::UnloadSharedLibrary(void* handle)
	{
		if (handle) {
			HMODULE module = static_cast<HMODULE>(handle);
			FreeLibrary(module);
		}
	}

}

#endif 
