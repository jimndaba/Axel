#pragma once
#ifndef ASSETBASE_H
#define ASSETBASE_H

#include <core/Core.h>
#include <core/UUID.h>

namespace Axel
{
    enum class AssetTypeOptions { None = 0, Texture2D, Mesh, Shader, Material, Scene };

    class AX_API  Asset {
    public:
        UUID AssetID; // The unique ID for this instance
        virtual AssetTypeOptions GetType() const = 0;
    };
}

#endif // 
