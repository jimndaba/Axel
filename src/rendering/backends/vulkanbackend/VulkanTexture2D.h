#pragma once
#ifndef VULKANTEXTURE2D_H
#define VULKANTEXTURE2D_H

#include <core/Core.h>
#include <rendering/Texture.h>
#include <vulkan/vulkan.h>

namespace Axel
{
	class GraphicsContext;
	class VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(uint32_t width, uint32_t height, const unsigned char* data);
		virtual ~VulkanTexture2D() override;
		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetRendererID() const override { return m_RendererID; }
		void SetData(const void* data, uint32_t size) override;
		void Invalidate(void* data);
		AssetTypeOptions GetType() const { return AssetTypeOptions::Texture2D; }

		

		VkImage GetImage() const { return m_Image; }
		VkImageView GetImageView() const { return m_ImageView; }
		VkSampler GetSampler() const { return m_Sampler; }
		VkDeviceMemory GetDeviceMemory() const { return m_DeviceMemory; }

		void* GetData() const override { return m_LocalData; }
		void SetLocalData(void* data) override { m_LocalData = data; }

	private:
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
		uint32_t m_RendererID = 0; // This would be the Vulkan image handle or similar

		VkImage m_Image;
		VkFormat m_Format;
		VkImageView m_ImageView;
		VkSampler m_Sampler;
		VkDeviceMemory m_DeviceMemory; // The actual GPU memory allocation

		void* m_LocalData = nullptr; // Temporary CPU-side storage for the texture data
	};
}


#endif // !VULKANTEXTURE2D_H
