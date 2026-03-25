#pragma once
#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "core/UUID.h"
#include <string>
#include <math/Math.h>
#include <assets/ISerialisable.h>
#include <assets/Serialisation/Archive.h>

namespace Axel
{

	struct IDComponent : ISerialisable
	{
		UUID ID; // Uses our new class
		void Serialize(IArchive& ar) {
			ar.BeginStruct("IDComponent");
			ar.Property("EntityID", ID);
			ar.EndStruct();
		}
	};

	struct TagComponent : ISerialisable
	{
		std::string Name;
		void Serialize(IArchive& ar) {
			ar.BeginStruct("TagComponent");
			ar.Property("Name", Name);
			ar.EndStruct();
		}
	};

	struct SpriteComponent : ISerialisable
	{
		UUID TextureHandle; // Just the ID!
		Vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };

		// This makes the Inspector UI easy to build later
		void Serialize(IArchive& ar) {
			ar.BeginStruct("SpriteComponent");
			ar.Property("Texture", TextureHandle);
			ar.Property("Color", Color);
			ar.EndStruct();
		}
	};

	struct TransformComponent : ISerialisable
	{
		Vec3 Position{ 0.0f, 0.0f, 0.0f };
		Vec3 Rotation{ 0.0f, 0.0f, 0.0f };
		Vec3 Scale{ 1.0f, 1.0f, 1.0f };
		void Serialize(IArchive& ar) {
			ar.BeginStruct("TransformComponent");
			ar.Property("Position", Position);
			ar.Property("Rotation", Rotation);
			ar.Property("Scale", Scale);
			ar.EndStruct();
		}
	};

	struct MeshComponent : ISerialisable
	{
		UUID MeshHandle; // Just the ID!
		void Serialize(IArchive& ar) {
			ar.BeginStruct("MeshComponent");
			ar.Property("Mesh", MeshHandle);
			ar.EndStruct();
		}
	};


}



#endif