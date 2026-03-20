#pragma once
#ifndef INPUTSTATE_H
#define INPUTSTATE_H

#include <cstdint>

namespace Axel
{

    enum class DigitalButton {
        ActionDown, ActionRight, ActionLeft, ActionUp,
        L1, R1, L2, R2, Select, Start,
        DpadUp, DpadDown, DpadLeft, DpadRight,
        Count
    };

    struct TouchPoint {
        uint32_t ID;
        float X, Y;
        bool IsActive;
    };

    struct InputState {
        // --- Digital State (Buttons/Keyboard) ---
        // A bitset representing the current "Down" state of all buttons
        uint64_t ButtonsDown;
        uint64_t ButtonsPressedThisFrame;

        // --- Analog State (Sticks/Triggers) ---
        struct {
            float LeftStickX, LeftStickY;   // Range -1.0 to 1.0
            float RightStickX, RightStickY;
            float LeftTrigger, RightTrigger; // Range 0.0 to 1.0
        } Controllers[4]; // Support for local multiplayer (Xbox/Switch)

        // --- Pointer State (Mouse/Touch) ---
        float MouseX, MouseY;
        float MouseWheelDelta;
        TouchPoint Touches[10]; // Multi-touch support for Android/Switch Screen

        // --- Motion/Sensor (Switch/Android/Xbox Mobile) ---
        struct {
            float GyroX, GyroY, GyroZ;
            float AccelX, AccelY, AccelZ;
        } Motion;

        // Helper to check state
        bool IsButtonDown(DigitalButton btn) const {
            return (ButtonsDown & (1ULL << (int)btn)) != 0;
        }
    };
}

#endif