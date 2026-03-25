#pragma once
#ifndef VULKANDESCRIPTORSET_H
#define VULKANDESCRIPTORSET_H

#include <vulkan/vulkan.h>
#include <rendering/DescriptorSet.h>
#include "VulkanContext.h"


namespace Axel
{
	class VulkanPipeline;
	class VulkanDevice;

	class VulkanDescriptorSet : public DescriptorSet
	{
	public:
		VulkanDescriptorSet(VulkanContext* ctxt, const Ref<VulkanPipeline>& pipeline, uint32_t setIndex);
		~VulkanDescriptorSet() = default;

		virtual void Write(const std::string& name, const Ref<UniformBuffer>& buffer) override;
		virtual void Write(const std::string& name, const Ref<Texture>& texture) override;
		virtual void Update() override;
		virtual void Destroy() override;	

		VkDescriptorSet GetHandle() const { return m_DescriptorSet; }
	private:
		VkDescriptorSet m_DescriptorSet;
		const Ref<VulkanPipeline> m_Pipeline;
		uint32_t m_SetIndex;
		
		VulkanDevice& device; // Reference to the VulkanDevice for updates
		VulkanContext* m_Context; // Store context for updates if needed
		// Internal storage to track what needs to be updated
		std::vector<VkWriteDescriptorSet> m_Writes;
		std::vector<VkDescriptorBufferInfo> m_BufferInfos; // Keep these alive until Update()
	};
}

#endif