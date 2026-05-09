#include "axelpch.h"
#include "MeshLoader.h"
#include <assets/Serialisation/JsonSaveArchive.h>
#include <rendering/MaterialInstance.h>
#include <math/Math.h>
#include <assets/AssetManager.h>



Axel::MeshLoader::MeshLoader()
{
}

template<typename T>
bool Axel::MeshLoader::LoadGLTF(const std::string& path, Mesh<T>& outMesh) {
    cgltf_options options = {};
    cgltf_data* data = nullptr;
    cgltf_result result = cgltf_parse_file(&options, path.c_str(), &data);

    if (result != cgltf_result_success) return false;

    cgltf_load_buffers(&options, data, path.c_str());

    outMesh.m_Materials.reserve(data->materials_count);
    for (size_t i = 0; i < data->materials_count; ++i) {
        outMesh.m_Materials.push_back(GetMaterialIndex(&data->materials[i], path));
    }

    for (size_t i = 0; i < data->nodes_count; ++i) {
        cgltf_node& node = data->nodes[i];

        // Only process nodes that actually have a mesh attached
        if (!node.mesh) continue;

        // 1. Manually construct the transform matrix from cgltf
        float m[16];
        cgltf_node_transform_world(&node, m);

        // Assuming your Mat4 constructor takes 16 floats or 4 Vec4s
        // glTF matrices are column-major
        Mat4 transform(
            Vec4(m[0], m[1], m[2], m[3]),
            Vec4(m[4], m[5], m[6], m[7]),
            Vec4(m[8], m[9], m[10], m[11]),
            Vec4(m[12], m[13], m[14], m[15])
        );

        // 2. Only iterate through the primitives of THIS node's mesh
        for (size_t j = 0; j < node.mesh->primitives_count; ++j) {
            cgltf_primitive& prim = node.mesh->primitives[j];

            Submesh sub;
            sub.IndexOffset = static_cast<uint32_t>(outMesh.m_Indices.size());
            uint32_t vertexStartForThisPrim = static_cast<uint32_t>(outMesh.m_Vertices.size());
            sub.VertexOffset = vertexStartForThisPrim;

            // Pass the transform to your attribute extractor
            ExtractAttributes(prim, outMesh, transform);
            ExtractIndices(prim, outMesh, vertexStartForThisPrim);

            sub.IndexCount = static_cast<uint32_t>(outMesh.m_Indices.size()) - sub.IndexOffset;

            if (prim.material) {
                sub.MaterialIndex = static_cast<uint32_t>(prim.material - data->materials);
            }
            else {
                sub.MaterialIndex = 0;
            }

            outMesh.m_Submeshes.push_back(sub);
        }
    }
    cgltf_free(data);
    return true;
}

Axel::UUID Axel::MeshLoader::GetMaterialIndex(cgltf_material* gltfMaterial, const std::string& modelPath) {
    // TODO if (!gltfMaterial) return AssetManager::GetDefaultMaterialID();

    std::string instanceName = gltfMaterial->name ? gltfMaterial->name : "unnamed";

    // Check if instance already exists
    if (AssetManager::HasAsset(instanceName)) {
        return AssetManager::GetAssetIDByName(instanceName);
    }

    // 1. Fetch the Template (e.g., the "Standard_PBR" Template)
    Ref<MaterialTemplate> pbrTemplate = AssetManager::GetAssetByName<MaterialTemplate>("Standard_PBR_Template");

    // 2. Create a Material Instance (The actual data container)
    Ref<MaterialInstance> instance = pbrTemplate->CreateInstance(instanceName);
    
    // 3. Map glTF properties to the Template's Descriptors
    if (gltfMaterial->has_pbr_metallic_roughness) {
        auto& pbr = gltfMaterial->pbr_metallic_roughness;
        // Use the Descriptors to find WHERE to write in the instance buffer
        instance->Set("u_AlbedoColour", Vec4(pbr.base_color_factor[0]));
        instance->Set("u_Metallic", pbr.metallic_factor);
        instance->Set("u_Roughness", pbr.roughness_factor);

        if (pbr.base_color_texture.texture) {
            //TEXTURE LOADING TO DO
            //std::string texPath = ResolveTexturePath(modelPath, pbr.base_color_texture.texture);

            // Load as a formal Texture Asset
            //UUID texID = AssetManager::GetOrCreateTextureAsset(texPath);

            // Set the texture property in the instance
            //instance->Set("u_AlbedoMap", texID);
        }
    }

    std::string asset_folder =  AssetManager::GetAssetFolderPath() +  "/Materials/";
    std::string asset_path = asset_folder + instanceName + AssetManager::GetMaterialsFileExtension();

    AssetMetadata meta;
    meta.AssetID = instance->AssetID;
    meta.AssetType = AssetTypeOptions::Material;
    meta.Name = instanceName;
    meta.Path = asset_path;
    AssetManager::RegisterAsset(meta,instance);

   
    JsonSaveArchive ar(asset_path);
    instance->Serialize(ar);
    ar.Save();
    return instance->AssetID;
}

template bool Axel::MeshLoader::LoadGLTF<Axel::StaticVertex>(const std::string&, Mesh<StaticVertex>&);
template bool Axel::MeshLoader::LoadGLTF<Axel::SkeletalVertex>(const std::string&, Mesh<SkeletalVertex>&);