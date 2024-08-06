#version 450

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 Color = fragColor;
    outColor = vec4(Color, 1.0);
}
