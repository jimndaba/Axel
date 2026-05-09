#pragma once
#ifndef MESH_H
#define MESH_H

#include "core/Core.h"
#include "math/Math.h"
#include "GraphicsCore.h"
#include <core/UUID.h>
#include <assets/AssetBase.h>

namespace Axel
{
	const int MAX_BONE_INFLUENCE = 4;

	struct  AX_API Vertex
	{
	};

	struct AX_API  StaticVertex : Vertex
	{
		// Static & Common
		Vec3 Position;
		Vec3 Normal;
		Vec2 TexCoord;
		Vec3 Tangent;   
		Vec4 VertexColor;
	};

	struct AX_API  SkeletalVertex : Vertex
	{
		// Static & Common
		Vec3 Position;
		Vec3 Normal;
		Vec2 TexCoord;
		Vec3 Tangent;

		// Skinned Meshes
		int BoneIDs[MAX_BONE_INFLUENCE];
		float Weights[MAX_BONE_INFLUENCE];

		// Vegetation / Extra Data
		// VertexColor.a can be used for wind-sway mask
		Vec4 VertexColor;
	};

	struct AX_API MeshAllocation {
		uint32_t VertexOffset;
		uint32_t IndexOffset;
		uint32_t IndexCount;
	};

	struct Submesh {
		uint32_t IndexOffset; // Relative to the MeshAllocation's start
		uint32_t VertexOffset;
		uint32_t IndexCount;
		uint32_t MaterialIndex;  // The "Single Source of Truth" link
	};

	template<typename T>
	class AX_API Mesh : public IAsset
	{
	public:
		Mesh(MeshTypeOptions options)
			:MeshType(options){ }

		Mesh(MeshTypeOptions options, MeshAllocation allocation)
			: MeshType(options), m_Allocation(allocation) {}

		MeshTypeOptions MeshType;
		MeshAllocation GetAllocation() const { return m_Allocation; }

		std::vector<T> m_Vertices;
		std::vector<Submesh> m_Submeshes;
		std::vector<uint32_t> m_Indices;
		std::vector<UUID> m_Materials;
		MeshAllocation m_Allocation;

		AssetTypeOptions GetType() const override { return AssetTypeOptions::Mesh; }
	
	};

}


#endif



