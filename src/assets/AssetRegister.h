#pragma once
#ifndef ASSETREGISTER_H	
#define ASSETREGISTER_H

#include <core/Core.h>
#include <core/UUID.h>
#include <unordered_map>
#include "AssetBase.h"
#include "ISerialisable.h"

namespace Axel
{
	struct AX_API  AssetMetadata : ISerialisable	
	{
		AssetTypeOptions AssetType = AssetTypeOptions::None; // e.g., Texture, Mesh, Shader
		UUID AssetID;          
		std::string Path;
		std::string Name;
		bool IsDataLoaded = false; // Internal flag for the Manager

		virtual void Serialize(IArchive& ar) override;

		bool IsValid() const { return AssetType != AssetTypeOptions::None && !Path.empty(); }
	};

	class AX_API AssetRegister : public ISerialisable
	{
	public:
		void AddAsset(UUID id, const AssetMetadata& metadata) 
		{ 
			m_NameMap[metadata.Name] = id;
			m_Registry[id] = metadata; 
		}
		bool Contains(UUID id) const { return m_Registry.find(id) != m_Registry.end(); }

		// Find path by ID
		const AssetMetadata& GetMetadata(UUID id) const;

		const std::unordered_map<UUID, AssetMetadata>& GetFullRegistry() const { return m_Registry; }

		int GetAssetCount() const { return (int)m_Registry.size(); }

		virtual void Serialize(IArchive& ar)override;

		UUID GetUUIDByName(const std::string& name) const {
			if (m_NameMap.find(name) != m_NameMap.end()) {
				return m_NameMap.at(name);
			}
			return UUID(0); // Return invalid UUID
		}

		// Fast access for the Editor
		const std::vector<UUID>& GetAssetsOfType(AssetTypeOptions type) const {
			return m_TypeCache.at(type);


		}

	private:
		std::unordered_map<std::string, UUID> m_NameMap;
		std::unordered_map<UUID, AssetMetadata> m_Registry;
		std::unordered_map<AssetTypeOptions, std::vector<UUID>> m_TypeCache; // For quick lookups by type
		
		struct RegistryImpl; // Forward declaration
		RegistryImpl* m_Impl = nullptr; // Pimpl hides the yaml-cpp structures
	};
}



#endif