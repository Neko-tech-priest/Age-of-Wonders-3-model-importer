#version 450

layout(location = 0) in vec3 inPosition;
//layout(location = 1) in vec3 inTangent;
//layout(location = 2) in vec3 inBitangent;
//layout(location = 3) in vec3 inNormal;
layout(location = 1) in vec2 inTexCoord;

layout(set = 0, binding = 0) uniform CameraBufferObject
{
    mat4 view;
    mat4 proj;
}
ubo;
layout(set = 1, binding = 2) buffer modelBufferObject
{
    mat4 model;
}
model;

layout(location = 0) out vec2 fragTexCoord;
void main()
{
    vec3 position = inPosition;
    gl_Position = ubo.proj * ubo.view * vec4(position, 1.0);
    fragTexCoord = inTexCoord;
}
