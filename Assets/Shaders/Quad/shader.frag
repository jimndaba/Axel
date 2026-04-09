#version 450

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Color;

layout(binding = 2) uniform sampler2D texSampler;

void main() 
{ 
    // Multiply the texture color by the instance color (tinting/alpha)
    vec4 texColor = texture(texSampler, v_TexCoord);
    
    // Discard transparent pixels to prevent depth-testing issues 
    // (Common in 2D/3D sprite rendering)
    if(texColor.a < 0.1)
        discard;

    o_Color = texColor * v_Color;
}