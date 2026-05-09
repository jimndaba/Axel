#include "axelpch.h"
#include "AssetManager.h"
#include "Serialisation/JsonSaveArchive.h"
#include "Serialisation/JsonLoadArchive.h"

Axel::AssetManager* Axel::AssetManager::s_Instance = nullptr;

Axel::AssetManager::AssetManager(GraphicsDevice* device):
	m_Device(device)
{
	s_Instance = this;
}

void Axel::AssetManager::Initialize(const std::string& projectFile)
{
	SetAssetFilePath(projectFile);
}

void Axel::AssetManager::RegisterAsset(AssetMetadata& meta)
{
	s_Instance->m_Registry.AddAsset(meta.AssetID, meta);
	AXLOG_TRACE("Successfully Imported and Registered Mesh: {} , {}", meta.Name, meta.AssetID);
}

void Axel::AssetManager::RegisterAsset(AssetMetadata& meta,const Ref<IAsset>& asset)
{
	s_Instance->m_Registry.AddAsset(meta.AssetID, meta);
	s_Instance->m_LoadedAssets[meta.AssetID] = asset; 
	AXLOG_TRACE("Successfully Imported and Registered Mesh: {} , {}", meta.Name,meta.AssetID.ToString());
}

Axel::UUID Axel::AssetManager::GetAssetIDByName(const std::string& name)
{
	auto uuid = s_Instance->m_Registry.GetUUIDByName(name);	
	return uuid;
}

void Axel::AssetManager::SaveRegistry()
{
    JsonSaveArchive ar(s_Instance->GetAssetFilePathInternal());
	s_Instance->m_Registry.Serialize(ar);
	ar.Save();
}

void Axel::AssetManager::LoadRegistry()
{
	JsonLoadArchive ar(s_Instance->GetAssetFilePathInternal());
	ar.Load();
	s_Instance->m_Registry.Serialize(ar);

}

bool Axel::AssetManager::HasAsset(const std::string& name)
{
	auto uuid = s_Instance->m_Registry.GetUUIDByName(name);
	if (uuid == UUID::Invalid)
		return false;
	return true;
}

bool Axel::AssetManager::HasAsset(UUID id)
{
	return s_Instance->m_Registry.Contains(id);
}

std::string& Axel::AssetManager::GetAssetFolderPath()
{
	return s_Instance->m_CurrentAssetFolder;
}
