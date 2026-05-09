#pragma once
#ifndef REFLECTION_H
#define REFLECTION_H

#include <assets/Serialisation/Archive.h>
#include <scene/Components.h>
#include <assets/AssetRegister.h>

namespace Axel
{	

	template<typename ArchiveType, typename PropertyType>
	struct ArchiveTraits
	{
		// Does the archive have a direct Property() overload for this type?
		// Default: NO. Archives opt-in by specializing this struct.
		static constexpr bool SupportsDirectProperty = false;
	};

	template<typename T, typename ArchiveType>
	void VisitProperty(const char* name, T& value, ArchiveType& ar)
	{
		if constexpr (
			std::is_arithmetic_v<T> ||
			std::is_same_v<T, std::string> ||
			std::is_same_v<T, UUID> ||
			ArchiveTraits<ArchiveType, T>::SupportsDirectProperty
			)
		{
			ar.Property(name, value);
		}
		else
		{
			ar.BeginStruct(name);
			Reflect(value, ar);
			ar.EndStruct();
		}
	}

	template<typename T>
	void VisitProperty(const char* name, const T& value, IArchive& ar)
	{
		using MutableT = std::remove_const_t<T>;

		MutableT copy = value;

		VisitProperty(name, copy, ar);
	}

	template<typename T, typename ArchiveType>
	void VisitProperty(const char* name, const char* itemName, std::vector<T>& vec, ArchiveType& ar)
	{
		uint32_t size = (uint32_t)vec.size();

		if (ar.GetMode() == ArchiveModeOptions::Load)
		{			
			if (ar.BeginCollection(name, size))
			{	// Use the archive's internal index if possible, 
				// or a local counter that stays in sync.
				vec.resize(size);
				int i = 0;
				while(ar.HasNext())
				{
					VisitProperty(itemName, vec[i], ar);
					ar.NextItem();
					i++;
				}
				ar.EndCollection();
			}
		}
		else
		{
			if (ar.BeginCollection(name, size))
			{	// Use the archive's internal index if possible, 
				// or a local counter that stays in sync.
				for (uint32_t i = 0; i < size; ++i)
				{					
					VisitProperty(itemName, vec[i], ar);
				}
				ar.EndCollection();
			}
		}
		
	}


	template<typename ArchiveType>
	inline void Reflect(Vec2& v, ArchiveType& ar)
	{
		ar.Property("x", v.x);
		ar.Property("y", v.y);
	}

	template<typename ArchiveType>
	inline void Reflect(Vec3& v, ArchiveType& ar)
	{
		ar.Property("x", v.x);
		ar.Property("y", v.y);
		ar.Property("z", v.z);
	}

	template<typename ArchiveType>
	inline void Reflect(Vec4& v, ArchiveType& ar)
	{
		ar.Property("x", v.x);
		ar.Property("y", v.y);
		ar.Property("z", v.z);
		ar.Property("w", v.w);
	}	

	template<typename ArchiveType>
	inline void Reflect(TagComponent& comp, ArchiveType& archive) {
		VisitProperty("Name", comp.Name,archive);
		VisitProperty("IsActive", comp.IsActive,archive);
	}	

	template<typename ArchiveType>
	inline void Reflect(TransformComponent& comp, ArchiveType& archive) {
		VisitProperty("Position", comp.Position, archive);
		VisitProperty("Rotation", comp.Rotation, archive);
		VisitProperty("Scale", comp.Scale, archive);
	}

	template<typename ArchiveType>
	inline void Reflect(IDComponent& comp, ArchiveType& archive)
	{
		VisitProperty("ID", comp.ID, archive);
	}
	

	template<typename ArchiveType>
	inline void Reflect(CameraComponent& comp, ArchiveType& archive)
	{
		VisitProperty("AspectRatio", comp.AspectRatio, archive);
		VisitProperty("OrthoFar", comp.OrthoFar, archive);
		VisitProperty("OrthoNear", comp.OrthoNear, archive);
		VisitProperty("OrthoSize", comp.OrthoSize, archive);
		VisitProperty("PerspectiveFar", comp.PerspectiveFar, archive);
		VisitProperty("PerspectiveNear", comp.PerspectiveNear, archive);
		VisitProperty("PerspectiveFOV", comp.PerspectiveFOV, archive);
		VisitProperty("Primary", comp.Primary, archive);
		auto proj_type = (uint32_t)comp.ProjectionType;
		VisitProperty("ProjectionType", proj_type, archive);
		if (archive.GetMode() == ArchiveModeOptions::Load)
		{
			comp.ProjectionType = (CameraComponent::ProjectionTypeOptions)proj_type;
		}

	}

	template<typename ArchiveType>
	inline void Reflect(HierachyComponent& comp, ArchiveType& archive)
	{
		VisitProperty("Parent", comp.Parent, archive);
		//VisitProperty("Children", comp.Children, archive);
		VisitProperty("Children", "Child", comp.Children, archive);
	}

	template<typename ArchiveType>
	inline void Reflect(MeshComponent& comp, ArchiveType& archive)
	{
		VisitProperty("CastShadows", comp.bCastShadows, archive);
		VisitProperty("IsVisible", comp.bIsVisible, archive);
		VisitProperty("Materials", comp.MaterialSlots, archive);
		VisitProperty("Mesh", comp.MeshHandle, archive);
	}


	template<typename ArchiveType>
	inline void Reflect(AssetMetadata& meta, ArchiveType& archive)
	{
		if (archive.BeginStruct("AssetMetaData"))
		{
			archive.Property("AssetID", meta.AssetID);

			auto asset_type = (uint32_t)meta.AssetType;
			archive.Property("AssetType", asset_type);
			if (archive.GetMode() == ArchiveModeOptions::Load)
			{
				meta.AssetType = (AssetTypeOptions)asset_type;
			}
			archive.Property("Name", meta.Name);
			archive.Property("Path", meta.Path);
			archive.EndStruct();
		}
	};


	template<typename T, typename ArchiveType>
	void Reflect(std::vector<T>& vec, ArchiveType& ar)
	{
		uint32_t size = (uint32_t)vec.size();

		if (ar.BeginCollection("Collection", size))
		{
			if (ar.GetMode() == ArchiveModeOptions::Load)
				vec.resize(size);

			for (uint32_t i = 0; i < size; i++)
			{
				ar.NextItem();
				VisitProperty(nullptr, vec[i], ar);
			}

			ar.EndCollection();
		}
	}
	

	template<typename T, typename ArchiveType>
	void ReflectComponent(T& comp, ArchiveType& ar)
	{
		Reflect(comp, ar);
	}

	
}


#endif 