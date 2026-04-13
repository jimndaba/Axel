#include "axelpch.h"

#include "VulkanPipeline.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "VulkanRenderPass.h"
#include "VulkanShader.h"
#include <core/Utils.h>


Axel::VulkanPipeline::VulkanPipeline(const PipelineSpecification& spec, VulkanDevice& device):m_Specification(spec), m_Device(device)
{
    auto logicalDevice = m_Device.GetLogicalDevice();
    auto vShader = std::static_pointer_cast<VulkanShader>(spec.Shader);

    // Inside VulkanPipeline.cpp
    const auto& resources = vShader->GetResources();

    // 1. Find the highest set index to determine array size
    uint32_t maxSet = 0;
    for (auto const& [setIndex, bindings] : resources) {
        maxSet = std::max(maxSet, setIndex);
    }

    // We need one Layout for each Set (Set 0, Set 1, etc.)
    std::vector<VkDescriptorSetLayout> layouts(maxSet + 1);
    for (auto& [setIndex, bindingsMap] : resources) {
        std::vector<VkDescriptorSetLayoutBinding> vkBindings;

        for (auto& [bindingIndex, resource] : bindingsMap) {
            VkDescriptorSetLayoutBinding b{};
            b.binding = resource.Binding;
            b.descriptorType = AxelToVulkanType(resource.Type);
            b.descriptorCount = resource.Count;
            b.stageFlags = AxelStageToVulkan(resource.Stage);
            b.pImmutableSamplers = nullptr;
            vkBindings.push_back(b);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(vkBindings.size());
        layoutInfo.pBindings = vkBindings.data();

        VkDescriptorSetLayout layout;
        if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &layout) != VK_SUCCESS) {
            // Handle error (Logging, etc)
        }

        // CRITICAL: Assign to the specific index, do NOT push_back
        layouts[setIndex] = layout;

        // Store in your member map for binding calls later
        m_DescriptorSetLayouts[setIndex] = layout;
    }

    /*
    for (uint32_t i = 0; i < layouts.size(); ++i) {
        if (layouts[i] == VK_NULL_HANDLE) {
            // You should have a single "EmptyLayout" created at engine init 
            // to plug holes in the descriptor sets.
            layouts[i] = m_Device.GetEmptyDescriptorSetLayout();
        }
    }
    */

    // Collect all layouts from the map into a sorted vector
    std::vector<VkDescriptorSetLayout> allLayouts;

    for (auto const& [setIndex, layout] : m_DescriptorSetLayouts) {
        allLayouts.push_back(layout);
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(allLayouts.size());
    pipelineLayoutInfo.pSetLayouts = allLayouts.data();

    // --- 3. Pipeline Layout (Uniforms/Push Constants) ---
    // For now, we create an empty layout. Later, this comes from the Shader Reflection.
    std::vector<VkPushConstantRange> vkRanges;
    for (const auto& range : vShader->GetPushConstantRanges()) {
        VkPushConstantRange vkRange;
        vkRange.stageFlags = AxelStageToVulkan(range.Stages);
        vkRange.offset = range.Offset;
        vkRange.size = range.Size;
        vkRanges.push_back(vkRange);
    } 
    pipelineLayoutInfo.pushConstantRangeCount = (uint32_t)vkRanges.size();
    pipelineLayoutInfo.pPushConstantRanges = vkRanges.data();

    if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &m_Layout) != VK_SUCCESS) {
        AXEL_CORE_ASSERT(false, "Failed to create pipeline layout!");
    }


    // --- 1. Vertex Input State ---
    const auto& layout = spec.Layout;
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = layout.GetStride();
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
    uint32_t location = 0;
    for (const auto& element : layout.GetElements()) {
        VkVertexInputAttributeDescription attr{};
        attr.binding = 0;
        attr.location = location++;
        attr.format = AxelShaderDataTypeToVulkanFormat(element.Type);
        attr.offset = element.Offset;
        attributeDescriptions.push_back(attr);
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // --- 2. Rasterization State (Using your Enums) ---
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = AxelPolygonModeToVulkan(spec.FillMode);
    rasterizer.cullMode = AxelCullModeToVulkan(spec.FaceCulling);
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // Standard for Axel
    rasterizer.lineWidth = 1.0f;
     

    // --- 4. The Final Pipeline ---
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    // Shader Stages
   
    const auto& stages = vShader->GetVulkanStages();
    pipelineInfo.stageCount = (uint32_t)stages.size();
    pipelineInfo.pStages = stages.data();
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.layout = m_Layout;

    
   

    // Attach to the RenderPass
    auto vRenderPass = std::static_pointer_cast<Axel::VulkanRenderPass>(spec.TargetRenderPass);
    pipelineInfo.renderPass = vRenderPass->GetHandle();
    pipelineInfo.subpass = 0;

    // ... Other states (Viewport, Scissor, Multisampling, ColorBlend, DepthStencil) ...
    std::vector<VkDynamicState> dynamicStates = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();
    pipelineInfo.pDynamicState = &dynamicState;

    // This is for a single color attachment (our Swapchain)
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE; // Enable transparency!
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
	pipelineInfo.pColorBlendState = &colorBlending;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = spec.DepthTest ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS; // Standard "closer is better"
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
	pipelineInfo.pDepthStencilState = &depthStencil;    

    // 1. Create the multisample state (Even if not using MSAA)
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // Standard 1 sample per pixel
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;
    pipelineInfo.pMultisampleState = &multisampling;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // THE triangle setting
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    pipelineInfo.pInputAssemblyState = &inputAssembly; // This was NULL

    // Need dummy Viewport state because we used Dynamic State
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    pipelineInfo.pViewportState = &viewportState;
    
    if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline) != VK_SUCCESS) {
        AXEL_CORE_ASSERT(false, "Failed to create graphics pipeline!");
    }
}

Axel::VulkanPipeline::~VulkanPipeline()
{
}

void Axel::VulkanPipeline::Bind(const GraphicsContext& context) const
{
    // 1. Get the handle from the context (the buffer currently recording)
	auto& vkContext = static_cast<const VulkanContext&>(context);
	VkCommandBuffer commandBuffer = vkContext.GetActiveCommandBuffer();

    // 2. Bind the Graphics Pipeline
    // VK_PIPELINE_BIND_POINT_GRAPHICS tells Vulkan this isn't a Compute or RayTracing bind
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

    // 3. Set Dynamic States
    // Since we enabled these in the constructor, we MUST set them now or the draw will fail
    auto& swapchain = *vkContext.GetSwapchain();
    VkExtent2D extent = swapchain.GetExtent();

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = extent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Axel::VulkanPipeline::Unbind() const
{
}

void Axel::VulkanPipeline::Destroy()
{
	if (m_Pipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(m_Device.GetLogicalDevice(), m_Pipeline, nullptr);
     if(m_Layout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(m_Device.GetLogicalDevice(), m_Layout, nullptr);
     for (auto& [setIndex, layout] : m_DescriptorSetLayouts) {
         vkDestroyDescriptorSetLayout(m_Device.GetLogicalDevice(), layout, nullptr);
     }
     m_Specification.Shader->Destroy();

}
