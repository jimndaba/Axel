#pragma once
#ifndef VKRENDERAPI
#define VKRENDERAPI

#include "../../RenderAPI.h"
#include <vulkan/vulkan.h>
#include "core/Core.h"
#include "math/Math.h"

namespace Axel
{
    class VulkanContext;
    class VulkanDevice;
    class VertexBuffer;

    class VulkanRenderAPI : public RenderAPI {
    public:
        explicit VulkanRenderAPI(VulkanContext* context);
        virtual void Init() override;
        virtual void Shutdown() override;


        virtual void SetClearColor(const Vec4& color) override;
        virtual void Clear() override;


        virtual const char* GetAPIName() const override { return "Vulkan API"; };
        virtual std::string GetAPIInfo() const override {
            // You can query Vulkan version and device info here using vkEnumerateInstanceVersion and vkEnumeratePhysicalDevices
            return "Vulkan Version: 1.3";
		}

        virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount) override;
        virtual void DrawQuad(const Mat4& transform, const Ref<Texture2D>& texture) override;
        virtual void SubmitCommandBuffer(Ref<RenderCommandBuffer> commandBuffer) override;
		virtual void BindDescriptorSet(uint32_t setIndex, const Ref<DescriptorSet>& set, const Ref<Pipeline>& pipeline) override;
        virtual void BindTextureDescriptorSet(uint32_t setIndex, Ref<Texture2D>& texture,Ref<Pipeline>& pipeline) override;

    protected:
		virtual GraphicsContext* GetContext() override { return (GraphicsContext*)m_Context; }

    private:
        /// Clear color state
        VkClearValue m_ClearColor{};
        VulkanContext* m_Context;

       // Helper methods
        void ValidateCommandBuffer(Ref<RenderCommandBuffer> commandBuffer);
    };




}

#endif