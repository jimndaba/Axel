#pragma once
#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <core/Logger.h>
#include "AssetRegister.h"

#include "Loaders/TextureLoader.h"

namespace Axel
{
	class AX_API  AssetManager
	{
    public:
        // The main "Request" function

        // Called on Engine Init
        void Initialize(const std::string& projectFile);

        template<typename T>
        Ref<T> GetAsset(UUID id) {
            // 1. Check if it's already in memory
            if (m_LoadedAssets.find(id) != m_LoadedAssets.end()) {
                return std::static_pointer_cast<T>(m_LoadedAssets[id]);
            }

            // 2. Not in memory, find where it is on disk via the Registry
            AssetMetadata& metadata = m_Registry.GetMetadata(id);
            if (metadata.AssetType == AssetTypeOptions::None) return nullptr;

            // 3. Load it based on type
            Ref<Asset> asset = nullptr;
			if (!metadata.IsValid()) return nullptr;// Sanity check

            switch (metadata.AssetType) {
            case AssetTypeOptions::Texture2D: asset = TextureLoader::Load(metadata.Path); break;
            //case AssetTypeOptions::Mesh:      asset = MeshImporter::Load(metadata.Path); break;
            }

            if (asset) {
                asset->AssetID = id;
                m_LoadedAssets[id] = asset;
                return std::static_pointer_cast<T>(asset);
            }
            return nullptr;
        }
        void SaveRegistry(const std::string& path);
		void LoadRegistry(const std::string& path);



    private:
        /*
        template<typename T>
        Ref<T> LoadAssetInternal(UUID id, const AssetMetadata& metadata) {
            // Here you would switch based on metadata.Type
            // For now, let's assume T has a Load function
            Ref<T> asset = CreateRef<T>();
            if (asset->LoadFromFile(metadata.FilePath)) {
                asset->Handle = id;
                return asset;
            }
            return nullptr;
        }
        */
        AssetRegister m_Registry;
		std::unordered_map<UUID, Ref<Asset>> m_LoadedAssets; // In-memory cache of loaded assets
    };	
}


#endif
