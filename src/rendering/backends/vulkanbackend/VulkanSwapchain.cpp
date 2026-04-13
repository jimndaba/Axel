#include "axelpch.h"
#include "core/Logger.h"
#include "VulkanSwapchain.h"
#include <algorithm>

Axel::VulkanSwapchain::VulkanSwapchain(VulkanDevice& device, VkSurfaceKHR surface, VkExtent2D windowExtent) : m_Device(device), m_Surface(surface), m_WindowExtent(windowExtent) {
	Create();
	CreateImageViews();
}

Axel::VulkanSwapchain::~VulkanSwapchain()
{
    //for (auto imageView : m_ImageViews) vkDestroyImageView(m_Device.GetLogicalDevice(), imageView, nullptr);
    //vkDestroySwapchainKHR(m_Device.GetLogicalDevice(), m_Swapchain, nullptr);
}

uint32_t Axel::VulkanSwapchain::AcquireNextImage(VkSemaphore signalSemaphore)
{
    // 1. Ask Vulkan for the next available image index.
    // It will signal 'signalSemaphore' when the GPU is actually ready to use it.
    VkResult result = vkAcquireNextImageKHR(
        m_Device.GetLogicalDevice(),
        m_Swapchain,
        UINT64_MAX,      // Timeout (wait forever)
        signalSemaphore, // Signal this when ready
        VK_NULL_HANDLE,  // No fence needed here if using semaphores
        &m_CurrentImageIndex
    );

    // 2. Handle Window Resizing (Crucial for Axel!)
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // Here you would normally trigger a swapchain recreation
        AXLOG_ERROR("Swapchain out of date! Needs recreation.");
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        AXEL_CORE_ASSERT(false, "Failed to acquire swapchain image!");
    }

    return m_CurrentImageIndex;
}

void Axel::VulkanSwapchain::AcquireNextImage()
{
}

void Axel::VulkanSwapchain::RecreateSwapchain()
{
    cleanupSwapChain();
    Create();
    CreateImageViews();
}

void Axel::VulkanSwapchain::cleanupSwapChain()
{
    for (auto imageView : m_ImageViews) {
        vkDestroyImageView(m_Device.GetLogicalDevice(), imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_Device.GetLogicalDevice(), m_Swapchain, nullptr);
}

void Axel::VulkanSwapchain::Create()
{
    SwapchainSupportDetails support = QuerySwapchainSupport(m_Device.GetPhysicalDevice());

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(support.Formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(support.PresentModes);
    VkExtent2D extent = ChooseSwapExtent(support.Capabilities);

    uint32_t imageCount = support.Capabilities.minImageCount + 1;
    if (support.Capabilities.maxImageCount > 0 && imageCount > support.Capabilities.maxImageCount) {
        imageCount = support.Capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_Surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    auto indices = m_Device.GetQueueFamilyIndices();
    uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(), indices.PresentFamily.value() };

    if (indices.GraphicsFamily != indices.PresentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = support.Capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(m_Device.GetLogicalDevice(), &createInfo, nullptr, &m_Swapchain);
	if (result != VK_SUCCESS)
    {
        AXLOG_ERROR("Failed to create Swapchain!");
    }
 
    vkGetSwapchainImagesKHR(m_Device.GetLogicalDevice(), m_Swapchain, &imageCount, nullptr);
    m_Images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_Device.GetLogicalDevice(), m_Swapchain, &imageCount, m_Images.data());

    m_Format = surfaceFormat.format;
    m_Extent = extent;
}

void Axel::VulkanSwapchain::CreateImageViews()
{
    m_ImageViews.resize(m_Images.size());

    for (size_t i = 0; i < m_Images.size(); i++) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_Images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = m_Format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkResult result = vkCreateImageView(m_Device.GetLogicalDevice(), &viewInfo, nullptr, &m_ImageViews[i]);
        
        if (result != VK_SUCCESS)
        {
         AXLOG_ERROR("Failed to create image views!");
        }
    }
}

VkSurfaceFormatKHR Axel::VulkanSwapchain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& format : availableFormats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return format;
    }
    return availableFormats[0];
}

VkPresentModeKHR Axel::VulkanSwapchain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& mode : availablePresentModes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) return mode;
    }

    // 2. IMMEDIATE is the rawest "Off" switch
    for (const auto& mode : availablePresentModes) {
        if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            return mode;
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Axel::VulkanSwapchain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& caps)
{
    if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max()) return caps.currentExtent;
    VkExtent2D actualExtent = m_WindowExtent;
    actualExtent.width = std::clamp(actualExtent.width, caps.minImageExtent.width, caps.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, caps.minImageExtent.height, caps.maxImageExtent.height);
    return actualExtent;
}

Axel::SwapchainSupportDetails Axel::VulkanSwapchain::QuerySwapchainSupport(VkPhysicalDevice device)
{
    SwapchainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.Capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.Formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.Formats.data());
    }
    uint32_t modeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &modeCount, nullptr);
    if (modeCount != 0) {
        details.PresentModes.resize(modeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &modeCount, details.PresentModes.data());
    }

    // Inside VulkanSwapChain or where you query 
    uint32_t minImageCount = details.Capabilities.minImageCount + 1;

    // Ensure we don't exceed the hardware's maximum
    if (details.Capabilities.maxImageCount > 0 && minImageCount > details.Capabilities.maxImageCount) {
        minImageCount = details.Capabilities.maxImageCount;
    }

    return details;
}
