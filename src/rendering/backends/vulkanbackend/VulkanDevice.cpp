#include "axelpch.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include <map>
#include <stdexcept>
#include <set>
#include "../../../core/Logger.h"


const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};


const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

Axel::VulkanDevice::VulkanDevice(VulkanContext* context)
{
    m_Context = context;
    s_Instance = this;
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
    // 1. Pick Physical Device (GPU)
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Context->GetInstance(), &deviceCount, nullptr);
    m_Devices.resize(deviceCount);
    vkEnumeratePhysicalDevices(m_Context->GetInstance(), &deviceCount, m_Devices.data());

    // Logic to pick a discrete GPU with Graphics Queues
    pickPhysicalDevice(m_Context->GetInstance(), m_Context->GetSurface());

    CreateLogicalDevice(m_Context->GetSurface());
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

Axel::QueueFamilyIndices Axel::VulkanDevice::FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
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

void Axel::VulkanDevice::CopyBuffer(const VulkanContext& context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    // 1. Open a temporary "One-Time" command buffer
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands(context);

    // 2. Define the copy region (from offset 0 to 'size')
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0; // Optional: start at beginning of source
    copyRegion.dstOffset = 0; // Optional: start at beginning of destination
    copyRegion.size = size;

    // 3. Record the transfer command
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    // 4. Close, Submit to Graphics Queue, and Wait for the GPU to finish
    EndSingleTimeCommands(context,commandBuffer);
}

VkCommandBuffer Axel::VulkanDevice::BeginSingleTimeCommands(const VulkanContext& context) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = context.GetCommandPool()->GetHandle(); // Use a transient pool if possible
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_LogicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void Axel::VulkanDevice::EndSingleTimeCommands(const VulkanContext& context,VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_GraphicsQueue); // Wait for the copy to finish
    vkFreeCommandBuffers(m_LogicalDevice, context.GetCommandPool()->GetHandle(), 1, &commandBuffer);
}

void Axel::VulkanDevice::WaitIdle()
{
	vkDeviceWaitIdle(m_LogicalDevice);
}

const Axel::DeviceCapabilities& Axel::VulkanDevice::GetCaps() const
{
    return DeviceCapabilities();
}

std::shared_ptr<Axel::Buffer> Axel::VulkanDevice::CreateVertexBuffer(uint32_t size)
{
    return Ref<Buffer>();
}

std::shared_ptr<Axel::Buffer> Axel::VulkanDevice::CreateIndexBuffer(uint32_t size)
{
    return Ref<Buffer>();
}

std::shared_ptr<Axel::Texture> Axel::VulkanDevice::CreateTexture(const TextureSpecification& spec)
{
    return Ref<Texture>();
}
