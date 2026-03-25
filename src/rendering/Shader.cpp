#include "axelpch.h"
#include "Shader.h"
#include "backends/vulkanbackend/VulkanShader.h"
#include "backends/vulkanbackend/VulkanContext.h"

std::shared_ptr<Axel::Shader> Axel::Shader::Create(GraphicsDevice& device,const std::string& filepath)
{
    // We assume the user passed a base name like "assets/shaders/Triangle"
    std::map<ShaderStage, std::string> sources;
    sources[ShaderStage::Vertex] = filepath + "/vert.spv";
    sources[ShaderStage::Fragment] = filepath + "/frag.spv";
    return CreateRef<VulkanShader>(device, sources);
}
