#pragma once
#ifndef PIPELINE_H
#define PIPELINE_H	

#include <core/Core.h>
#include "BufferLayout.h"

namespace Axel
{
	class Shader;
	class RenderPass;
	class GraphicsContext;


	struct AX_API PipelineSpecification {

		Ref<Shader> Shader;
		BufferLayout Layout;
		PolygonMode FillMode = PolygonMode::Fill;
		CullMode FaceCulling = CullMode::Back;
		bool DepthTest = true;
		Ref<RenderPass> TargetRenderPass;
	};

	class AX_API Pipeline
	{
	public:
		virtual ~Pipeline() = default;
		virtual void Bind(const GraphicsContext& context) const = 0;
		virtual void Unbind() const = 0;
		virtual void Destroy() = 0;
		static Ref<Pipeline> Create(GraphicsContext* ctxt, const PipelineSpecification& spec);
	};

}








#endif


