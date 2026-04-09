#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H

#include <assets/ISerialisable.h>
#include <assets/AssetBase.h>


namespace Axel
{
	enum class MaterialParameterType
	{
		Float,
		Int,
		Bool,
		Vec2,
		Vec3,
		Vec4,
		Texture,
	};

	struct MaterialParameter
	{
		std::string Name;
		MaterialParameterType Type;

		uint32_t Offset;     // Offset in CPU/GPU buffer
		uint32_t Size;       // Size in bytes

		uint32_t Binding;    // Binding slot (for textures/buffers)
	};

	class MaterialParameterLayout
	{
	public:
		const MaterialParameter* Find(const std::string& name) const;

		uint32_t GetBufferSize() const { return BufferSize; }

		const std::vector<MaterialParameter>& GetParameters() const { return Parameters; }

	private:
		std::vector<MaterialParameter> Parameters;
		std::unordered_map<std::string, uint32_t> Lookup;

		uint32_t BufferSize = 0; // Total UBO size
	};

	class MaterialTemplate
	{
	public:
		MaterialParameterLayout Layout;
	};

	class Material :public ISerialisable, public IAsset
	{
	public:
		Material(MaterialTemplate* temp);

		void SetFloat(const std::string& name, float value);
		void SetVector(const std::string& name, const glm::vec4& value);
		void SetTexture(const std::string& name, TextureHandle texture);

		void Upload(); // Push to GPU


		virtual AssetTypeOptions GetType() const { return AssetTypeOptions::Material; }
		virtual void Serialize(IArchive& ar) override;
	private:
		MaterialTemplate* Template;

		std::vector<uint8_t> ParameterData; // UBO data
		bool Dirty = true;
	};





}


#endif