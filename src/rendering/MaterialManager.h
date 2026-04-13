#pragma once
#ifndef MATERIALMANAGER_H
#define MATERIALMANAGER_H

#include <core/Core.h>
#include <core/UUID.h>
#include <rendering/Buffers.h>

namespace Axel
{
    class MaterialInstance;
    class GraphicsContext;

	struct MaterialMetadata {
		uint32_t Offset; // Offset in the global GPU buffer
		uint32_t Size;   // Size of this specific material block
        uint32_t Index;
	};


	class AX_API MaterialManager
	{
    public:
        MaterialManager(GraphicsContext* ctxt);
        ~MaterialManager() = default;

        // Register an instance to be tracked and uploaded
        void RegisterMaterial(Ref<MaterialInstance> instance);

        // The heart of the system: Syncs CPU data to GPU
        void Update();

        uint32_t GetMaterialIndex(UUID id);

        // Used by the Renderer to bind the global material table
        Ref<ShaderStorageBuffer> GetMaterialBuffer() const { return m_MaterialTableBuffer; }

    private:
        GraphicsContext* m_Context;

        // Mapping of Instance UUID -> Metadata (where it lives in the SSBO)
        std::unordered_map<UUID, MaterialMetadata> m_MaterialRegistry;
        std::vector<Ref<MaterialInstance>> m_ActiveInstances;

        Ref<ShaderStorageBuffer> m_MaterialTableBuffer;
        std::vector<uint8_t> m_CPUBuffer; // Local "staging" memory

        bool m_NeedsReallocation = false;
        bool m_IsDirty = true;

	};






}

#endif