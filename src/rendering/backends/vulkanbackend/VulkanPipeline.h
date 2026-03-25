#pragma once
#ifndef VULKANPIPELINE_H
#define VULKANPIPELINE_H

#include <vulkan/vulkan.h>
#include <rendering/Pipeline.h>

namespace Axel
{
	class VulkanDevice;
	class VulkanPipeline : public Pipeline
	{
	public:
		VulkanPipeline(const PipelineSpecification& spec, VulkanDevice& device);
		~VulkanPipeline();
		void Bind(const GraphicsContext& context) const;
		void Unbind() const;
		void Destroy();

		VkPipeline GetHandle() const { return m_Pipeline; }
		VkPipelineLayout GetLayout() const { return m_Layout; }
		PipelineSpecification GetSpecification() const { return m_Specification; }
		VkDescriptorSetLayout GetDescriptorSetLayout(uint32_t setIndex) const { return m_DescriptorSetLayouts.at(setIndex); }
		std::map<uint32_t, VkDescriptorSetLayout> GetDescriptorSetLayouts() const { return m_DescriptorSetLayouts; }
	private:
		VkPipeline m_Pipeline;
		VkPipelineLayout m_Layout;
		PipelineSpecification m_Specification;
		VulkanDevice& m_Device;
		std::map<uint32_t, VkDescriptorSetLayout> m_DescriptorSetLayouts;
	};
}


#endif // !
