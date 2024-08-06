#version 450

layout(location = 0) in vec2 fragTexCoord;
//layout(location = 1) in mat3 TBN;
//layout(location = 1) in vec3 inTangentLightDirection;

layout(set = 1, binding = 5) uniform sampler2D albedoTexture;

layout(location = 0) out vec4 outColor;

void main()
{
    vec3 Color = texture(albedoTexture, fragTexCoord).rgb;
    outColor = vec4(Color, 1.0);
}
