#pragma once
#ifndef APPLICATION_H
#define APPLICATION_H

#include "platform/Platform.h"
#include "GameInterface.h"
#include "core/Logger.h"
#include "events/EventBus.h"



namespace Axel
{
    class GraphicsContext;
    class Renderer;

    class Application {
    public:
        Application();
        virtual ~Application();

        // High-level engine flow
        void Init();
        void Run();
        void Quit();
        void Shutdown();

        // Getters for global access (optional, or use Service Locator)
        static Application& Get() { return *s_Instance; }
        AxPlatform& GetPlatform() { return *m_Platform; }

        void LoadGameDLL(const std::string& path);

    protected:
        // Overridable by the specific game (e.g., MyGame : public Application)
        virtual void OnStart() {}
        virtual void OnUpdate(float deltaTime) {}
        virtual void OnShutdown() {}

    private:
        bool m_Running = true;

        // Ownership: These are destroyed in REVERSE order of declaration
        std::unique_ptr<Logger>     m_Logger;
        std::unique_ptr<Eventbus>   m_EventBus;
        std::unique_ptr<AxPlatform>   m_Platform;

        std::unique_ptr<GraphicsContext> mContext;
        std::unique_ptr<Renderer> mRenderer;

        static Application* s_Instance;
        void* m_GameLib = nullptr;
        IGame* m_ActiveGame = nullptr;
    };
}

#endif