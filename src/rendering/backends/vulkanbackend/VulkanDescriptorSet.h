#pragma once
#ifndef VULKANDESCRIPTORSET_H
#define VULKANDESCRIPTORSET_H

#include <core/Core.h>
#include <vulkan/vulkan.h>
#include <rendering/DescriptorSet.h>
#include <rendering/DescriptorSetLayout.h>
#include "VulkanContext.h"

namespace Axel
{
	class VulkanDevice;
	class VulkanDescriptorSetLayout;

	class AX_API VulkanDescriptorSet : public DescriptorSet
	{
	public:
		VulkanDescriptorSet(VulkanContext* ctxt, const Ref<DescriptorSetLayout>& layout);
		~VulkanDescriptorSet() override = default;

		virtual void SetData(const void* data, uint32_t size) override;
		// API-agnostic interface
		virtual void Write(const std::string& name, const Ref<UniformBuffer>& buffer) override;
		virtual void Write(const std::string& name, const Ref<Texture2D>& texture) override;
		virtual void Write(const std::string& name, const Ref<ShaderStorageBuffer>& buffer) override;

		virtual void Update() override;
		virtual void Destroy() override;

		VkDescriptorSet GetHandle() const { return m_DescriptorSet; }

	private:
		uint32_t GetBindingFromName(const std::string& name) const;

	private:
		VkDescriptorSet m_DescriptorSet = VK_NULL_HANDLE;

		Ref<VulkanDescriptorSetLayout> m_Layout;		
		VulkanContext* m_Context;

		// Write tracking
		std::vector<VkWriteDescriptorSet> m_Writes;
		std::vector<VkDescriptorBufferInfo> m_BufferInfos;
		std::vector<VkDescriptorImageInfo> m_ImageInfos;
	};
}

#endif