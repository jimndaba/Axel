#include "axelpch.h"
#include "Texture.h"

#include <rendering/RenderAPI.h>
#include "backends/GraphicsContext.h"
#include "backends/GraphicsDevice.h"
#include "backends/vulkanbackend/VulkanTexture2D.h"

std::shared_ptr<Axel::Texture2D>Axel::Texture2D::Create(uint32_t width, uint32_t height, const unsigned char* data)
{
    switch (RendererAPI::GetAPI())
    {
    case RendererAPI::API::None:
        AXEL_CORE_ASSERT(false, "RendererAPI::None is not supported!");
        return nullptr;

    case RendererAPI::API::Vulkan:
        return std::make_shared<VulkanTexture2D>(width,height,data);
    }

    AXEL_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}
