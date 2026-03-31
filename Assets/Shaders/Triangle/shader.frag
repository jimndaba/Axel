
#version 450

layout(location = 0) in vec4 v_Color;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 o_Color;

layout(binding = 1) uniform sampler2D texSampler;

void main() 
{ 
    o_Color = texture(texSampler, fragTexCoord);
}