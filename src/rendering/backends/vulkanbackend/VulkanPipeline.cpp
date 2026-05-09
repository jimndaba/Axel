#include "axelpch.h"

#include "VulkanPipeline.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "VulkanRenderPass.h"
#include "VulkanShader.h"
#include "VulkanUtils.h"
#include <rendering/GraphicsContext.h>
#include "VulkanDescriptorSetLayout.h"
#include "VulkanCommandBuffer.h"

Axel::VulkanPipeline::VulkanPipeline(const PipelineSpecification& spec, GraphicsContext& cntxt):m_Specification(spec), 
m_Device(*std::static_pointer_cast<VulkanDevice>(cntxt.GetDevice()))
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

    for (auto& [setIndex, bindingsMap] : resources)
    {

        std::vector<DescriptorBinding> bindings;
        for (auto& [bindingIndex, resource] : bindingsMap)
        {
            DescriptorBinding b = resource;
            b.Binding = bindingIndex; // Assign the real shader index to the layout
            bindings.push_back(b);
        }

        // Create the platform-agnostic layout (which should internally handle vkCreateDescriptorSetLayout)
        // You likely have a factory method or a constructor for VulkanDescriptorSetLayout        // Store in your member map
        m_DescriptorSetLayouts[setIndex] = DescriptorSetLayout::Create(&cntxt, bindings);       
    }

    // 2. Prepare the raw handles for the Pipeline Layout
    if (!m_DescriptorSetLayouts.empty()) {
        maxSet = m_DescriptorSetLayouts.rbegin()->first;
    }

    // Vulkan requires an array of layouts. If sets are missing (e.g., set 0 and set 2 exist, but not 1),
    // you must provide a null or empty layout for the gaps, or ensure your shader reflection 
    // accounts for contiguous sets.
    std::vector<VkDescriptorSetLayout> vkRawLayouts(maxSet + 1);

    for (uint32_t i = 0; i <= maxSet; ++i)
    {
        if (m_DescriptorSetLayouts.find(i) != m_DescriptorSetLayouts.end())
        {
            // Cast to your Vulkan implementation and get the handle
            auto vkLayout = std::static_pointer_cast<VulkanDescriptorSetLayout>(m_DescriptorSetLayouts[i]);
            vkRawLayouts[i] = (VkDescriptorSetLayout)vkLayout->GetNativeLayout();
        }
        else
        {
            // Handle gaps in set indices with an empty layout if necessary, 
            // though usually, reflection provides contiguous sets.
            vkRawLayouts[i] = CreateEmptyDescriptorSetLayout(logicalDevice);
        }
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(vkRawLayouts.size());
    pipelineLayoutInfo.pSetLayouts = vkRawLayouts.data();

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
   

    uint32_t offset = 0;
    auto elements = layout.GetElements();
    // 1. Sort by Location first
    std::sort(elements.begin(), elements.end(), [](const auto& a, const auto& b) {
        return a.Location < b.Location;
        });

    for (auto& element : elements) {
        element.Offset = offset;
        offset += element.Size; // Ensure Size is 12 for vec3, 8 for vec2
    }
    uint32_t stride = offset; // This should be 32
    uint32_t location = 0;
    for (const auto& element : elements) {
        VkVertexInputAttributeDescription attr{};
        attr.binding = 0;
        attr.location =element.Location;
        attr.format = AxelShaderDataTypeToVulkanFormat(element.Type);
        attr.offset = element.Offset;
        attributeDescriptions.push_back(attr);
    }

 

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    if (elements.empty())
    {
        // FOR THE GRID: No buffers required
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr;
    }
    else
    {
        // FOR STANDARD MESHES: Use your existing logic
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    }
    
    // --- 2. Rasterization State (Using your Enums) ---
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = AxelPolygonModeToVulkan(spec.FillMode);
    rasterizer.cullMode = AxelCullModeToVulkan(spec.FaceCulling);
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; // Standard for Axel
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
    colorBlendAttachment.blendEnable = m_Specification.EnableBlending ? VK_TRUE : VK_FALSE; // Enable transparency!
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
	pipelineInfo.pColorBlendState = &colorBlending;



    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = spec.DepthTest ? VK_TRUE : VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; // Standard "closer is better"
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

VkDescriptorSetLayout  Axel::VulkanPipeline::CreateEmptyDescriptorSetLayout(VkDevice device)
{
    VkDescriptorSetLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = 0;
    info.pBindings = nullptr;

    VkDescriptorSetLayout layout;
    vkCreateDescriptorSetLayout(device, &info, nullptr, &layout);
    return layout;
}

void Axel::VulkanPipeline::Bind(const GraphicsContext& context) const
{
    // Safely cast the base GraphicsContext to the VulkanContext at runtime.
    // Use dynamic_cast on a pointer (with const_cast to get a non-const pointer)
    // so we can call non-const VulkanContext methods from a const Bind().
    auto* vkContext = static_cast<const VulkanContext*>(&context); // Get address, then cast
    auto buffer = static_cast<VulkanCommandBuffer*>(vkContext->GetCurrentCommandBuffer().get());
    VkCommandBuffer commandBuffer = buffer->GetHandle();

    // 2. Bind the Graphics Pipeline
    // VK_PIPELINE_BIND_POINT_GRAPHICS tells Vulkan this isn't a Compute or RayTracing bind
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);   
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
         
         vkDestroyDescriptorSetLayout(m_Device.GetLogicalDevice(), (VkDescriptorSetLayout)layout->GetNativeLayout(), nullptr);
     }
     m_Specification.Shader->Destroy();

}
