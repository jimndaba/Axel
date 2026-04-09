#pragma once
#ifndef DESCRIPTORSET_H
#define DESCRIPTORSET_H	

#include <core/Core.h>


namespace Axel
{
	class GraphicsContext;
	class UniformBuffer;
	class Texture2D;
	class ShaderStorageBuffer;
	class Pipeline;

	class AX_API DescriptorSet
	{
	public:
		virtual ~DescriptorSet() = default;

		// "Write" data into the set by Name (matches GLSL name)
		virtual void Write(const std::string& name, const Ref<UniformBuffer>& buffer) = 0;
		virtual void Write(const std::string& name, const Ref<ShaderStorageBuffer>& buffer) = 0;
		virtual void Write(const std::string& name, const Ref<Texture2D>& texture) = 0;

		// The "Commit" or "Update" step that actually tells the GPU 
		virtual void Update() = 0;
		virtual void Destroy() = 0;	
		static Ref<DescriptorSet> Create(GraphicsContext* ctxt, const Ref<Pipeline>& pipeline, uint32_t setIndex);

	};
}


#endif