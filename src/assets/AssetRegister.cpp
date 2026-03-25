#include "axelpch.h"
#include "AssetRegister.h"
#include <yaml-cpp/yaml.h>
#include <fstream>

#include "Serialisation/Archive.h"

struct Axel::AssetRegister::RegistryImpl {
    std::unordered_map<Axel::UUID, AssetMetadata> Data;
};

const Axel::AssetMetadata& Axel::AssetRegister::GetMetadata(UUID id) const
{
	auto it = m_Registry.find(id);
	if (it != m_Registry.end()) {
        return it->second;
    }
	return AssetMetadata(); // Return a default if not found, or consider throwing an exception
}

void Axel::AssetRegister::Serialize(IArchive& ar)
{
    uint32_t count = (uint32_t)m_Registry.size();
    if (ar.BeginCollection("Assets", count)) 
    {       
        if (ar.GetMode() == ArchiveModeOptions::Save)
        {
            for (auto& [id, metadata] : m_Registry) 
            {
                metadata.Serialize(ar);
                ar.NextItem();               
            }
        }
        else // Load mode
        {
            m_Registry.clear(); // Fresh start for the address book

            for (uint32_t i = 0; i < count; ++i)
            {                
                AssetMetadata metadata;
                metadata.Serialize(ar);
                m_Registry[metadata.AssetID] = metadata; // Use the deserialized ID as the key
                ar.NextItem();
            }

        }
        ar.EndCollection();
    }

}

void Axel::AssetMetadata::Serialize(IArchive& ar) 
{
	int typeVal = (int)AssetType;
    ar.BeginStruct("Asset");   
    ar.Property("ID", AssetID);
    ar.Property("Name", Name);    
    ar.Property("Path", Path);   
    ar.Property("Type", typeVal);

    // 3. THE CRITICAL STEP: Re-assign back to the class during Load
    if (ar.GetMode() == ArchiveModeOptions::Load)
    {
        AssetType = (AssetTypeOptions)typeVal;
    }

    ar.EndStruct();
}