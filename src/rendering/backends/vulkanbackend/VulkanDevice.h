#pragma once
#ifndef VULKANDEVICE_H
#define VULKANDEVICE_H

#include "rendering/backends/GraphicsDevice.h"
#include "vulkan/vulkan.h"
#include <optional>

namespace Axel
{
    class VulkanContext;

    struct QueueFamilyIndices {
        std::optional<uint32_t> GraphicsFamily;
        std::optional<uint32_t> PresentFamily;

        bool IsComplete() { return GraphicsFamily.has_value() && PresentFamily.has_value(); }
    };

    class VulkanDevice : public GraphicsDevice {
    public:
        VulkanDevice(VulkanContext* context);
        
        void Init();
        void WaitIdle();   
        void CopyBuffer(const VulkanContext& context,VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void EndSingleTimeCommands(const VulkanContext& context,VkCommandBuffer commandBuffer);
        VkCommandBuffer BeginSingleTimeCommands(const VulkanContext& context);

        

        VkDevice GetLogicalDevice() const { return m_LogicalDevice; }
        VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
        VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
        VkQueue GetPresentQueue() const { return m_PresentQueue; }
        QueueFamilyIndices GetQueueFamilyIndices() const { return m_Queues; }
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        const DeviceCapabilities& GetCaps() const;
        bool checkValidationLayerSupport();

        // Resource Creation (Abstracted)
        Ref<Buffer> CreateVertexBuffer(uint32_t size);
        Ref<Buffer> CreateIndexBuffer(uint32_t size);
        Ref<Texture> CreateTexture(const TextureSpecification& spec);

    private:
        VkPhysicalDevice m_PhysicalDevice;
        VkDevice m_LogicalDevice;
        VkQueue m_GraphicsQueue;
        VkQueue m_PresentQueue;
        QueueFamilyIndices m_Queues;

        VulkanContext* m_Context;
        std::vector<VkPhysicalDevice> m_Devices;

    private:
        bool isDeviceSuitable(VkPhysicalDevice device);
        void pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface);
       
        int rateDeviceSuitability(VkPhysicalDevice device);
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
        
        void CreateLogicalDevice(VkSurfaceKHR surface);
  

      
    };






}

#endif