#include "axelpch.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanContext.h"
#include "VulkanDevice.h" 
#include "VulkanUtils.h"

Axel::VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(
	VulkanContext* context,	
	const  std::vector<DescriptorBinding>& bindings)
	: m_Context(context),	
	m_Bindings(bindings)
{
	CreateLayout();
}

Axel::VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
{
	if (m_Layout != VK_NULL_HANDLE)
	{
		auto device = std::static_pointer_cast<VulkanDevice>(m_Context->GetDevice());
		vkDestroyDescriptorSetLayout(
			device->GetLogicalDevice(),
			m_Layout,
			nullptr
		);

		m_Layout = VK_NULL_HANDLE;
	}
}

const Axel::DescriptorBinding* Axel::VulkanDescriptorSetLayout::GetBinding(const std::string& name) const
{
	for (const auto& binding : m_Bindings)
	{
		if (binding.Name == name)
		{
			return &binding;
		}
	}
	return nullptr;
}

void Axel::VulkanDescriptorSetLayout::CreateLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> vkBindings;
	vkBindings.reserve(m_Bindings.size());

	for (const auto& binding : m_Bindings)
	{
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding.Binding;
		layoutBinding.descriptorType = AxelDescriptorTypeToVulkan(binding.Type);
		layoutBinding.descriptorCount = binding.Count;
		layoutBinding.stageFlags = VK_SHADER_STAGE_ALL; // simplify for now
		layoutBinding.pImmutableSamplers = nullptr;
		vkBindings.push_back(layoutBinding);
	}

	VkDescriptorSetLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	createInfo.bindingCount = static_cast<uint32_t>(vkBindings.size());
	createInfo.pBindings = vkBindings.data();

	auto device = std::static_pointer_cast<VulkanDevice>(m_Context->GetDevice());
	vkCreateDescriptorSetLayout(
		device->GetLogicalDevice(),
		&createInfo,
		nullptr,
		&m_Layout
	);
}
