#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(set = 1, binding = 1) uniform sampler2D albedoTexture;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 Color = texture(albedoTexture, fragTexCoord).rgb;
    outColor = vec4(Color, 1.0);
}
