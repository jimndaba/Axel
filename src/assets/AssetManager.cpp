#include "axelpch.h"
#include "AssetManager.h"
#include "Serialisation/YAMLArchive.h"

Axel::AssetManager* Axel::AssetManager::s_Instance = nullptr;

Axel::AssetManager::AssetManager(GraphicsDevice* device):
	m_Device(device)
{
	s_Instance = this;
}

void Axel::AssetManager::Initialize(const std::string& projectFile)
{
}

void Axel::AssetManager::RegisterAsset(AssetMetadata& meta)
{
	s_Instance->m_Registry.AddAsset(meta.AssetID, meta);
}

void Axel::AssetManager::SaveRegistry(const std::string& path)
{
    YAMLArchive ar(ArchiveModeOptions::Save, path);
	s_Instance->m_Registry.Serialize(ar);
}

void Axel::AssetManager::LoadRegistry(const std::string& path)
{
	YAMLArchive ar(ArchiveModeOptions::Load, path);
	s_Instance->m_Registry.Serialize(ar);
}
