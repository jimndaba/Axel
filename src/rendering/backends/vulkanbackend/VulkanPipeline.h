#pragma once
#ifndef VULKANPIPELINE_H
#define VULKANPIPELINE_H

#include <vulkan/vulkan.h>
#include <rendering/Pipeline.h>

namespace Axel
{
	class VulkanDevice;
	class GraphicsContext;
	class DescriptorSetLayout;

	class VulkanPipeline : public Pipeline
	{
	public:
		VulkanPipeline(const PipelineSpecification& spec, GraphicsContext& cntxt);
		~VulkanPipeline();
		VkDescriptorSetLayout CreateEmptyDescriptorSetLayout(VkDevice device);
		virtual void Bind(const GraphicsContext& context) const override;
		void Unbind() const;
		void Destroy();

		VkPipeline GetHandle() const { return m_Pipeline; }
		VkPipelineLayout GetLayout() const { return m_Layout; }
		PipelineSpecification GetSpecification() const { return m_Specification; }
		Ref<DescriptorSetLayout> GetDescriptorSetLayout(uint32_t setIndex) const override { return m_DescriptorSetLayouts.at(setIndex); }

		std::map<uint32_t, Ref<DescriptorSetLayout>> GetDescriptorSetLayouts() const { return m_DescriptorSetLayouts; }
	private:
		VkPipeline m_Pipeline;
		VkPipelineLayout m_Layout;
		PipelineSpecification m_Specification;
		VulkanDevice& m_Device;
		std::map<uint32_t, Ref<DescriptorSetLayout>> m_DescriptorSetLayouts;
	};
}


#endif // !
