#pragma once
#ifndef GAMEINTERFACE_H
#define GAMEINTERFACE_H

#include "core/Core.h"
#include "InputState.h"

namespace Axel
{
    struct GameContext {
        class Eventbus* Bus;
        struct Logger* Log;
        class GraphicsContext* Graphics;
        class GraphicsDevice* Device;
		class Renderer* Renderer;
        class MaterialManager* MatManager;
        // Add other core systems here
        // Add other core systems here
    };

    // The interface the DLL must implement
    class IGame {
    public:
        virtual ~IGame() = default;
        virtual void OnLoad(GameContext* ctx) = 0;
        virtual void OnUpdate(float dt, const InputState& input) = 0;
        virtual void OnUnload() = 0;

        GameContext* context;
    };
}

#endif