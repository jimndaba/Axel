#pragma once
#ifndef SCENELOADER_H
#define SCENELOADER_H


#include <core/Core.h>

namespace Axel
{
	class Scene;
	class SceneLoader
	{

	public:
		SceneLoader() = default;

		static Ref<Scene> LoadScene(const std::string& path);



	};


}

#endif