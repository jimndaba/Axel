#include "axelpch.h"
#include "VulkanShader.h"
#include "VulkanDevice.h"
#include <core/Utils.h>
#include <fstream>
#include <memory>
#include <spirv_refl/spirv_reflect.h>

std::vector<uint32_t> ReadSPIRVFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);
    size_t fileSize = (size_t)file.tellg();
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
    file.seekg(0);
    file.read((char*)buffer.data(), fileSize);
    file.close();
    return buffer;
}

Axel::ShaderResourceType MapType(SpvReflectDescriptorType type) {
    switch (type) {
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:         return Axel::ShaderResourceType::UniformBuffer;
    case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return Axel::ShaderResourceType::CombinedImageSampler;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:        return Axel::ShaderResourceType::StorageBuffer;
    default: return  Axel::ShaderResourceType::None;
    }
}

Axel::VulkanShader::VulkanShader(GraphicsDevice& device, const std::map<ShaderStage, std::string>& shaderFiles) :m_Device(device)
{
	auto vdevice = static_cast<VulkanDevice*>(&device);
    for (auto& [stage, path] : shaderFiles) {
        auto code = ReadSPIRVFile(path);


		Reflect(code, stage);

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size() * sizeof(uint32_t);
        createInfo.pCode = code.data();

        VkShaderModule module;
        vkCreateShaderModule(vdevice->GetLogicalDevice(), &createInfo, nullptr, &module);
        m_Modules[stage] = module;

        // Bake the stage info
        VkPipelineShaderStageCreateInfo stageInfo{};
        stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stageInfo.stage = AxelStageToVulkan(stage);
        stageInfo.module = module;
        stageInfo.pName = "main";

        m_StageInfos.push_back(stageInfo);
    }


}

Axel::VulkanShader::~VulkanShader()
{
}

void Axel::VulkanShader::Destroy()
{
    for(auto& [stage, module] : m_Modules) {
        vkDestroyShaderModule(static_cast<VulkanDevice*>(&m_Device)->GetLogicalDevice(), module, nullptr);
	}
}

void Axel::VulkanShader::Reflect(const std::vector<uint32_t>& spirvCode, ShaderStage stage)
{
    SpvReflectShaderModule module;
    SpvReflectResult result = spvReflectCreateShaderModule(spirvCode.size() * 4, spirvCode.data(), &module);

    // 1. Get all Descriptor Sets
    uint32_t count = 0;
    spvReflectEnumerateDescriptorSets(&module, &count, nullptr);
    std::vector<SpvReflectDescriptorSet*> sets(count);
    spvReflectEnumerateDescriptorSets(&module, &count, sets.data());

    // 2. Loop through every set and binding found in the SPV
    for (auto* set : sets) {
        for (uint32_t i = 0; i < set->binding_count; ++i) {
            auto* binding = set->bindings[i];

            ShaderResource resource{};
            resource.Name = binding->name;
            resource.Set = binding->set;
            resource.Binding = binding->binding;
            resource.Stage = stage;
			resource.Count = binding->array.dims_count > 0 ? binding->array.dims[0] : 1; // Handle arrays of resources

            // Map SPIRV-Reflect types to Axel types
            resource.Type = MapType(binding->descriptor_type);

            if (resource.Type == ShaderResourceType::UniformBuffer)
                resource.Size = binding->block.size;

            // Store in a map: m_Resources[set_index][binding_index]
            if (m_Resources[resource.Set].find(resource.Binding) != m_Resources[resource.Set].end()) {
                m_Resources[resource.Set][resource.Binding].Stage |= stage;
            }
            else {
                m_Resources[resource.Set][resource.Binding] = resource;
            }
        }
    }

    spvReflectDestroyShaderModule(&module);
}

void Axel::VulkanShader::CreateModule(ShaderStage stage, const std::vector<uint32_t>& code)
{
    auto vdevice = static_cast<VulkanDevice*>(&m_Device);
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    VkShaderModule module;
    if (vkCreateShaderModule(vdevice->GetLogicalDevice(), &createInfo, nullptr, &module) != VK_SUCCESS) {
        AXEL_CORE_ASSERT(false, "Failed to create shader module!");
    }

    m_Modules[stage] = module;

    // Create the Stage Info for the Pipeline
    VkPipelineShaderStageCreateInfo stageInfo{};
    stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo.module = module;
    stageInfo.pName = "main"; // Entry point (Standard for Axel)

    // Map Axel Stage to Vulkan Bit
    switch (stage) {
    case ShaderStage::Vertex:   stageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT; break;
    case ShaderStage::Fragment: stageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT; break;
    case ShaderStage::Compute:  stageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT; break;
    }

    m_StageInfos.push_back(stageInfo);
}
