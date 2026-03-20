#include "axelpch.h"
#include "VulkanContext.h"
#include "GLFW/glfw3.h"
#include <stdexcept>
#include "../../../core/Logger.h"
#include <rendering/backends/FrameBuffer.h>

Axel::VulkanContext::VulkanContext(void* windowHandle) : m_WindowHandle(windowHandle) {
    AXEL_CORE_ASSERT(m_WindowHandle, "Window handle is null!")
}

Axel::VulkanContext::~VulkanContext()
{
    

  
}

void Axel::VulkanContext::Init()
{
    CreateInstance();
    CreateSurface();
   

    // 3. Initialize the Device (The GPU hardware)
    // We pass 'this' so the device can access the Instance and Surface
    m_Device = CreateScope<VulkanDevice>(this);

    m_CommandPool = CreateRef<VulkanCommandPool>(*m_Device);
    // 4. INITIALIZE THE SWAPCHAIN
    // It needs the device to create handles, the surface to display on, 
    // and the window size to determine the resolution.
    VkExtent2D extent = { m_WindowWidth, m_WindowHeight }; // TODO: Update window widgr height
    m_Swapchain = CreateScope<VulkanSwapchain>(*m_Device, m_Surface, extent);

    CreateSyncObjects();

    CreateFramebuffers();

	CreateRenderPass();
}

void  Axel::VulkanContext::CreateSyncObjects() {

    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(m_Device->GetLogicalDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_Device->GetLogicalDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_Device->GetLogicalDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

void Axel::VulkanContext::CreateFramebuffers()
{
    auto& images = m_Swapchain->GetImageViews();
    m_SwapChainFramebuffers.resize(images.size());

    for (size_t i = 0; i < images.size(); i++) {
        FramebufferSpecification spec;

        // 1. Define the "Recipe" (What the RenderPass expects)
        spec.Attachments = { FramebufferTextureFormat::RGBA8 };

        // 2. Provide the "Physical Memory" (The Swapchain View)
        spec.ExistingImages = { images[i] };

        spec.SwapChainTarget = true;
        spec.RenderPass = m_MainRenderPass;
        spec.Width = m_Swapchain->GetExtent().width;
        spec.Height = m_Swapchain->GetExtent().height;

        // 3. This creates a VulkanFramebuffer wrapping that specific Swapchain Image
        m_SwapChainFramebuffers[i] = Framebuffer::Create(spec,m_Device.get());
    }
}

void Axel::VulkanContext::CreateRenderPass()
{
    
}

void Axel::VulkanContext::SwapBuffers()
{
    VkDevice device = m_Device->GetLogicalDevice();
    VkQueue presentQueue = m_Device->GetPresentQueue();
    VkSwapchainKHR swapchain = m_Swapchain->GetHandle();

    // 1. Acquire the next image index from the swapchain
    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, m_Swapchain->GetHandle(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    

    ///POOSSSIBLE RENDER COMMAND EXECUTION


    // 3. Present the image to the screen
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame]; // Wait for blue clear to finish

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;

    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // Handle window resize
        m_Swapchain->RecreateSwapchain();
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Axel::VulkanContext::OnWindowResize(uint32_t width, uint32_t height)
{
    // Wait for the GPU to finish its current work
    vkDeviceWaitIdle(m_Device->GetLogicalDevice());

    // Re-initialize the swapchain with the new dimensions
    VkExtent2D newExtent = { width, height };
    m_Swapchain = CreateScope<VulkanSwapchain>(*m_Device, m_Surface, newExtent);
}

std::shared_ptr<Axel::Framebuffer> Axel::VulkanContext::GetCurrentFramebuffer()
{
    // 1. Get the index from the swapchain
    uint32_t index = m_Swapchain->GetCurrentIndex();

    // 2. Safety check: Ensure we don't index out of bounds
    AXEL_CORE_ASSERT(index < m_SwapChainFramebuffers.size(), "Swapchain index out of range!");

    // 3. Return the pre-created framebuffer for this specific image
    return m_SwapChainFramebuffers[index];
}

std::shared_ptr<Axel::RenderPass > Axel::VulkanContext::GetMainRenderPass()
{
    return m_MainRenderPass;
}

void Axel::VulkanContext::BeginFrame()
{
    VkDevice device = m_Device->GetLogicalDevice();

    // 1. Wait for the GPU to finish the frame from 2 cycles ago (Frames in Flight)
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // 2. Acquire the next image from the swapchain
    // This signals imageAvailableSemaphores[m_CurrentFrame] when the image is ready
    m_Swapchain->AcquireNextImage(imageAvailableSemaphores[currentFrame]);

    // 3. Reset the fence so we can use it for the new submission
    vkResetFences(device, 1, &inFlightFences[currentFrame]);
}

void Axel::VulkanContext::EndFrame()
{
    auto device = m_Device->GetLogicalDevice();
    auto graphicsQueue = m_Device->GetGraphicsQueue();
    auto presentQueue = m_Device->GetPresentQueue();

    // 1. Submit the Command Buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Wait for the image to be available from the swapchain before writing to it
    VkSemaphore waitSemaphores[] = { GetImageAvailableSemaphore() };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    // The actual recorded commands
    auto commandBuffer = m_Swapchain->GetCurrentCommandBuffer();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // Signal this semaphore when the GPU is finished rendering
    VkSemaphore signalSemaphores[] = { GetRenderFinishedSemaphore() };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // Reset the fence so we can wait on it next frame
    VkFence fence = GetInFlightFence();
    vkResetFences(device, 1, &fence);

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS) {
        AXEL_CORE_ASSERT(false, "Failed to submit draw command buffer!");
    }

    // 2. Present the result to the screen
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores; // Wait for rendering to finish

    VkSwapchainKHR swapChains[] = { m_Swapchain->GetHandle() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    uint32_t imageIndex = m_Swapchain->GetCurrentImageIndex();
    presentInfo.pImageIndices = &imageIndex;

    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // Handle Window Resize here!
        RecreateSwapchain();
    }
    else if (result != VK_SUCCESS) {
        AXEL_CORE_ASSERT(false, "Failed to present swap chain image!");
    }
}

void Axel::VulkanContext::Shutdown()
{
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(m_Device->GetLogicalDevice(), renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_Device->GetLogicalDevice(), imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_Device->GetLogicalDevice(), inFlightFences[i], nullptr);
    }

    if (m_CommandPool) vkDestroyCommandPool(m_Device->GetLogicalDevice(), m_CommandPool->GetHandle(), nullptr);
    if (m_Surface) vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);

    m_Swapchain->cleanupSwapChain();
	
    if (m_Instance) vkDestroyInstance(m_Instance, nullptr);
}

void Axel::VulkanContext::CreateInstance()
{

    // 1. App Info
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Axel Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Axel";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3; // Using modern Vulkan 1.3

    // 2. Extensions (Required for Window Surface)
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    // 3. Validation Layers
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
        createInfo.ppEnabledLayerNames = m_ValidationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
    AXEL_CORE_ASSERT(result == VK_SUCCESS, "Failed to create Vulkan Instance!");
}

void Axel::VulkanContext::SetupDebugMessenger()
{
}

void Axel::VulkanContext::CreateSurface()
{
    // GLFW abstracts away the OS-specific vkCreateWin32SurfaceKHR or vkCreateXcbSurfaceKHR
    VkResult result = glfwCreateWindowSurface(m_Instance, static_cast<GLFWwindow*>(m_WindowHandle), nullptr, &m_Surface);
    if (result != VK_SUCCESS)
    {
        AXLOG_ERROR("Failed to create Window Surface!");
    }
    //AXEL_CORE_ASSERT(result == VK_SUCCESS, "Failed to create Window Surface!");
}
