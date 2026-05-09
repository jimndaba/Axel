#include "axelpch.h"
#include "Texture.h"

#include <rendering/RenderAPI.h>
#include "GraphicsContext.h"
#include "GraphicsDevice.h"
#include "backends/vulkanbackend/VulkanTexture2D.h"
#include <rendering/Renderer.h>

std::shared_ptr<Axel::Texture2D>Axel::Texture2D::Create(TextureCreationInfo& info)
{	
    switch (Renderer::GetGraphicsContext()->GetCurrentAPI())
    {
    case RenderAPI::API::None:
        AXEL_CORE_ASSERT(false, "RendererAPI::None is not supported!");
        return nullptr;

    case RenderAPI::API::Vulkan:
        return std::make_shared<VulkanTexture2D>(info);
    }

    AXEL_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
