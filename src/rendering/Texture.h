#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#include <core/Core.h>
#include <assets/AssetBase.h>
#include <rendering/GraphicsCore.h>

namespace Axel
{
	class GraphicsContext;
	class DescriptorSet;

	struct TextureCreationInfo
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
		TextureFormatOptions TextureFormat = TextureFormatOptions::RGBA8;

		// Sampling options
		TextureFilterOptions MinFilter = TextureFilterOptions::Linear;
		TextureFilterOptions MagFilter = TextureFilterOptions::Linear;
		TextureWrapOptions WrapS = TextureWrapOptions::Repeat;
		TextureWrapOptions WrapT = TextureWrapOptions::Repeat;

		// Behavior options
		bool GenerateMips = false;
		TextureUsageOptions Usage = TextureUsageOptions::None;

		// Initial data (set to nullptr for empty attachments like Normals/Depth)
		const void* Data = nullptr;
	};

	class AX_API Texture2D : public  IAsset
	{

	public:
		virtual ~Texture2D() = default;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;	
		virtual void Destroy() = 0;	
		virtual bool ResizeImage(uint32_t width, uint32_t height) = 0;
		virtual void SetData(const void* data, uint32_t size) = 0;
		virtual void* GetData() const = 0;
		virtual void SetLocalData(void* data) = 0;
		virtual AssetTypeOptions GetType() const override { return AssetTypeOptions::Texture2D ; }
		static Ref<Texture2D> Create(TextureCreationInfo& info);
		
	
	};
}	

#endif