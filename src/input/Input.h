#pragma once
#ifndef INPUT_H
#define INPUT_H

#include <core/Core.h>
#include "KeyCodes.h"
#include <math/Math.h>

namespace Axel
{

    class AX_API Input {
    public:
        // Polling functions
        static bool IsKeyPressed(KeyOptions key);
        static bool IsMouseButtonPressed(MouseButtonOptions button);
        static Vec2 GetMousePosition();

        // Gamepad Polling (Groundwork for future)
        static bool IsGamepadButtonPressed(int joystickId, int button);
        static float GetGamepadAxis(int joystickId, int axis);
        static bool IsGamepadConnected(int joystickId);
    };



}

#endif