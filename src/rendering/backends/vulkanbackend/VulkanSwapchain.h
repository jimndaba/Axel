#pragma once
#ifndef VULKANSWAPCHAIN_H
#define VULKANSWAPCHAIN_H

#include "vulkan/vulkan.h"
#include "core/Core.h"
#include <vector>
#include <memory>
#include "VulkanDevice.h"


namespace Axel
{

    struct SwapchainSupportDetails {
        VkSurfaceCapabilitiesKHR Capabilities;
        std::vector<VkSurfaceFormatKHR> Formats;
        std::vector<VkPresentModeKHR> PresentModes;
    };

    class VulkanSwapchain
    {
    public:
        VulkanSwapchain(VulkanDevice& device, VkSurfaceKHR surface, VkExtent2D windowExtent);
        ~VulkanSwapchain();

        // Getters for the Renderer
        VkSwapchainKHR GetHandle() const { return m_Swapchain; }
        VkFormat GetFormat() const { return m_Format; }
        VkExtent2D GetExtent() const { return m_Extent; }
        uint32_t GetImageCount() const { return static_cast<uint32_t>(m_Images.size()); }
        VkImageView GetImageView(uint32_t index) const { return m_ImageViews[index]; }
        const std::vector<VkImageView>& GetImageViews() const { return m_ImageViews; }
        uint32_t GetMinImageCount() const { return m_MinImageCount; }

        // Returns the index of the image we should render to
        uint32_t AcquireNextImage(VkSemaphore signalSemaphore);
        uint32_t GetCurrentImageIndex() const { return m_CurrentImageIndex; }
        uint32_t GetCurrentIndex() const { return m_CurrentBufferIndex; }
     
        void AcquireNextImage();
        void RecreateSwapchain();
        void cleanupSwapChain();
    private:
        void Create();
        void CreateImageViews();

        // Helper Selection Logic
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device);

    private:
        VulkanDevice& m_Device;
        VkSurfaceKHR m_Surface;
        VkExtent2D m_WindowExtent;
        uint32_t m_CurrentImageIndex = 0;
        uint32_t m_CurrentBufferIndex = 0;
        VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
        VkFormat m_Format;
        VkExtent2D m_Extent;
        uint32_t m_MinImageCount;
        std::vector<VkImage> m_Images;
        std::vector<VkImageView> m_ImageViews;
    };




}


#endif