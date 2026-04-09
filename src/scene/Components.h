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
			ar.Property("Name", Name);			
		}
	};

	struct SpriteComponent : ISerialisable
	{
		UUID TextureHandle; // Just the ID!
		Vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };

		// This makes the Inspector UI easy to build later
		void Serialize(IArchive& ar) {		
			ar.Property("Texture", TextureHandle);
			ar.Property("Color", Color);		
		}
	};

	struct TransformComponent : ISerialisable
	{
		Vec3 Position{ 0.0f, 0.0f, 0.0f };
		Vec3 Rotation{ 0.0f, 0.0f, 0.0f };
		Vec3 Scale{ 1.0f, 1.0f, 1.0f };

		Mat4 LocalTransform = Mat4(1.0f);
		Mat4 WorldTransform = Mat4(1.0f);

		void Serialize(IArchive& ar) {		
			ar.Property("Position", Position);
			ar.Property("Rotation", Rotation);
			ar.Property("Scale", Scale);		
		}
	};

	struct MeshComponent : ISerialisable
	{
		UUID MeshHandle; // Just the ID!
		void Serialize(IArchive& ar) {		
			ar.Property("Mesh", MeshHandle);	
		}
	};

	struct CameraComponent : ISerialisable
	{
		enum class ProjectionTypeOptions { Perspective = 0, Orthographic = 1 };
		ProjectionTypeOptions ProjectionType = ProjectionTypeOptions::Orthographic;

		// Perspective properties
		float PerspectiveFOV = 45.0f;
		float PerspectiveNear = 0.01f, PerspectiveFar = 1000.0f;

		// Orthographic properties
		float OrthoSize = 10.0f;
		float OrthoNear = -1.0f, OrthoFar = 1.0f;

		bool Primary = true; // Is this the camera we render from?
		float AspectRatio = 1.778f; // 16:9 default

		Mat4 Projection = Mat4(1.0f);

		CameraComponent() { CalculateProjection(); }

		void CalculateProjection()
		{
			if (ProjectionType == ProjectionTypeOptions::Perspective)
			{
				Projection = Math::Perspective(Math::Radians(PerspectiveFOV), AspectRatio, PerspectiveNear, PerspectiveFar);
			}
			else
			{
				float orthoLeft = -OrthoSize * AspectRatio * 0.5f;
				float orthoRight = OrthoSize * AspectRatio * 0.5f;
				float orthoBottom = -OrthoSize * 0.5f;
				float orthoTop = OrthoSize * 0.5f;

				Projection = Math::Ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, OrthoNear, OrthoFar);
			}
		}

		void Serialize(IArchive& ar) override {			
			ar.Property("ProjectionType", (int&)ProjectionType);
			ar.Property("FOV", PerspectiveFOV);
			ar.Property("OrthoSize", OrthoSize);
			ar.Property("Primary", Primary);
		}
	};

	struct HierarchyComponent : ISerialisable
	{
		UUID Parent = UUID(0); // 0 means no parent (Root)
		std::vector<UUID> Children;

		void Serialize(IArchive& ar) override {			
			ar.Property("Parent", Parent);
			ar.Property("Children", Children);
		}
	};

}



#endif