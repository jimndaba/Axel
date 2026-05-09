#pragma once
#ifndef MESHLOADER_H
#define MESHLOADER_H

#include <rendering/Mesh.h>
#include <core/UUID.h>
#include <cgltf/cgltf.h>
#include <core/Logger.h>

namespace Axel
{
	class MaterialTemplate;
	class MaterialInstance;


	class MeshLoader
	{
	public:

		MeshLoader();
		template<typename T>
		static bool LoadGLTF(const std::string& path, Mesh<T>& outMesh);

	private:
		template<typename T>
		static void ExtractAttributes(cgltf_primitive& prim, Mesh<T>& outMesh, Mat4 transform);

		template<typename T>
		static void ExtractIndices(cgltf_primitive& prim, Mesh<T>& outMesh, uint32_t vertexStart);

		static Axel::UUID GetMaterialIndex(cgltf_material* gltfMaterial, const std::string& modelPath);


	};

    template<typename T>
    inline void Axel::MeshLoader::ExtractAttributes(cgltf_primitive& prim, Mesh<T>& outMesh, Mat4 transform) {
        size_t startVertex = outMesh.m_Vertices.size();
        cgltf_accessor* positionAccessor = nullptr;

        for (size_t i = 0; i < prim.attributes_count; ++i) {
            if (prim.attributes[i].type == cgltf_attribute_type_position) {
                positionAccessor = prim.attributes[i].data;
                break;
            }
        }

        if (!positionAccessor) {
            AXLOG_ERROR("Mesh primitive has no POSITION attribute!");
            return;
        }

        size_t vertexCount = positionAccessor->count;
        outMesh.m_Vertices.resize(startVertex + vertexCount);

        // --- STEP 1: Initialize defaults ONCE for this primitive range ---
        for (size_t v = 0; v < vertexCount; ++v) {
            T& vertex = outMesh.m_Vertices[startVertex + v];
            vertex.Position = { 0, 0, 0 };
            vertex.Normal = { 0, 1, 0 };
            vertex.TexCoord = { 0, 0 };
            vertex.Tangent = { 1, 0, 0 };
            vertex.VertexColor = { 1, 1, 1, 1 };
        }

        // --- STEP 2: Fill specific attributes ---
        for (size_t i = 0; i < prim.attributes_count; ++i) {
            cgltf_attribute& attr = prim.attributes[i];
            cgltf_accessor* accessor = attr.data;

            std::vector<float> buffer(accessor->count * cgltf_num_components(accessor->type));
            cgltf_accessor_unpack_floats(accessor, buffer.data(), buffer.size());

            for (size_t v = 0; v < vertexCount; ++v) {
                T& vertex = outMesh.m_Vertices[startVertex + v];
                float* data = &buffer[v * cgltf_num_components(accessor->type)];

                switch (attr.type) {
                case cgltf_attribute_type_position:
                    Vec4 localPos = Vec4(data[0], data[1], data[2], 1.0f);
                    Vec4 worldPos = transform * localPos;
                    vertex.Position = { worldPos.x, worldPos.y, worldPos.z };
                    break;
                case cgltf_attribute_type_normal:
                    Vec3 localNormal = { data[0], data[1], data[2] };
                    vertex.Normal = glm::normalize(Mat3(transform) * localNormal);
                    break;
                case cgltf_attribute_type_texcoord:
                    vertex.TexCoord = { data[0], data[1] };
                    break;
                case cgltf_attribute_type_tangent:
                    vertex.Tangent = { data[0], data[1], data[2] };
                    break;
                case cgltf_attribute_type_color:
                    // Check if 3 or 4 components to avoid out-of-bounds
                    vertex.VertexColor.r = data[0];
                    vertex.VertexColor.g = data[1];
                    vertex.VertexColor.b = data[2];
                    if (cgltf_num_components(accessor->type) == 4)
                        vertex.VertexColor.a = data[3];
                    break;
                }
            }
        }
    }

	template<typename T>
	inline void Axel::MeshLoader::ExtractIndices(cgltf_primitive& prim, Mesh<T>& outMesh, uint32_t vertexOffset)
	{
        if (!prim.indices)
        {
            //for (uint32_t i = 0; i < vertexCount; ++i)
           //     outMesh.m_Indices.push_back(vertexOffset + i);
            return;
        }

        size_t indexCount = prim.indices->count;
        outMesh.m_Indices.reserve(outMesh.m_Indices.size() + indexCount);

        for (size_t i = 0; i < indexCount; ++i) {
            // Keep these LOCAL (starting from 0 for every submesh)
            uint32_t localIndex = static_cast<uint32_t>(cgltf_accessor_read_index(prim.indices, i));
            outMesh.m_Indices.push_back(localIndex);
        }
	}






}


#endif