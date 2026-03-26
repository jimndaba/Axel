#pragma once
#ifndef VULKANDEVICE_H
#define VULKANDEVICE_H


#include "vulkan/vulkan.h"
#include <optional>
#include "rendering/backends/GraphicsDevice.h"
#include <core/UUID.h>
#include <shared_mutex>

namespace Axel
{
    class VulkanContext;

   

    class VulkanDevice : public GraphicsDevice {
    public:
        struct QueueFamilyIndices {
            std::optional<uint32_t> GraphicsFamily;
            std::optional<uint32_t> PresentFamily;

            bool IsComplete() { return GraphicsFamily.has_value() && PresentFamily.has_value(); }
        };

        /// Staging buffer pool for efficient uploads
        struct StagingBuffer {
            VkBuffer Handle = VK_NULL_HANDLE;
            VkDeviceMemory Memory = VK_NULL_HANDLE;
            size_t Size = 0;
            bool IsInUse = false;
        };


        VulkanDevice(VulkanContext* context);        
        void Init() override;
		void Shutdown() override;
        void WaitIdle() override;

        // Resource Creation (Abstracted)
        Ref<VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t size)override;    
        Ref<IndexBuffer> CreateIndexBuffer(uint32_t* indices, uint32_t count)override;
        Ref<Texture2D> CreateTexture(uint32_t width, uint32_t height, const unsigned char* data) override;

        // =========================
        // GPU Upload (GraphicsDevice interface)
        // =========================
        virtual bool UploadTexture(Ref<Texture2D> texture) override;
        virtual bool UploadMesh(Ref<Mesh> mesh) override;
        virtual bool UploadBuffer(Ref<Buffer> buffer) override;
        virtual void UnloadTexture(UUID textureID) override;
        virtual bool IsTextureResident(UUID textureID) const override;


        VkDevice GetLogicalDevice() const { return m_LogicalDevice; }
        VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
        VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
        VkQueue GetPresentQueue() const { return m_PresentQueue; }
        QueueFamilyIndices GetQueueFamilyIndices() const { return m_Queues; }
        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        const DeviceCapabilities& GetCaps() const;
        bool checkValidationLayerSupport();       

		void DestroyTexture(Ref<Texture2D>& texture);
        void SubmitTextureToGPU(Ref<Texture2D>& texture);

        void TransitionImageLayout(VkCommandBuffer cb, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
        void CopyBufferToImage(VkCommandBuffer cb, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        VkCommandBuffer BeginSingleTimeCommands();
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);


        StagingBuffer& AcquireStagingBuffer(size_t minimumSize);
        void ReleaseStagingBuffer(StagingBuffer& buffer);

    private:
        VkPhysicalDevice m_PhysicalDevice;
        VkDevice m_LogicalDevice;
        VkQueue m_GraphicsQueue;
        VkQueue m_PresentQueue;
        VkCommandPool m_TransferCommandPool = VK_NULL_HANDLE;  // For staging buffer uploads
        QueueFamilyIndices m_Queues;

        VulkanContext* m_Context = nullptr;
        std::vector<VkPhysicalDevice> m_Devices;

        std::unordered_map<UUID, VkImage> m_ResidentTextures;
        std::shared_mutex m_ResidentTextureLock;
      
        std::vector<StagingBuffer> m_StagingBuffers;
        std::shared_mutex m_StagingBufferLock;

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