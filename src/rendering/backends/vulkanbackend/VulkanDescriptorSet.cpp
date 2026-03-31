#include "axelpch.h"
#include "VulkanDescriptorSet.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"
#include "VulkanUniformBuffer.h"
#include <core/Utils.h>
#include <core/Logger.h>
#include "VulkanTexture2D.h"
Axel::VulkanDescriptorSet::VulkanDescriptorSet(VulkanContext* ctxt, const Ref<VulkanPipeline>& pipeline, uint32_t setIndex):
	m_Pipeline(pipeline), m_SetIndex(setIndex), device(static_cast<VulkanDevice&>(*ctxt->GetDevice())), m_Context(ctxt)
{
    // 1. Grab the layout that was automatically created via reflection
    VkDescriptorSetLayout layout = m_Pipeline->GetDescriptorSetLayout(setIndex);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = ctxt->GetDescriptorPool(); // Usually owned by Device/Renderer
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    if (vkAllocateDescriptorSets(device.GetLogicalDevice(), &allocInfo, &m_DescriptorSet) != VK_SUCCESS) {
        AXLOG_ERROR("Failed to allocate descriptor set!");
    }
}

void Axel::VulkanDescriptorSet::Write(const std::string& name, const Ref<UniformBuffer>& buffer)
{
    // 1. Ask the Shader/Pipeline: "Where is 'u_Camera'?"
	const auto& vShader = std::static_pointer_cast<VulkanShader>(m_Pipeline->GetSpecification().Shader);
    const auto& resources = vShader->GetResources();

    // Search the reflected map for the resource name
    const ShaderResource* target = nullptr;
    for (auto& [binding, res] : resources.at(m_SetIndex)) {
        if (res.Name == name) {
            target = &res;
            break;
        }
    }

    if (!target) return; // Resource not found in this set

    // 2. Prepare the Vulkan Write
    auto vkBuffer = std::static_pointer_cast<VulkanUniformBuffer>(buffer);

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = vkBuffer->GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = target->Size;
    m_BufferInfos.push_back(bufferInfo); // Store to keep pointer valid

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_DescriptorSet;
    write.dstBinding = target->Binding;
    write.descriptorType = AxelToVulkanType(target->Type);
    write.descriptorCount = 1;
    write.pBufferInfo = &m_BufferInfos.back();

    m_Writes.push_back(write);
}

void Axel::VulkanDescriptorSet::Write(const std::string& name, const Ref<Texture2D>& texture)
{
    // 1. Ask the Shader/Pipeline: "Where is 'u_Camera'?"
    const auto& vShader = std::static_pointer_cast<VulkanShader>(m_Pipeline->GetSpecification().Shader);
    const auto& vTexture = std::static_pointer_cast<VulkanTexture2D>(texture);
    const auto& resources = vShader->GetResources();

    // Search the reflected map for the resource name
    const ShaderResource* target = nullptr;
    for (auto& [binding, res] : resources.at(m_SetIndex)) {
        if (res.Name == name) {
            target = &res;
            break;
        }
    }

    if (!target) return; // Resource not found in this set

    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = vTexture->GetSampler();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = vTexture->GetImageView();
    m_ImageInfos.push_back(imageInfo); // Store to keep pointer valid

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_DescriptorSet;
    write.dstBinding = target->Binding;
    write.descriptorType = AxelToVulkanType(target->Type);
    write.descriptorCount = 1;
    write.pImageInfo = &m_ImageInfos.back();

    m_Writes.push_back(write);
}

void Axel::VulkanDescriptorSet::Update()
{
    vkUpdateDescriptorSets(device.GetLogicalDevice(), (uint32_t)m_Writes.size(), m_Writes.data(), 0, nullptr);
    m_BufferInfos.clear();    
    m_ImageInfos.clear();
    m_Writes.clear();
}

void Axel::VulkanDescriptorSet::Destroy()
{
   if(m_DescriptorSet)
   {
       vkFreeDescriptorSets(device.GetLogicalDevice(), m_Context->GetDescriptorPool(), 1, &m_DescriptorSet);
   }    
}
