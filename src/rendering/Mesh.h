#pragma once
#ifndef MESH_H
#define MESH_H

#include "core/Core.h"
#include "Buffer.h"
#include "math/Math.h"

namespace Axel
{
	const int MAX_BONE_INFLUENCE = 4;
	struct Vertex
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

	class Mesh
	{
	public:
		Mesh();
	};

}


#endif



