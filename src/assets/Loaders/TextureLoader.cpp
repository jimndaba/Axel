#include "axelpch.h"
#include "TextureLoader.h"

#include <rendering/Texture.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <core/Logger.h>

std::shared_ptr<Axel::Texture2D> Axel::TextureLoader::Load(const std::string& path)
{
    int width, height, channels;
    // stb_image handles the heavy lifting
    // Force 4 channels (RGBA) for consistency in the shader
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);

    if (!data) {
        AXLOG_ERROR("Failed to load texture at path: {0}", path);
        return nullptr;
    }
    // Create the actual engine object (which sends data to the GPU)
    Ref<Texture2D> texture = Texture2D::Create( width, height, data);

    stbi_image_free(data);
    return texture;
}
