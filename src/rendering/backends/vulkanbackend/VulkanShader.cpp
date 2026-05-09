#include "axelpch.h"
#include "VulkanShader.h"
#include "VulkanDevice.h"
#include "VulkanUtils.h"
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

Axel::DescriptorType MapType(SpvReflectDescriptorType type) {
    switch (type) {
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:         return Axel::DescriptorType::UniformBuffer;
    case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: return Axel::DescriptorType::ImageSampler;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:        return Axel::DescriptorType::StorageBuffer;
    case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:        return Axel::DescriptorType::StorageImage;
    default: return  Axel::DescriptorType::None;
    }
}

Axel::ShaderDataType SpirvToAxelType(SpvReflectFormat format) {
    switch (format) {
    case SPV_REFLECT_FORMAT_R32_SFLOAT:          return Axel::ShaderDataType::Float;
    case SPV_REFLECT_FORMAT_R32G32_SFLOAT:       return Axel::ShaderDataType::Float2;
    case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT:    return Axel::ShaderDataType::Float3;
    case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT: return Axel::ShaderDataType::Float4;
    case SPV_REFLECT_FORMAT_R32_SINT:            return Axel::ShaderDataType::Int;
        // ... add others ...
    default: return Axel::ShaderDataType::None;
    }
}

Axel::PropertyType MapMemberType(const SpvReflectBlockVariable& var) {
    // Safety check for the pointer
    if (!var.type_description) return Axel::PropertyType::Unknown;

    auto flags = var.type_description->type_flags;

    if (flags & SPV_REFLECT_TYPE_FLAG_FLOAT) {
        auto components = var.numeric.vector.component_count;
        if (components == 1) return Axel::PropertyType::Float;
        if (components == 2) return Axel::PropertyType::Vec2;
        if (components == 3) return Axel::PropertyType::Vec3;
        if (components == 4) return Axel::PropertyType::Vec4;
    }

    if (flags & SPV_REFLECT_TYPE_FLAG_INT)  return Axel::PropertyType::Int;
    if (flags & SPV_REFLECT_TYPE_FLAG_BOOL) return Axel::PropertyType::Bool;

    return Axel::PropertyType::Unknown;
}

Axel::VulkanShader::VulkanShader(GraphicsDevice& device, const std::map<ShaderStage, std::string>& shaderFiles) :m_Device(device)
{
	auto vdevice = static_cast<VulkanDevice*>(&device);
    for (auto& [stage, path] : shaderFiles) {
        auto code = ReadSPIRVFile(path);

        // Only reflect Vertex Layout for the Vertex Stage
        // This prevents the Fragment stage from overwriting your attributes
        if (stage == ShaderStage::Vertex) {
            ReflectVertexLayout(code); // Dedicated function for inputs
        }

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

    uint32_t count = 0;
    spvReflectEnumerateDescriptorSets(&module, &count, nullptr);
    std::vector<SpvReflectDescriptorSet*> sets(count);
    spvReflectEnumerateDescriptorSets(&module, &count, sets.data());

    for (auto* set : sets) {
        for (uint32_t i = 0; i < set->binding_count; ++i) {
            auto* binding = set->bindings[i];

            // 1. Check if resource exists (stage merging)
            if (m_Resources[binding->set].count(binding->binding)) {
                m_Resources[binding->set][binding->binding].Stage |= stage;
                continue;
            }

            // 2. Setup basic resource info
            DescriptorBinding resource{};
            resource.Name = binding->name;
            resource.Binding = binding->binding;
            resource.Stage = stage;
            resource.Count = (binding->array.dims_count > 0) ? binding->array.dims[0] : 1;
            resource.Type = MapType(binding->descriptor_type);

            // 3. Reflect internal members for Buffers (UBO/SSBO)
            if (resource.Type == DescriptorType::UniformBuffer || resource.Type == DescriptorType::StorageBuffer) {
                resource.Size = binding->block.size;

                // Handle potential runtime arrays or naming prefixes
                std::string prefix = "";
                SpvReflectBlockVariable* rootBlock = &binding->block;

                // If the root is a wrapper, drill down
                if (rootBlock->member_count == 1 && rootBlock->members[0].type_description->op == SpvOpTypeRuntimeArray) {
                    prefix = std::string(rootBlock->members[0].name) + ".";
                    rootBlock = &rootBlock->members[0];
                }

                // Populate the Members vector for the Material Editor
                for (uint32_t j = 0; j < rootBlock->member_count; ++j) {
                    auto& spvMember = rootBlock->members[j];

                    ShaderMember member;
                    member.Name = prefix + spvMember.name;
                    member.Offset = spvMember.offset;
                    member.Size = spvMember.size;
                    member.Type = MapMemberType(spvMember); // Your existing mapping function

                    resource.Members.push_back(member);
                }
            }

            // 4. Store in the nested map
            m_Resources[binding->set][binding->binding] = resource;
        }
    }

    // 5. Push Constants
    uint32_t pushCount = 0;
    spvReflectEnumeratePushConstantBlocks(&module, &pushCount, nullptr);
    if (pushCount > 0) {
        std::vector<SpvReflectBlockVariable*> blocks(pushCount);
        spvReflectEnumeratePushConstantBlocks(&module, &pushCount, blocks.data());

        for (auto* block : blocks) {
            PushConstantRange range;
            range.Offset = block->offset;
            range.Size = block->size;
            range.Stages = stage;
            m_PushConstantRanges.push_back(range);
        }
    }

    spvReflectDestroyShaderModule(&module);
}

void  Axel::VulkanShader::ReflectVertexLayout(const std::vector<uint32_t>& code) {
    SpvReflectShaderModule module;
    spvReflectCreateShaderModule(code.size() * 4, code.data(), &module);

    uint32_t count = 0;
    spvReflectEnumerateInputVariables(&module, &count, nullptr);
    std::vector<SpvReflectInterfaceVariable*> inputs(count);
    spvReflectEnumerateInputVariables(&module, &count, inputs.data());

    std::vector<BufferElement> elements;
    for (auto* input : inputs) {
        if (input->built_in != -1) continue;
        // Use your updated BufferElement with Location
        elements.push_back({
            SpirvToAxelType(input->format),
            input->name,
            input->location
            });
    }

    // Final truth: The BufferLayout for the whole shader
    m_VertexLayout = BufferLayout(elements);
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
