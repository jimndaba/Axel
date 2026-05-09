#pragma once
#ifndef MESHRENDERSYSTEM_H
#define MESHRENDERSYSTEM_H

#include "IRenderSystem.h"

namespace Axel
{

	class MeshRenderSystem : public IRenderSystem
	{
	public:
		MeshRenderSystem() = default;
		void Submit(Scene& scene, SceneRenderDesc& desc) override;
		float ComputeDepth(const Vec3& worldPos, const SceneRenderDesc& desc);

	};




}

#endif 