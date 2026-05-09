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

	struct IDComponent
	{
		UUID ID; // Uses our new class
	};

	struct TagComponent
	{
		bool IsActive;
		std::string Name = "New Entity";	
	};
		
	struct TransformComponent
	{
		Vec3 Position{ 0.0f, 0.0f, 0.0f };
		Vec3 Rotation{ 0.0f, 0.0f, 0.0f };
		Vec3 Scale{ 1.0f, 1.0f, 1.0f };

		Mat4 LocalTransform = Mat4(1.0f);
		Mat4 WorldTransform = Mat4(1.0f);			
	};

	struct MeshComponent
	{
		UUID MeshHandle; // Just the ID!
		std::vector<UUID> MaterialSlots; // Index matches Submesh index

		bool bCastShadows = true;
		bool bIsVisible = true;
	};

	struct CameraComponent
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
	};

	struct HierachyComponent
	{
		UUID Parent = UUID(0); // 0 means no parent (Root)
		std::vector<UUID> Children;		
	};

	struct PropertyOverride {
		std::string ComponentName; // e.g., "TransformComponent"
		std::string PropertyName;  // e.g., "Translation"		
	};

	struct PrefabComponent
	{ 
		UUID SceneID;
		bool IsDirty = false;
	};

	struct TileMapComponent
	{ };

	struct WorldComponent
	{ };


}



#endif