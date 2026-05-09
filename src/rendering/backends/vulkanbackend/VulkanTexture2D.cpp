#include "axelpch.h"
#include "VulkanTexture2D.h"
#include <rendering/Renderer.h>
#include <rendering/RenderAPI.h>
#include <rendering/backends/vulkanbackend/VulkanRenderAPI.h>
#include <rendering/GraphicsContext.h>
#include <rendering/backends/vulkanbackend/VulkanContext.h>
#include <rendering/backends/vulkanbackend/VulkanDevice.h>
#include <core/Logger.h>
#include "VulkanUtils.h"


Axel::VulkanTexture2D::VulkanTexture2D(TextureCreationInfo& info)
	: m_Width(info.Width), m_Height(info.Height), m_Specification(info)
{
    // 1. Cache the CPU data so the Device can find it during Submit
        uint32_t size = info.Width * info.Height * TextureFormatComponentCount(info.TextureFormat);
        m_LocalData = malloc(size);
        if (info.Data)
            memcpy(m_LocalData, info.Data, size);

		Invalidate(m_LocalData);
}

Axel::VulkanTexture2D::~VulkanTexture2D()
{
    Destroy();
}

void Axel::VulkanTexture2D::Destroy()
{
    if (m_LocalData)
    {
        free(m_LocalData);
        m_LocalData = nullptr;
    }

    // 1. Synchronize: Ensure the GPU is done with the old image
    VulkanContext* ctxt = dynamic_cast<VulkanContext*>(Renderer::GetGraphicsContext());
    VulkanDevice* device = dynamic_cast<VulkanDevice*>(ctxt->GetDevice().get());
    

    // 2. Cleanup old resources
    // Assuming your class owns these members
    if (m_ImageView) {
        vkDestroyImageView(device->GetLogicalDevice(), m_ImageView, nullptr);
        m_ImageView = VK_NULL_HANDLE;
    }
    if (m_Image) {
        vkDestroyImage(device->GetLogicalDevice(), m_Image, nullptr);
      
    }
    if (m_DeviceMemory) {
        vkFreeMemory(device->GetLogicalDevice(), m_DeviceMemory, nullptr);
        m_DeviceMemory = VK_NULL_HANDLE;
    }
}

bool Axel::VulkanTexture2D::ResizeImage(uint32_t width, uint32_t height)
{
    if (width == 0 || height == 0) return false;
    if (m_Width == width && m_Height == height) return true;

    // 1. Synchronize: Ensure the GPU is done with the old image
    VulkanContext* ctxt = dynamic_cast<VulkanContext*>(Renderer::GetGraphicsContext());
    VulkanDevice* device = dynamic_cast<VulkanDevice*>(ctxt->GetDevice().get());
    vkDeviceWaitIdle(device->GetLogicalDevice());

    // 2. Cleanup old resources
    // Assuming your class owns these members
    Destroy();

    // 3. Update Dimensions
    m_Width = width;
    m_Height = height;

    // 4. Re-allocate and Re-create
    // You should call your existing internal creation method here
    // e.g., CreateImage(), CreateImageView(), etc.
    bool success = Invalidate(m_LocalData); // Custom internal method to re-init resources

    if (success) {
        // Set a flag or fire a callback so the ViewModel knows 
        // to call ImGui_ImplVulkan_AddTexture again
        //m_IsDirty = true;
    }

    return success;
}

void Axel::VulkanTexture2D::SetData(const void* data, uint32_t size)
{
	SetLocalData((void*)data);
}

bool Axel::VulkanTexture2D::Invalidate(void* data)
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
    if (IsDepthFormat(m_Specification.TextureFormat))
    {
        imageInfo.format = device->FindDepthFormat();
    }
    else
    {
        imageInfo.format = AxelTextureFormatToVulkan(m_Specification.TextureFormat);
    }
   
    imageInfo.tiling =VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = AxelUsageToVulkan(m_Specification.TextureFormat,m_Specification.Usage);
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
    samplerInfo.magFilter = AxelFilterToVulkan(m_Specification.MagFilter);
    samplerInfo.minFilter = AxelFilterToVulkan(m_Specification.MinFilter);
    samplerInfo.addressModeU = AxelWrapToVulkan(m_Specification.WrapS);
    samplerInfo.addressModeV = AxelWrapToVulkan(m_Specification.WrapT);
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
        return false;
    }
    return true;
}


