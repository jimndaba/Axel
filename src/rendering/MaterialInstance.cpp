#include "axelpch.h"
#include "MaterialInstance.h"
#include <assets/AssetManager.h>

static Axel::MaterialPropertyValue GetDefaultValueForType(Axel::PropertyType type) {
    switch (type) {
    case Axel::PropertyType::Bool:   return false;
    case Axel::PropertyType::Int:    return (int32_t)0;
    case Axel::PropertyType::Float:  return 0.0f;
    case Axel::PropertyType::Vec2: return glm::vec2(0.0f);
    case Axel::PropertyType::Vec3: return glm::vec3(0.0f);
    case Axel::PropertyType::Vec4: return glm::vec4(1.0f); // Default Albedo to White
    //case Axel::PropertyType::Mat3:   return glm::mat3(1.0f);
    //case Axel::PropertyType::Mat4:   return glm::mat4(1.0f);
        // Textures default to a null/invalid UUID
    case Axel::PropertyType::None:
    default:                     return float(0.0f);
    }
}

Axel::MaterialInstance::MaterialInstance(UUID templateID):
    m_TemplateID(templateID)
{
    auto mt = AssetManager::GetAsset<MaterialTemplate>(m_TemplateID);
    if (mt) {
        // Initialize the map with the schema from the Template
        for (const auto& desc : mt->GetDescriptors()) {
            m_Properties[desc.Name] = GetDefaultValueForType(desc.Type);
        }
    }
}

void Axel::MaterialInstance::PackData(uint8_t* destination)
{
    auto mt = AssetManager::GetAsset<MaterialTemplate>(m_TemplateID);
    if (!mt) return;

    for (const auto& desc : mt->GetDescriptors()) {
        auto it = m_Properties.find(desc.Name);
        if (it == m_Properties.end()) continue;

        uint8_t* target = destination + desc.Offset;

        std::visit([target](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, UUID>) {
                // For now, treat UUID as a 64-bit handle or resolve to index
                memcpy(target, &arg, sizeof(UUID));
            }
            else if constexpr (std::is_same_v<T, bool>) {
                // Vulkan/GLSL bools are usually 4 bytes (uint32_t) for alignment
                uint32_t b = arg ? 1 : 0;
                memcpy(target, &b, sizeof(uint32_t));
            }
            else {
                // Standard math types (Vec2, Vec3, Vec4, etc.)
                memcpy(target, &arg, sizeof(T));
            }
            }, it->second);
    }
}

void Axel::MaterialInstance::Serialize(IArchive& archive)
{
    archive.Property("TemplateID", m_TemplateID);
    for (auto& [name, value] : m_Properties)
    {
        std::visit([&](auto& arg)
            {
                archive.Property(name.c_str(), arg);
            }, value);
    }
}
