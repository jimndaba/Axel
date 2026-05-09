#include "axelpch.h"
#include "Input.h"

#if defined(AX_PLATFORM_WINDOWS)

#include <GLFW/glfw3.h>
#include <core/Application.h>


bool Axel::Input::IsKeyPressed(KeyOptions key)
{
    auto* window = static_cast<GLFWwindow*>(Application::Get().GetPlatform().GetNativeWindow());
    auto state = glfwGetKey(window, static_cast<int32_t>(key));
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Axel::Input::IsMouseButtonPressed(MouseButtonOptions button)
{
    auto* window = static_cast<GLFWwindow*>(Application::Get().GetPlatform().GetNativeWindow());
    auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
    return state == GLFW_PRESS;
}
Axel::Vec2 Axel::Input::GetMousePosition()
{
    auto* window = static_cast<GLFWwindow*>(Application::Get().GetPlatform().GetNativeWindow());
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    return { (float)xpos, (float)ypos };
}

// Gamepad Polling (Groundwork for future)
bool Axel::Input::IsGamepadButtonPressed(int joystickId, int button)
{
    return false;
}
float Axel::Input::GetGamepadAxis(int joystickId, int axis)
{
    return 0.0f;
}
bool Axel::Input::IsGamepadConnected(int joystickId)
{
    return false;
}



#endif