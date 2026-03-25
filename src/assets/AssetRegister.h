#pragma once
#ifndef ASSETREGISTER_H	
#define ASSETREGISTER_H

#include <core/Core.h>

namespace Axel
{
	class AssetRegister
	{
	public:
		static void RegisterAsset(const char* name, uint64_t uuid);
		static uint64_t GetAssetUUID(const char* name);
	};
}



#endif