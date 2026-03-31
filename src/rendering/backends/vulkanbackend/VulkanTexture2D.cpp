#include "axelpch.h"
#include "VulkanTexture2D.h"
#include <rendering/Renderer.h>
#include <rendering/RenderAPI.h>
#include <rendering/backends/vulkanbackend/VulkanRenderAPI.h>
#include <rendering/backends/GraphicsContext.h>
#include <rendering/backends/vulkanbackend/VulkanContext.h>
#include <rendering/backends/vulkanbackend/VulkanDevice.h>
#include <core/Logger.h>

Axel::VulkanTexture2D::VulkanTexture2D(uint32_t width, uint32_t height, const unsigned char* data)
	: m_Width(width), m_Height(height)
{
    // 1. Cache the CPU data so the Device can find it during Submit
        uint32_t size = width * height * 4;
        m_LocalData = malloc(size);
        if (data)
            memcpy(m_LocalData, data, size);

		Invalidate(m_LocalData);
}

Axel::VulkanTexture2D::~VulkanTexture2D()
{
}

void Axel::VulkanTexture2D::SetData(const void* data, uint32_t size)
{
	SetLocalData((void*)data);
}

void Axel::VulkanTexture2D::Invalidate(void* data)
{
    
    VulkanContext* ctxt = dynamic_cast<VulkanContext*>(Renderer::GetGraphicsContext());
    VulkanDevice* device = dynamic_cast<VulkanDevice*>(ctxt->GetDevice().get());
    VkDevice vkdevice = device->GetLogicalDevice();

    // 2. Create VkImage
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = m_Width;
    imageInfo.extent.height = m_Height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateImage(vkdevice, &imageInfo, nullptr, &m_Image);

    // 3. Allocate Memory (This is the bit you were missing!)
    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(vkdevice, m_Image, &memReqs);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = device->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(vkdevice, &allocInfo, nullptr, &m_DeviceMemory);
    vkBindImageMemory(vkdevice, m_Image, m_DeviceMemory, 0);

    m_ImageView = device->CreateImageView(m_Image, imageInfo.format);
 
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    //samplerInfo.maxAnisotropy = device->Properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    if (vkCreateSampler(vkdevice, &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }

}