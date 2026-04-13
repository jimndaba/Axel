#pragma once
#ifndef MATERIALINSTANCE_H
#define MATERIALINSTANCE_H

#include <core/Core.h>
#include <core/UUID.h>
#include <assets/ISerialisable.h>
#include <assets/AssetBase.h>
#include <math/Math.h>
#include <variant>
#include <type_traits>


namespace Axel
{

	using MaterialPropertyValue = std::variant<
		bool,
		int32_t,
		uint32_t,
		float,
		Vec2,
		Vec3,
		Vec4,
		UUID
	>;

	class AX_API MaterialInstance: public ISerialisable, public IAsset
	{
	public: 
		MaterialInstance(UUID templateID);
		virtual ~MaterialInstance() = default;

		// --- Property Management ---
		template<typename T>
		void Set(const std::string& name, const T& value) {
			if (m_Properties.find(name) != m_Properties.end()) {
				m_Properties[name] = value;
				m_IsDirty = true;
			}
		}

		template<typename T>
		T Get(const std::string& name) {
			return std::get<T>(m_Properties[name]);
		}

		// --- The "Bake" for the SSBO ---
		void PackData(uint8_t* destination);

		// --- ISerialisable Implementation ---
		virtual void Serialize(IArchive& archive) override;
		//virtual void OnLoad(struct AssetContext& context) override;

		AssetTypeOptions GetType() const override { return AssetTypeOptions::Material; }
		UUID GetTemplateID() const { return m_TemplateID; }

	private:
		UUID m_TemplateID;
		// The dynamic data store
		std::map<std::string, MaterialPropertyValue> m_Properties;
		bool m_IsDirty = true;
	};







}

#endif