#version 450
layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec3 a_Color;
layout(location = 2) in vec2 a_UV;


layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec2 fragTexCoord;

layout(binding = 0) uniform UniformBufferObject {
    mat4 projection;
} ubo;


void main() 
{ 
    v_Color = vec4(a_Color,1.0);
    fragTexCoord = a_UV;
    gl_Position = ubo.projection * vec4(a_Position,0.0, 1.0);
}
