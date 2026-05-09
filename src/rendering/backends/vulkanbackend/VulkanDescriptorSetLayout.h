#pragma once
#ifndef VULKANDESCRIPTORSETLAYOUT_H
#define VULKANDESCRIPTORSETLAYOUT_H

#include <rendering/DescriptorSetLayout.h>
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <string>

namespace Axel
{	
	class VulkanContext;

	class VulkanDescriptorSetLayout : public DescriptorSetLayout
	{
	public:
		VulkanDescriptorSetLayout(
			VulkanContext* context,		
			const std::vector<DescriptorBinding>& bindings);

		~VulkanDescriptorSetLayout() override;

		const DescriptorBinding* GetBinding(const std::string& name) const;
		uint32_t GetSetIndex() const { return m_SetIndex; }
		virtual void* GetNativeLayout() const override { return m_Layout; };

	private:
		void CreateLayout();

	private:
		VulkanContext* m_Context = nullptr;

		uint32_t m_SetIndex = 0;

		VkDescriptorSetLayout m_Layout = VK_NULL_HANDLE;

		std::vector<DescriptorBinding> m_Bindings;
	};


}

#endif