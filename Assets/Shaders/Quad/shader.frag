#version 450

struct MaterialData {
vec4 u_Albedo;
vec4 u_Emission;
};

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Color;

layout(std430, set = 1, binding = 0) readonly buffer MaterialTable {
    MaterialData material[];
} u_MaterialTable;

layout(push_constant) uniform Push {
    uint u_MaterialIndex;
} pc;

layout(binding = 2) uniform sampler2D texSampler;

void main() 
{ 
    MaterialData data = u_MaterialTable.material[0];

    // Multiply the texture color by the instance color (tinting/alpha)
    vec4 texColor = texture(texSampler, v_TexCoord);
    vec4 color = data.u_Emission;

    // Discard transparent pixels to prevent depth-testing issues 
    // (Common in 2D/3D sprite rendering)
    if(texColor.a < 0.1)
        discard;

    o_Color = color * texColor ;
}