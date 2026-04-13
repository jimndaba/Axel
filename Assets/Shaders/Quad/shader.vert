#version 450

const int Max_Sprites = 10000;

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec3 a_Color;
layout(location = 2) in vec2 a_UV;


// Output to Fragment Shader
layout(location = 0) out vec4 v_Color;
layout(location = 1) out vec2 v_TexCoord;
layout(location = 2) out flat  uint v_MaterialIndex; // Passed from Vertex


layout(binding = 0) uniform UniformBufferObject {
    mat4 ViewProjection;
} ubo;

struct SpritePacket {
    mat4 Transform;
    vec4 Color;
    uint TextureIndex;
    uint MaterialIndex;
    uint _padding[2]; // Keep 16-byte alignment if needed
};

layout(std430, binding = 1) readonly buffer SpriteData {
    SpritePacket packets[];
} ssbo;


void main() 
{ 
    SpritePacket data = ssbo.packets[gl_InstanceIndex];
    v_Color = data.Color;
    v_TexCoord = a_UV;
    v_MaterialIndex = data.MaterialIndex;
  
    gl_Position = ubo.ViewProjection * data.Transform * vec4(a_Position,0.0, 1.0);
}
