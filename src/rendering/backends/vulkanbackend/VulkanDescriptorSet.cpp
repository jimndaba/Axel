#include "axelpch.h"
#include "VulkanDescriptorSet.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "VulkanPipeline.h"
#include "VulkanShader.h"
#include "VulkanUniformBuffer.h"
#include "VulkanShaderStorageBuffer.h"
#include <core/Logger.h>
#include "VulkanTexture2D.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanUtils.h"

Axel::VulkanDescriptorSet::VulkanDescriptorSet(
    VulkanContext* ctxt,
    const Ref<DescriptorSetLayout>& layout)
    : m_Context(ctxt), 
    m_Layout(std::static_pointer_cast<VulkanDescriptorSetLayout>(layout))
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_Context->GetDescriptorPool();
    allocInfo.descriptorSetCount = 1;

    VkDescriptorSetLayout vkLayout = (VkDescriptorSetLayout)m_Layout->GetNativeLayout();
    allocInfo.pSetLayouts = &vkLayout;

    auto device = std::static_pointer_cast<VulkanDevice>(m_Context->GetDevice());
    VkResult result = vkAllocateDescriptorSets(device->GetLogicalDevice(), &allocInfo, &m_DescriptorSet);

    if (result != VK_SUCCESS) {
        AXLOG_ERROR(" Failed to create DescriptorSet : {} ", (int)result);
    }
}

void Axel::VulkanDescriptorSet::Write(const std::string& name, const Ref<UniformBuffer>& buffer)
{
    auto binding = m_Layout->GetBinding(name);
    if (!binding) return;

    auto vkBuffer = std::static_pointer_cast<VulkanUniformBuffer>(buffer);

    m_BufferInfos.emplace_back();
    auto& bufferInfo = m_BufferInfos.back();
    bufferInfo.buffer = vkBuffer->GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = vkBuffer->GetSize();

    m_Writes.emplace_back();
    auto& write = m_Writes.back();
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_DescriptorSet;
    write.dstBinding = binding->Binding;
    write.descriptorType = AxelDescriptorTypeToVulkan(binding->Type);
    write.descriptorCount = 1;
}

void Axel::VulkanDescriptorSet::Write(const std::string& name, const Ref<Texture2D>& texture)
{
    auto binding = m_Layout->GetBinding(name);
    if (!binding) return;

    auto vkTex = std::static_pointer_cast<VulkanTexture2D>(texture);

    m_ImageInfos.emplace_back();
    auto& imageInfo = m_ImageInfos.back();
    imageInfo.imageView = vkTex->GetImageView();
    imageInfo.sampler = vkTex->GetSampler();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    m_Writes.emplace_back();
    auto& write = m_Writes.back();
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_DescriptorSet;
    write.dstBinding = binding->Binding;
    write.descriptorType = AxelDescriptorTypeToVulkan(binding->Type);
    write.descriptorCount = 1;
}

void Axel::VulkanDescriptorSet::Write(const std::string& name, const Ref<ShaderStorageBuffer>& buffer)
{
    auto binding = m_Layout->GetBinding(name);
    if (!binding) return;

    auto vkBuffer = std::static_pointer_cast<VulkanShaderStorageBuffer>(buffer);

    m_BufferInfos.emplace_back();
    auto& bufferInfo = m_BufferInfos.back();
    bufferInfo.buffer = vkBuffer->GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = buffer->GetSize();

    m_Writes.emplace_back();
    auto& write = m_Writes.back();
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_DescriptorSet;
    write.dstBinding = binding->Binding;
    write.descriptorType = AxelDescriptorTypeToVulkan(binding->Type);
    write.descriptorCount = 1;
}

void Axel::VulkanDescriptorSet::Update()
{
    uint32_t bufferIndex = 0;
    uint32_t imageIndex = 0;

    for (auto& write : m_Writes)
    {
        if (write.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
            write.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
        {
            write.pBufferInfo = &m_BufferInfos[bufferIndex++];
        }
        else if (write.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
        {
            write.pImageInfo = &m_ImageInfos[imageIndex++];
        }
    }

    auto device = std::static_pointer_cast<VulkanDevice>(m_Context->GetDevice());
    vkUpdateDescriptorSets(
        device->GetLogicalDevice(),
        static_cast<uint32_t>(m_Writes.size()),
        m_Writes.data(),
        0,
        nullptr
    );

    m_BufferInfos.clear();
    m_ImageInfos.clear();
    m_Writes.clear();
}

void Axel::VulkanDescriptorSet::SetData(const void* data, uint32_t size)
{
    // 1. Get the logical device from your Manager/Context
    auto device = std::static_pointer_cast<VulkanDevice>(m_Context->GetDevice());

    // 2. Map the 'data' to the underlying VkDescriptorBufferInfo 
    // or update the descriptor set directly using vkUpdateDescriptorSets.

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_DescriptorSet; // The VkDescriptorSet handle
    descriptorWrite.dstBinding = 0;           // Standardized binding for this set
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;

    // We assume 'data' points to a VkDescriptorBufferInfo for now 
    // to keep it API-agnostic at the caller level.
    descriptorWrite.pBufferInfo = static_cast<const VkDescriptorBufferInfo*>(data);

    vkUpdateDescriptorSets(device->GetLogicalDevice(), 1, &descriptorWrite, 0, nullptr);

}

void Axel::VulkanDescriptorSet::Destroy()
{
    if (m_DescriptorSet != VK_NULL_HANDLE)
    {
        auto device = std::static_pointer_cast<VulkanDevice>(m_Context->GetDevice());
        vkFreeDescriptorSets(
            device->GetLogicalDevice(),
            m_Context->GetDescriptorPool(),
            1,
            &m_DescriptorSet
        );

        m_DescriptorSet = VK_NULL_HANDLE;
    }
}

uint32_t Axel::VulkanDescriptorSet::GetBindingFromName(const std::string& name) const
{
    return 0;
}
