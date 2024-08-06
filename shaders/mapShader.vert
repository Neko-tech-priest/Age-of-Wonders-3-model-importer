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
//layout(binding = 3) uniform sampler2D normalMap;

layout(location = 0) out vec2 fragTexCoord;
//layout(location = 1) out mat3 TBN;

//vec3 lightDirection = vec3(0, 0, 1);
void main()
{
    /*vec3 T, B, N;
    N.xy = texture(normalMap, fragTexCoord).ag * 2.0 - 1.0;
    N.z = sqrt(1.0 - N.x * N.x - N.y * N.y);
    
    B = normalize(cross(N, vec3(1, 0, 0)));
    T = cross(B, N);
    TBN = transpose(mat3(T, B, N));*/
    
    //gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * ubo.view * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
}
