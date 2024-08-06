#version 450

layout(location = 0) in uint inPackedData;

layout(set = 0, binding = 0) uniform CameraBufferObject
{
    mat4 view;
    mat4 proj;
}
ubo;

layout(location = 0) out vec2 fragTexCoord;

void main()
{
    ivec3 position_int;
    vec3 position;
    position.x = int((inPackedData>>28) & 15) - 2;
    position.y = int((inPackedData>>24) & 15) - 2;
    position.z = int((inPackedData>>20) & 15) - 2;

    //position = position_int;

    fragTexCoord.x = (inPackedData>>19) & 1;
    fragTexCoord.y = (inPackedData>>18) & 1;

    gl_Position = ubo.proj * ubo.view * vec4(position, 1.0);
}
