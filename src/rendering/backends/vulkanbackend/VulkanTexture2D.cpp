#include "axelpch.h"
#include "VulkanTexture2D.h"
#include <rendering/backends/GraphicsContext.h>
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
    /*
    VkDevice device = VulkanContext::GetDevice()->GetLogicalDevice();

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
    imageInfo.sharingMode = VK_SH_MODE_EXCLUSIVE;

    vkCreateImage(device, &imageInfo, nullptr, &m_Image);

    // 3. Allocate Memory (This is the bit you were missing!)
    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(device, m_Image, &memReqs);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = VulkanContext::GetDevice()->FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(device, &allocInfo, nullptr, &m_DeviceMemory);
    vkBindImageMemory(device, m_Image, m_DeviceMemory, 0);
    */
}