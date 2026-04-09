#pragma once
#ifndef SHADER_H
#define SHADER_H


#include <core/Core.h>
#include "GraphicsCore.h"
#include <assets/AssetBase.h>

namespace Axel
{
	using SetIndex = uint32_t;
	using BindingIndex = uint32_t;

	class GraphicsDevice;
	class AX_API Shader : public IAsset
	{
		

	public:
		virtual ~Shader() = default;
		
		std::map<SetIndex, std::map<BindingIndex, ShaderResource>> GetResources() const { return m_Resources; }
		static Ref<Shader> Create(GraphicsDevice& device,const std::string& filepath);		
		virtual void Destroy() = 0;
		virtual AssetTypeOptions GetType() const { return AssetTypeOptions::Shader; }
		std::map<SetIndex, std::map<BindingIndex, ShaderResource>> m_Resources;

	};
}

#endif