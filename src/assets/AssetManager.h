#pragma once
#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <core/Logger.h>
#include "AssetRegister.h"
#include <rendering/Shader.h>
#include <rendering/Texture.h>
#include <rendering/MaterialTemplate.h>
#include "Loaders/TextureLoader.h"
#include <rendering/backends/GraphicsDevice.h>

namespace Axel
{
    class Shader; 

	class AX_API  AssetManager
	{
    public:
        AssetManager(GraphicsDevice* device);
        // The main "Request" function

        // Called on Engine Init
        void Initialize(const std::string& projectFile);

        static void RegisterAsset(AssetMetadata& meta);

        template<typename T>
        static Ref<T> GetAsset(UUID id) {
            return s_Instance->GetAssetInternal<T>(id);           
        }

        template<typename T>
        static Ref<T> GetAssetByName(const std::string& name) {
            return s_Instance->GetAssetByNameInternal<T>(name);
        }

        static void SaveRegistry(const std::string& path);
		static void LoadRegistry(const std::string& path);

        AssetRegister& GetRegister() { return m_Registry; }
     
    private:

        GraphicsDevice* m_Device  = nullptr;
        static AssetManager* s_Instance; // The only static part

        template<typename T>
        Ref<T> GetAssetInternal(UUID id) {
            // 1. Check if it's already in memory
            if (s_Instance->m_LoadedAssets.find(id) != s_Instance->m_LoadedAssets.end()) {
                return std::static_pointer_cast<T>(s_Instance->m_LoadedAssets[id]);
            }

            // 2. Not in memory, find where it is on disk via the Registry
            const AssetMetadata& metadata = s_Instance->m_Registry.GetMetadata(id);
            if (metadata.AssetType == AssetTypeOptions::None) return nullptr;

            // 3. Load it based on type
            Ref<IAsset> asset = nullptr;
            if (!metadata.IsValid()) return nullptr;// Sanity check

            switch (metadata.AssetType) {
            case AssetTypeOptions::Texture2D:
                asset = std::static_pointer_cast<IAsset>(TextureLoader::Load(metadata.Path));
                break;
            case AssetTypeOptions::Shader:
                asset = std::static_pointer_cast<IAsset>(Shader::Create(*s_Instance->m_Device, metadata.Path));
                break;
            case AssetTypeOptions::MaterialTemplate:
                //asset = std::static_pointer_cast<IAsset>(MaterialTemplateLoader::Load(metadata.Path));
                break;
            }

            if (asset) {
                asset->AssetID = id;
                s_Instance->m_LoadedAssets[id] = asset;
                return std::static_pointer_cast<T>(asset);
            }

            if (metadata.AssetType == AssetTypeOptions::Shader) {
                auto shader = Shader::Create(*m_Device, metadata.Path);
                asset = std::static_pointer_cast<IAsset>(shader);
            }
            return std::static_pointer_cast<T>(asset);
        }

        template<typename T>
        Ref<T> GetAssetByNameInternal(const std::string& name) {
            // 1. Ask the registry to find the UUID associated with this name
            // Assuming your AssetRegister has a GetUUIDByName method
            UUID id = m_Registry.GetUUIDByName(name);

            // 2. If the UUID is null/invalid, the asset doesn't exist in the registry
            // 3. Reuse the existing UUID-based logic (which handles caching and loading)
            return GetAsset<T>(id);
        }
        AssetRegister m_Registry;
        std::unordered_map<UUID, Ref<IAsset>> m_LoadedAssets; // In-memory cache of loaded assets
    };	
}


#endif
