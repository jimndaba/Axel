#pragma once
#ifndef VULKANCONTEXT_H
#define VULKANCONTEXT_H

#include "core/Core.h"
#include "rendering/backends/GraphicsContext.h"
#include "vulkan/vulkan.h"
#include "VulkanSwapchain.h"
#include "VulkanCommanPool.h"

namespace Axel
{
	class Framebuffer;
	class VulkanCommandBuffer;
	struct WindowResizeEvent;

	class VulkanContext : public GraphicsContext
	{
	public:
		VulkanContext(void* windowHandle);
		virtual ~VulkanContext() override;

		void Init() override;
		void SwapBuffers() override;

		// Getters for the backend
		VkInstance GetInstance() const { return m_Instance; }
		VkSurfaceKHR GetSurface() const { return m_Surface; }
		VulkanSwapchain* GetSwapchain() const { return m_Swapchain.get(); }
		GraphicsDevice* GetDevice() { return m_Device.get(); }

		void OnWindowResize(const std::shared_ptr<WindowResizeEvent> &event);

		VkSemaphore GetImageAvailableSemaphore() { return imageAvailableSemaphores[currentFrame]; }
		VkSemaphore GetRenderFinishedSemaphore() { return renderFinishedSemaphores[currentFrame]; }
		VkFence GetInFlightFence() { return inFlightFences[currentFrame]; }
		VkCommandBuffer GetActiveCommandBuffer() const { return m_CommandBuffers[currentFrame]; }
		VkDescriptorPool GetDescriptorPool() const { return m_DescriptorPool; }
		// Add this getter
		Ref<VulkanCommandPool> GetCommandPool() const { return m_CommandPool; }

		// Helper for the factory to get the current frame index (0 or 1)
		uint32_t GetCurrentFrameIndex() const { return currentFrame; }
		virtual Ref<Framebuffer> GetCurrentFramebuffer() override;
		virtual Ref<RenderPass> GetMainRenderPass() override;
		virtual Ref<RenderCommandBuffer> GetCurrentCommandBuffer() override;

	public:
		virtual void BeginFrame() override;
		virtual void EndFrame() override;
		virtual void Shutdown() override;

	private:
		void CreateInstance();
		void SetupDebugMessenger(); // Optional: For validation layers
		void CreateSurface();
		void CreateSyncObjects();
		void CreateFramebuffers();
		void CreateDescriptorPool();
		void CreateRenderPass();
		void DestroyFramebuffers();
	private:
		const int MAX_FRAMES_IN_FLIGHT = 2;
		uint32_t currentFrame = 0;
		uint32_t m_CurrentImageIndex = 0;

		void* m_WindowHandle;
		VkInstance m_Instance;
		VkSurfaceKHR m_Surface;
		VkDescriptorPool m_DescriptorPool;
		Ref<RenderPass> m_MainRenderPass;
		uint32_t m_WindowWidth;
		uint32_t m_WindowHeight;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<Ref<Framebuffer>> m_SwapChainFramebuffers;

		Scope<VulkanDevice> m_Device;       // The GPU
		Scope<VulkanSwapchain> m_Swapchain; // The Window's Image Buffers
		Ref<VulkanCommandPool> m_CommandPool;
		std::vector<VkCommandBuffer> m_CommandBuffers;
		std::vector<std::shared_ptr<VulkanCommandBuffer>> m_CommandBufferObjects;
		// For Validation Layers
		const std::vector<const char*> m_ValidationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};


	};
}



#endif