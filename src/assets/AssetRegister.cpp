#include "axelpch.h"
#include "AssetRegister.h"
#include <yaml-cpp/yaml.h>
#include <fstream>

#include "Serialisation/Archive.h"
#include <core/Reflection.h>

struct Axel::AssetRegister::RegistryImpl {
    std::unordered_map<Axel::UUID, AssetMetadata> Data;
};

const Axel::AssetMetadata& Axel::AssetRegister::GetMetadata(UUID id) const
{
    static const AssetMetadata s_Empty{};
    auto it = m_Registry.find(id);
    return (it != m_Registry.end()) ? it->second : s_Empty;
}

void Axel::AssetRegister::Serialize(IArchive& ar)
{
    if (ar.BeginStruct("AssetRegister"))
    {
        if (ar.GetMode() == ArchiveModeOptions::Save)
        {
            ar.BeginCollection("Assets", m_AssetCount);
            for (auto& [id, metadata] : m_Registry)
            {
                Reflect(metadata, ar);
            }
            ar.EndCollection();
        }
        else
        {
            m_Registry.clear(); // Fresh start for the address book

            if (ar.BeginCollection("Assets", m_AssetCount))
            {
                while (ar.HasNext())
                {                    
                    if (ar.BeginStruct("AssetMetaData"))
                    {
                        AssetMetadata metadata;
                        Reflect(metadata, ar);
                        m_Registry[metadata.AssetID] = metadata;
                        ar.EndStruct();
                    }
                    ar.NextItem();
                }

                ar.EndCollection();
            }
        }
        ar.EndStruct();
    }
}

void Axel::AssetMetadata::Serialize(IArchive& ar) 
{
}