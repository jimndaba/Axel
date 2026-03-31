#include "axelpch.h"
#include "../../../core/Logger.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include <map>
#include <stdexcept>
#include <set>

#include <rendering/Texture.h>
#include "VulkanTexture2D.h"
#include "VulkanBuffer.h"
#include "VkVertexBuffer.h"
#include "VkIndexBuffer.h"
#include "VulkanDescriptorSet.h"


const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

Axel::VulkanDevice::VulkanDevice(VulkanContext* context)
	:m_Context(context)
{
    Init();
}

bool Axel::VulkanDevice::isDeviceSuitable(VkPhysicalDevice device) {
     bool extensionsSupported = checkDeviceExtensionSupport(device);
    return m_Queues.IsComplete() && extensionsSupported;
}

bool Axel::VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

void Axel::VulkanDevice::pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);

        auto indices = FindQueueFamilies(device, surface);

        // Strategy: Prefer Discrete GPU, must have Graphics + Present support
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && indices.IsComplete()) {
            m_PhysicalDevice = device;
            break;
        }
    }

    AXEL_CORE_ASSERT(m_PhysicalDevice, "Failed to find a suitable GPU!");
}

uint32_t Axel::VulkanDevice::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    // 1. Query the physical device for all available memory properties
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

    // 2. Loop through all available memory types (usually 10-32 types)
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {

        // Check 'typeFilter': Is this specific memory type bit set in the filter?
        // (typeFilter comes from vkGetBufferMemoryRequirements)
        bool isSuitableType = (typeFilter & (1 << i));

        // Check 'properties': Does this memory type support ALL the flags we requested?
        // (e.g., VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        bool hasRequiredProperties = (memProperties.memoryTypes[i].propertyFlags & properties) == properties;

        if (isSuitableType && hasRequiredProperties) {
            return i;
        }
    }

    AXEL_CORE_ASSERT(false, "Failed to find suitable memory type for Vulkan buffer!");
    return 0;
}

int Axel::VulkanDevice::rateDeviceSuitability(VkPhysicalDevice device) {

    int score = 0;
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    // Application can't function without geometry shaders
    if (!deviceFeatures.geometryShader) {
        return 0;
    }

    return score;
}



void Axel::VulkanDevice::Init()
{
    VkInstance instance = m_Context->GetInstance();
    VkSurfaceKHR surface = m_Context->GetSurface();

    // 1. Pick Physical Device (GPU)
    pickPhysicalDevice(instance, surface);
    CreateLogicalDevice(surface);

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    m_Devices.resize(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, m_Devices.data());

    vkGetPhysicalDeviceProperties(m_PhysicalDevice, &Properties);
}

void Axel::VulkanDevice::Shutdown()
{
    if (!m_LogicalDevice) {
        return;
    }
    WaitIdle();

    // Cleanup staging buffers
    {
        std::unique_lock lock(m_StagingBufferLock);
        for (auto& buffer : m_StagingBuffers) {
            if (buffer.Handle != VK_NULL_HANDLE) {
                vkDestroyBuffer(m_LogicalDevice, buffer.Handle, nullptr);
            }
            if (buffer.Memory != VK_NULL_HANDLE) {
                vkFreeMemory(m_LogicalDevice, buffer.Memory, nullptr);
            }
        }
        m_StagingBuffers.clear();
    }

    //Cleanup texture descriptors
    {

        for (auto& descp : m_TextureDescriptorSets)
        {
            descp.second->Destroy();
        }

    }


    // Cleanup transfer command pool
    if (m_TransferCommandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(m_LogicalDevice, m_TransferCommandPool, nullptr);
        m_TransferCommandPool = VK_NULL_HANDLE;
    }

    // Cleanup logical device
    if (m_LogicalDevice != VK_NULL_HANDLE) {
        vkDestroyDevice(m_LogicalDevice, nullptr);
        m_LogicalDevice = VK_NULL_HANDLE;
    }


    AXLOG_INFO("VulkanDevice shutdown complete");
}


void Axel::VulkanDevice::CreateLogicalDevice(VkSurfaceKHR surface)
{
    m_Queues = FindQueueFamilies(m_PhysicalDevice, surface);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { m_Queues.GraphicsFamily.value(), m_Queues.PresentFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t family : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = family;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Required for showing images to the screen
    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    VkResult result = vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_LogicalDevice);
    if (result != VK_SUCCESS)
    {
        AXLOG_ERROR("Failed to create Logical Device!");
    }

    // Grab the actual handles for the queues
    vkGetDeviceQueue(m_LogicalDevice, m_Queues.GraphicsFamily.value(), 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_LogicalDevice, m_Queues.PresentFamily.value(), 0, &m_PresentQueue);
}

Axel::VulkanDevice::QueueFamilyIndices Axel::VulkanDevice::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{    
    QueueFamilyIndices indices{};

        // 1. Get the count of queue families
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        // 2. Retrieve the properties for all families
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            // Check for Graphics Support
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.GraphicsFamily = i;
            }

            // Check for Presentation Support (Surface support)
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport) {
                indices.PresentFamily = i;
            }

            // Optimization: If we found both, we can stop looking
            if (indices.IsComplete()) {
                break;
            }

            i++;
        }

        return indices;
}

bool Axel::VulkanDevice::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

void Axel::VulkanDevice::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    // 1. Open a temporary "One-Time" command buffer
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    // 2. Define the copy region (from offset 0 to 'size')
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional: start at beginning of source
    copyRegion.dstOffset = 0; // Optional: start at beginning of destination
    copyRegion.size = size;

    // 3. Record the transfer command
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    // 4. Close, Submit to Graphics Queue, and Wait for the GPU to finish
    EndSingleTimeCommands(commandBuffer);
}

VkCommandBuffer Axel::VulkanDevice::BeginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_Context->GetCommandPool()->GetHandle(); // Use a transient pool if possible
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_LogicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void Axel::VulkanDevice::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_GraphicsQueue); // Wait for the copy to finish
    vkFreeCommandBuffers(m_LogicalDevice, m_Context->GetCommandPool()->GetHandle(), 1, &commandBuffer);
}

Axel::VulkanDevice::StagingBuffer& Axel::VulkanDevice::AcquireStagingBuffer(size_t minimumSize)
{
    std::unique_lock lock(m_StagingBufferLock);

    // ============================================
    // STEP 1: Try to reuse an existing buffer
    // ============================================

    for (auto& buffer : m_StagingBuffers) {
        // Check if buffer is:
        // 1. Not currently in use
        // 2. Large enough for our data
        if (!buffer.IsInUse && buffer.Size >= minimumSize) {
            buffer.IsInUse = true;
            AXLOG_TRACE("Reused staging buffer: {} bytes", buffer.Size);
            return buffer;
        }
    }

    // ============================================
    // STEP 2: Create new buffer if no suitable one exists
    // ============================================

    AXLOG_TRACE("Creating new staging buffer: {} bytes", minimumSize);

    StagingBuffer newBuffer;
    newBuffer.Size = minimumSize;
    newBuffer.IsInUse = true;

    // Create VkBuffer
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = minimumSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;  // For uploading TO GPU
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_LogicalDevice, &bufferInfo, nullptr, &newBuffer.Handle) != VK_SUCCESS) {
        AXLOG_ERROR("Failed to create staging buffer!");
        // Return an empty buffer (caller should check Handle)
        return m_StagingBuffers.emplace_back(newBuffer);
    }

    // Get memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_LogicalDevice, newBuffer.Handle, &memRequirements);

    // Allocate HOST-VISIBLE memory (CPU can write to it)
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(
        memRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    if (vkAllocateMemory(m_LogicalDevice, &allocInfo, nullptr, &newBuffer.Memory) != VK_SUCCESS) {
        AXLOG_ERROR("Failed to allocate staging buffer memory!");
        vkDestroyBuffer(m_LogicalDevice, newBuffer.Handle, nullptr);
        return m_StagingBuffers.emplace_back(StagingBuffer{});
    }

    // Bind buffer to memory
    vkBindBufferMemory(m_LogicalDevice, newBuffer.Handle, newBuffer.Memory, 0);

    AXLOG_TRACE("Staging buffer created: handle={}, memory={}, size={}",
        (void*)newBuffer.Handle, (void*)newBuffer.Memory, newBuffer.Size);

    // Add to pool and return reference
    m_StagingBuffers.push_back(newBuffer);
    return m_StagingBuffers.back();
}

void Axel::VulkanDevice::ReleaseStagingBuffer(StagingBuffer& buffer)
{
    std::unique_lock lock(m_StagingBufferLock);

    // ============================================
    // Mark buffer as available for reuse
    // ============================================

    // Find the buffer in our list and mark it as not in use
    for (auto& existingBuffer : m_StagingBuffers) {
        if (existingBuffer.Handle == buffer.Handle) {
            existingBuffer.IsInUse = false;
            AXLOG_TRACE("Released staging buffer: {} bytes", existingBuffer.Size);
            return;
        }
    }

    AXLOG_WARN("Attempted to release staging buffer that is not in our pool!");
}

VkImageView Axel::VulkanDevice::CreateImageView(VkImage image, VkFormat format)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(m_LogicalDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image view!");
    }

    return imageView;
}

std::shared_ptr<Axel::DescriptorSet> Axel::VulkanDevice::GetTextureDescriptor(UUID& outid, Ref<Pipeline>& pipeline, uint32_t index)
{
    auto it = m_TextureDescriptorSets.find(outid);
    if (it != m_TextureDescriptorSets.end())
    {
        return m_TextureDescriptorSets[outid];
    }
    m_TextureDescriptorSets[outid] = DescriptorSet::Create(m_Context, pipeline, index);
    return m_TextureDescriptorSets[outid];
}

void Axel::VulkanDevice::WaitIdle()
{
    if (m_LogicalDevice) {
        vkDeviceWaitIdle(m_LogicalDevice);
    }
}

const Axel::DeviceCapabilities& Axel::VulkanDevice::GetCaps() const
{
    return DeviceCapabilities();
}

std::shared_ptr<Axel::VertexBuffer> Axel::VulkanDevice::CreateVertexBuffer(float* vertices, uint32_t size)
{
	return VertexBuffer::Create(vertices, size, m_Context);
}

std::shared_ptr<Axel::IndexBuffer> Axel::VulkanDevice::CreateIndexBuffer(uint32_t* indices, uint32_t count)
{
	return IndexBuffer::Create(indices,count, m_Context);
}

std::shared_ptr<Axel::Texture2D> Axel::VulkanDevice::CreateTexture(uint32_t width, uint32_t height, const unsigned char* data)
{
    return CreateRef<VulkanTexture2D>(width, height, data);
}

bool Axel::VulkanDevice::UploadTexture(Ref<Texture2D> texture)
{
    auto vkTexture = std::dynamic_pointer_cast<VulkanTexture2D>(texture);
    if (!vkTexture) {
        AXLOG_ERROR("VulkanDevice::UploadTexture: texture is not VulkanTexture2D");
        return false;
    }

    // ✅ Get texture data
    uint32_t width = vkTexture->GetWidth();
    uint32_t height = vkTexture->GetHeight();
    const void* cpuData = vkTexture->GetData();

    if (!cpuData) {
        AXLOG_ERROR("VulkanDevice::UploadTexture: texture has no CPU data");
        return false;
    }

    uint32_t dataSize = width * height * 4; // RGBA = 4 bytes per pixel

    // ============================================
        // STEP 1: Create Staging Buffer (CPU → GPU)
        // ============================================

    auto stagingBuffer = AcquireStagingBuffer(dataSize);

    void* mappedMemory;
    vkMapMemory(m_LogicalDevice, stagingBuffer.Memory, 0, dataSize, 0, &mappedMemory);
    memcpy(mappedMemory, cpuData, dataSize);
    vkUnmapMemory(m_LogicalDevice, stagingBuffer.Memory);

    VkImage image = vkTexture->GetImage();
    if (image == VK_NULL_HANDLE) {
        AXLOG_ERROR("VulkanDevice::UploadTexture: texture has no VkImage");
        vkDestroyBuffer(m_LogicalDevice, stagingBuffer.Handle, nullptr);
        vkFreeMemory(m_LogicalDevice, stagingBuffer.Memory, nullptr);
        return false;
    }

    auto cmd = BeginSingleTimeCommands();

    TransitionImageLayout(
        cmd,
        image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
    );

    CopyBufferToImage(cmd, stagingBuffer.Handle, image, vkTexture->GetWidth(), vkTexture->GetHeight());
   
    TransitionImageLayout(
        cmd,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );

    EndSingleTimeCommands(cmd);

    ReleaseStagingBuffer(stagingBuffer);
    return true;
}

bool Axel::VulkanDevice::UploadMesh(Ref<Mesh> mesh)
{
    return false;
}

bool Axel::VulkanDevice::UploadBuffer(Ref<Buffer> buffer)
{
    return false;
}

void Axel::VulkanDevice::UnloadTexture(UUID textureID)
{
}

bool Axel::VulkanDevice::IsTextureResident(UUID textureID) const
{
    return false;
}

void Axel::VulkanDevice::DestroyTexture(Ref<Texture2D>& texture)
{
    if (!texture) return;

    auto vkTex = std::static_pointer_cast<VulkanTexture2D>(texture);
    VkDevice device = m_LogicalDevice;

    // 1. Wait for the GPU to finish current work 
    //    // (In a production engine, you'd use a Deletion Queue, but for Sandbox, this is safe)   

    // 2. Destroy the "Shader Link"
    if (vkTex->GetSampler())
        vkDestroySampler(device, vkTex->GetSampler(), nullptr);

    if (vkTex->GetImageView())
        vkDestroyImageView(device, vkTex->GetImageView(), nullptr);

    // 3. Destroy the "Physical Resource"
    if (vkTex->GetImage())
        vkDestroyImage(device, vkTex->GetImage(), nullptr);

    // 4. Free the VRAM (Assuming manual allocation, not VMA)
    if (vkTex->GetDeviceMemory())
        vkFreeMemory(device, vkTex->GetDeviceMemory(), nullptr);

    AXLOG_INFO("VulkanDevice: Texture destroyed successfully.");
}

void Axel::VulkanDevice::TransitionImageLayout(VkCommandBuffer cb, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    // Transition 1: Preparation for Copy (Undefined -> Transfer Dest)
    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // Start as soon as possible
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    // Transition 2: Preparation for Shader (Transfer Dest -> Shader Read Only)
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Wait for copy to finish before FS reads
    }
    else {
        AXLOG_ERROR("Unsupported layout transition!");
        return;
    }

    vkCmdPipelineBarrier(
        cb,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}

void Axel::VulkanDevice::CopyBufferToImage(VkCommandBuffer cb, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;   // Tightly packed
    region.bufferImageHeight = 0; // Tightly packed

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1 // Depth is 1 for 2D textures
    };

    vkCmdCopyBufferToImage(
        cb,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // Must match the previous transition!
        1,
        &region
    );
}