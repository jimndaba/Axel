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
		PolygonModeOptions FillMode = PolygonModeOptions::Fill;
		CullModeOptions FaceCulling = CullModeOptions::Back;
		BlendingModeOptions BlendMode = BlendingModeOptions::None;

		bool DepthTest = true;
		bool DepthWrite = true;
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


