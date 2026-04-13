#include "axelpch.h"
#include "Application.h"
#include "Timer.h"

#include "rendering/backends/GraphicsContext.h"
#include "rendering/Renderer.h"
#include <rendering/MaterialManager.h>

Axel::Application::Application()
{
	m_Logger = std::make_unique<Logger>();
	m_Logger->Init();
	AXLOG_INFO("Axel Engine: Booting...");

	m_EventBus = std::make_unique<Eventbus>();
	
	m_Platform = std::make_unique<AxPlatform>();
	m_Platform->Init({"Axel Engine:...",1280, 720,false});
	AXLOG_INFO("Platform Layer: %s Detected" , m_Platform->GetPlatformName());

	Init();

	LoadGameDLL("AxelSandbox.dll");
}

Axel::Application::~Application()
{
}

void Axel::Application::Run()
{
	AxTimer timer;
	OnStart();

	while (m_Running)
	{
		float dt = timer.GetDeltaTime();

		if (!m_Platform->ProcessMessages())
		{
			m_Running = false;
		}


		OnUpdate(dt);	

		InputState inputState;
		if(m_ActiveGame)
			m_ActiveGame->OnUpdate(dt,inputState);
	}

	OnShutdown();
	Shutdown();
}

void Axel::Application::Quit()
{
	m_Platform->Shutdown();
	Eventbus::Clear();
}

void Axel::Application::Shutdown()
{	
	if (m_ActiveGame)
	{
		m_ActiveGame->OnUnload();
		delete(m_GameContext);
	}
	Renderer::Shutdown();
	mContext->Shutdown();
}

void Axel::Application::UpdateMaterials()
{
	m_MaterialManager->Update();
}

void Axel::Application::LoadGameDLL(const std::string& path)
{
	m_GameLib = m_Platform->LoadSharedLibrary(path.c_str());
	
	if (m_GameLib) {
		typedef Axel::IGame* (*CreateGameFn)();
		auto CreateGame = (CreateGameFn)m_Platform->GetSymbol(m_GameLib, "CreateGame");
		if (CreateGame) {
			m_ActiveGame = CreateGame();
			m_GameContext = new GameContext();
			m_GameContext->Bus = m_EventBus.get();
			m_GameContext->Log = m_Logger.get();
			m_GameContext->Device = mContext->GetDevice().get();
			m_GameContext->Graphics = mContext.get();
			m_GameContext->Renderer = mRenderer.get();
			m_GameContext->MatManager = m_MaterialManager.get();
			m_ActiveGame->OnLoad(m_GameContext);
		}
	}

}

void Axel::Application::OnStart()
{
	
}

void Axel::Application::OnUpdate(float deltaTime)
{

}

void Axel::Application::OnShutdown()
{
}

void Axel::Application::Init()
{

	// 2. Create the Graphics Context (VulkanContext)
	mContext = GraphicsContext::Create(m_Platform->GetNativeWindow());
	mContext->Init();

	m_AssetaManager = std::make_unique<AssetManager>(mContext->GetDevice().get());	

	m_MaterialManager = std::make_unique<MaterialManager>(mContext.get());

	Axel::AssetMetadata SpriteMeta{};
	SpriteMeta.Name = "SpriteShader";
	SpriteMeta.AssetType = Axel::AssetTypeOptions::Shader;
	SpriteMeta.Path = "Assets/Shaders/Quad";
	Axel::AssetManager::RegisterAsset(SpriteMeta);

	Renderer::Init(mContext.get(), m_MaterialManager.get());
}
