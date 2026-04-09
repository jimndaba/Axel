#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#include <core/Core.h>
#include <assets/AssetBase.h>

namespace Axel
{
	class GraphicsContext;
	class AX_API Texture2D : public  IAsset
	{

	public:
		virtual ~Texture2D() = default;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;
		virtual void SetData(const void* data, uint32_t size) = 0;
		virtual void* GetData() const = 0;
		virtual void SetLocalData(void* data) = 0;
		virtual AssetTypeOptions GetType() const override { return AssetTypeOptions::Texture2D ; }
		static Ref<Texture2D> Create(uint32_t width, uint32_t height, const unsigned char* data);
	
	};
}	

#endif