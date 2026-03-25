#pragma once
#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include <core/Core.h>

namespace Axel
{
	class Texture2D;
	class AX_API TextureLoader
	{
	public:
		static Ref<Texture2D> Load(const std::string& path);
	};
}


#endif // !
