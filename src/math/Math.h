#pragma once
#ifndef MATH_H
#define MATH_H


// 1. Vulkan uses 0.0 to 1.0 depth, whereas OpenGL uses - 1.0 to 1.0
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
// 2. Vulkan uses right-handed coordinates, but GLM defaults to OpenGL's clip space
#define GLM_FORCE_LEFT_HANDED

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Axel
{
    // You can alias common types to fit your engine's style
    using Vec2 = glm::vec2;
    using Vec3 = glm::vec3;
    using Vec4 = glm::vec4;
    using Mat4 = glm::mat4;
    using Quat = glm::quat;

    namespace MATRIX
    {
        inline Axel::Mat4 ORTHO(float left, float right, float bottom, float top, float nr, float fr)
        {
            return glm::ortho(left, right, bottom, top, nr, fr);
        }

    }
}


#endif