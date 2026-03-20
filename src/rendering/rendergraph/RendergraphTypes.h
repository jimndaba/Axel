#pragma once
#ifndef RENDERGRAPHTYPES_H
#define RENDERGRAPHTYPES_H

#include "core/Core.h"

namespace Axel
{
    using RenderGraphResourceID = uint32_t;

    enum class AttachmentType {
        Color, Depth, Resolve
    };

    struct RenderGraphTextureSpec {
        uint32_t Width, Height;
        ImageFormat Format;  // Clean!
        AttachmentType Type;
    };

}



#endif