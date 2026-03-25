#pragma once
#ifndef VULKANCOMMANDBUFFER_H
#define VULKANCOMMANDBUFFER_H


#include <vulkan/vulkan.h>
#include <vector>
#include "VulkanDevice.h"
#include "../../RenderCommandBuffer.h"

namespace Axel
{
    class Material;
    class GraphicsContext;

    class VulkanCommandBuffer :public RenderCommandBuffer{
    public:
        VulkanCommandBuffer(VulkanDevice& device, VkCommandPool pool);

        // New "Handle" constructor for wrapping existing buffers
        VulkanCommandBuffer(VulkanDevice& device, VkCommandBuffer existingHandle)
            : m_Device(device), m_ActiveBuffer(existingHandle) {
        }

        ~VulkanCommandBuffer() = default; // Pool owns the memory, usually

        virtual void Begin() override;
        virtual void End() override;

        // The "Blue Screen" logic happens here
        virtual void BeginRenderPass(Ref<RenderPass> renderPass, Ref<Framebuffer> framebuffer) override;
        virtual void EndRenderPass() override;

        virtual void SubmitCommandBuffer(GraphicsContext* context, Ref<RenderCommandBuffer> commandBuffer);

        VkCommandBuffer GetHandle() const { return m_ActiveBuffer; }

        virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount) override;
        virtual void BindMaterial(Ref<Material> material) override;

    private:
        VulkanDevice& m_Device;
        VkCommandBuffer m_ActiveBuffer = VK_NULL_HANDLE;
    };





}



#endif
