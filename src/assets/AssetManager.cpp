#include "axelpch.h"
#include "AssetManager.h"
#include "Serialisation/YAMLArchive.h"

void Axel::AssetManager::Initialize(const std::string& projectFile)
{
}

void Axel::AssetManager::SaveRegistry(const std::string& path)
{
    YAMLArchive ar(ArchiveModeOptions::Save, path);
	m_Registry.Serialize(ar);
}

void Axel::AssetManager::LoadRegistry(const std::string& path)
{
	YAMLArchive ar(ArchiveModeOptions::Load, path);

}
