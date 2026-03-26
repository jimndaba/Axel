#include "axelpch.h"
#include "VulkanContext.h"
#include "GLFW/glfw3.h"
#include <stdexcept>
#include "../../../core/Logger.h"
#include <rendering/backends/FrameBuffer.h>
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"

#include <events/EventBus.h>
#include <platform/Platform.h>


#include <rendering/backends/vulkanbackend/VulkanDevice.h>
#include <rendering/RenderAPI.h>
#include <rendering/backends/vulkanbackend/VulkanRenderAPI.h>


Axel::VulkanContext::VulkanContext(void* windowHandle) : m_WindowHandle(windowHandle) {
    AXEL_CORE_ASSERT(m_WindowHandle, "Window handle is null!")
}

Axel::VulkanContext::~VulkanContext()
{
    

  
}

void Axel::VulkanContext::Init()
{
    Eventbus::Subscribe(this, &Axel::VulkanContext::OnWindowResize);
    CreateInstance();
    CreateSurface();
   
	
    // 3. Initialize the Device (The GPU hardware)
    // We pass 'this' so the device can access the Instance and Surface
	m_Device = CreateRef<VulkanDevice>(this);
    m_Device->Init();
	auto vulkanDevice = std::dynamic_pointer_cast<VulkanDevice>(m_Device);

    m_CommandPool = CreateRef<VulkanCommandPool>(*vulkanDevice);

    
    m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    m_CommandBufferObjects.resize(MAX_FRAMES_IN_FLIGHT);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        // Allocate from your m_CommandPool
        m_CommandBuffers[i] = m_CommandPool->AllocateBuffer();
        m_CommandBufferObjects[i] = std::make_shared<VulkanCommandBuffer>(*vulkanDevice, m_CommandBuffers[i]);
    }
    

    
    // 4. INITIALIZE THE SWAPCHAIN
    // It needs the device to create handles, the surface to display on, 
    // and the window size to determine the resolution.
    VkExtent2D extent = { m_WindowWidth, m_WindowHeight }; // TODO: Update window widgr height
    m_Swapchain = CreateScope<VulkanSwapchain>(*vulkanDevice, m_Surface, extent);

    CreateSyncObjects();
    CreateRenderPass();
    CreateFramebuffers();
	CreateDescriptorPool();

	m_API = CreateRef<VulkanRenderAPI>(this);
	m_API->Init();
	m_APIType = RenderAPI::API::Vulkan;
    AXLOG_INFO("VulkanRenderAPI created");

}

void  Axel::VulkanContext::CreateSyncObjects() {
    auto device = std::static_pointer_cast<VulkanDevice>(m_Device)->GetLogicalDevice();

    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

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
        m_SwapChainFramebuffers[i] = Framebuffer::Create(this,spec);
    }
}

void Axel::VulkanContext::CreateDescriptorPool()
{    
    auto device = std::static_pointer_cast<VulkanDevice>(m_Device)->GetLogicalDevice();
    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 }, // Max 100 UBOs
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 } // Max 100 Textures
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = 100; // Max 100 "Materials" or "Sets" total

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
        AXLOG_ERROR("Failed to create Descriptor Pool!");
    }
}

void Axel::VulkanContext::CreateRenderPass()
{
    RenderPassSpecification spec;
    spec.DebugName = "MainSwapchainPass";

    // Map the swapchain format to your engine's ImageFormat enum
    // Assuming you have a helper like VulkanImage::VulkanFormatToAxelFormat
    spec.Format = ImageFormat::RGBA8;
    spec.LoadOp = AttachmentLoadOp::Clear;   // This triggers the Blue Clear
    spec.StoreOp = AttachmentStoreOp::Store; // This saves the Blue Clear to show us
    spec.ClearColor = { 0.1f, 0.1f, 0.8f, 1.0f }; // Axel Blue
    m_MainRenderPass = RenderPass::Create(this,spec);
}

void Axel::VulkanContext::DestroyFramebuffers()
{
    //for (auto framebuffer : m_SwapChainFramebuffers) {
	//	auto handle = std::static_pointer_cast<VulkanFramebuffer>(framebuffer)->GetHandle();        
    //    vkDestroyFramebuffer(m_Device->GetLogicalDevice(), handle, nullptr);
   // }
    m_SwapChainFramebuffers.clear();
}

void Axel::VulkanContext::SwapBuffers()
{
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

Axel::RenderAPI::API Axel::VulkanContext::GetCurrentAPI() const
{
    return RenderAPI::API::Vulkan;
}

void Axel::VulkanContext::OnWindowResize(const std::shared_ptr<WindowResizeEvent>& e)
{
    
    
    if (e->Width == 0 || e->Height == 0) return; // Handle minimization    
    m_WindowWidth = e->Width ;    
    m_WindowHeight = e->Height;
    auto device = std::static_pointer_cast<VulkanDevice>(m_Device);

    // Wait for the GPU to finish its current work
    vkDeviceWaitIdle(device->GetLogicalDevice());

    // Re-initialize the swapchain with the new dimensions
    m_Swapchain->RecreateSwapchain(); // Assuming Recreate handles the internal cleanup
	DestroyFramebuffers(); // Clean up old framebuffers that depended on the old swapchain

    CreateFramebuffers();
}

std::shared_ptr<Axel::Framebuffer> Axel::VulkanContext::GetCurrentFramebuffer()
{
    return m_SwapChainFramebuffers[m_CurrentImageIndex];;
}

std::shared_ptr<Axel::RenderPass > Axel::VulkanContext::GetMainRenderPass()
{
    return m_MainRenderPass;
}

std::shared_ptr<Axel::RenderCommandBuffer> Axel::VulkanContext::GetCurrentCommandBuffer()
{
    // 1. Safety check to ensure buffers exist
    AXEL_CORE_ASSERT(currentFrame < m_CommandBuffers.size(), "Frame index out of bounds!");

    // 2. Wrap the raw VkCommandBuffer handle in our Axel wrapper
    // We pass the raw handle so the Renderer can call commands on it.
    return m_CommandBufferObjects[currentFrame];;
}

void Axel::VulkanContext::BeginFrame()
{
    VkDevice device = std::static_pointer_cast<VulkanDevice>(m_Device)->GetLogicalDevice();

    // 1. Wait for the GPU to finish the frame from 2 cycles ago (Frames in Flight)
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // 3. Reset the fence so we can use it for the new submission
    vkResetFences(device, 1, &inFlightFences[currentFrame]);
    // 2. Acquire the next image from the swapchain
    // This signals imageAvailableSemaphores[m_CurrentFrame] when the image is ready
    // 2. ACQUIRE AND STORE THE INDEX
    // Change your AcquireNextImage to return the uint32_t index!
    m_CurrentImageIndex = m_Swapchain->AcquireNextImage(imageAvailableSemaphores[currentFrame]);
  

    auto commandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(GetCurrentCommandBuffer());
    vkResetCommandBuffer(commandBuffer->GetHandle(), 0);

    // 3. Start Recording
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer->GetHandle(), &beginInfo);
}

void Axel::VulkanContext::EndFrame()
{
    auto device = std::static_pointer_cast<VulkanDevice>(m_Device);
    auto graphicsQueue = device->GetGraphicsQueue();
    auto presentQueue = device->GetPresentQueue();

    // 1. Stop Recording
    vkEndCommandBuffer(m_CommandBufferObjects[currentFrame]->GetHandle());

    // 2. Submit (Using the pointers we fixed earlier)
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // Wait for the image to be available from the swapchain before writing to it
    VkSemaphore waitSemaphores[] = { GetImageAvailableSemaphore() };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    // The actual recorded commands
    auto commandBuffer = std::static_pointer_cast<VulkanCommandBuffer>(GetCurrentCommandBuffer())->GetHandle();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // Signal this semaphore when the GPU is finished rendering
    VkSemaphore signalSemaphores[] = { GetRenderFinishedSemaphore() };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    // Reset the fence so we can wait on it next frame
    //VkFence fence = GetInFlightFence();  
    //vkResetFences(device, 1, &fence);

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
       AXLOG_ERROR("Failed to submit draw command buffer!");
    }

    // 2. Present the result to the screen
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores; // Wait for rendering to finish

    VkSwapchainKHR swapChains[] = { m_Swapchain->GetHandle() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

   
    m_CurrentImageIndex = m_Swapchain->GetCurrentImageIndex();
    presentInfo.pImageIndices = &m_CurrentImageIndex;

    VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // Handle Window Resize here!
        m_Swapchain->RecreateSwapchain();
    }
    else if (result != VK_SUCCESS) {
        AXEL_CORE_ASSERT(false, "Failed to present swap chain image!");
    }
}

void Axel::VulkanContext::Shutdown()
{
    // 1. Wait for GPU to finish all work before we start tearing down
    VkDevice device = std::static_pointer_cast<VulkanDevice>(m_Device)->GetLogicalDevice();
    vkDeviceWaitIdle(device);

    // 2. Destroy Framebuffers first (they depend on the RenderPass and Swapchain)
    // Clearing the vector triggers the Ref<> destructors
    m_SwapChainFramebuffers.clear();

    // 3. Destroy Render Pass
    m_MainRenderPass->Destroy();
 

    // 4. Destroy Sync Objects
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    // 5. Cleanup Command Infrastructure
    if (m_CommandPool) {
        // Note: Destroying the pool automatically frees all command buffers allocated from it
        vkDestroyCommandPool(device, m_CommandPool->GetHandle(), nullptr);
    }

    // 6. Cleanup Swapchain & Surface
    if (m_Swapchain) {
        m_Swapchain->cleanupSwapChain();
    }

    if (m_Surface) {
        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    }

    if(m_DescriptorPool) {
        vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
	}

    // 7. Finally, destroy the Logical Device and Instance
    // If m_Device is a Scope/UniquePtr, it will destroy itself here

    // 2. Destroy the Logical Device
    if (device) {
		m_Device->Shutdown(); // Ensure the device cleans up its own resources first      
    }
    m_Device.reset();

    if (m_Instance) {
        vkDestroyInstance(m_Instance, nullptr);
    }

    if (m_API) {
        m_API->Shutdown();
        m_API = nullptr;
    }
}

void Axel::VulkanContext::Submit(Ref<RenderCommandBuffer> cmd)
{
}

void Axel::VulkanContext::CreateInstance()
{
    auto device = std::static_pointer_cast<VulkanDevice>(m_Device);
    if (enableValidationLayers && !device->checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }


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
