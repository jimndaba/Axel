#pragma once
#ifndef VULKANSHADER_H
#define VULKANSHADER_H




#include <vulkan/vulkan.h>	
#include <rendering/Shader.h>


namespace Axel
{
	class VulkanDevice;

	class VulkanShader : public Shader
	{
	public:
		VulkanShader(GraphicsDevice& device, const std::map<ShaderStage, std::string>& shaderFiles);
		virtual ~VulkanShader();
		//void Bind() const;
		//void Unbind() const;

		// The Bridge: VulkanPipeline calls this to build the pipeline
		virtual void Destroy() override;
		const std::vector<VkPipelineShaderStageCreateInfo>& GetVulkanStages() const { return m_StageInfos; }
		void Reflect(const std::vector<uint32_t>& spirvCode, ShaderStage stage);
	private:
		void CreateModule(ShaderStage stage, const std::vector<uint32_t>& code);

		GraphicsDevice& m_Device;
		std::vector<VkPipelineShaderStageCreateInfo> m_StageInfos;
		std::map<ShaderStage, VkShaderModule> m_Modules;
	};
}

#endif