#pragma once
#include <cstdint>
#ifndef KEYCODES_H
#define KEYCODES_H


namespace Axel {
    enum class KeyOptions : uint16_t {
        None = 0,
        // Alphanumeric
        A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        D0 = 48, D1, D2, D3, D4, D5, D6, D7, D8, D9,
        // Controls
        Space = 32, Escape = 256, Enter = 257, Tab = 258,
        LeftControl = 341, LeftShift = 340, LeftAlt = 342,
        // ... add as needed
    };

    enum class MouseButtonOptions : uint16_t {
        Left = 0, Right = 1, Middle = 2, Button3, Button4, Button5
    };
}

#endif