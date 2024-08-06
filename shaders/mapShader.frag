#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec2 fragTexCoord;
//layout(location = 1) in mat3 TBN;
//layout(location = 1) in vec3 inTangentLightDirection;

//push constants block
layout( push_constant ) uniform MapPushConstants
{
    int data[2];
	//int terrains_count;
    //int textureMasks_count;
} mapPushConstants;

layout(set = 1, binding = 1) uniform sampler2D normalMap;
layout(set = 1, binding = 2) uniform sampler2D textureMasks[];
layout(set = 1, binding = 3) uniform sampler2D terrainTextures[];
layout(set = 1, binding = 4) uniform sampler2D terrainNormals[];
layout(set = 1, binding = 5) buffer AlbedoScales
{
    float scale[];
}
albedoScales;
/*layout(set = 1, binding = 7) uniform MapConfigs
{
    int terrains_count;
    int textureMasks_count;
}
mapConfigs;*/

layout(location = 0) out vec4 outColor;

vec3 lightDirection = (vec3(1, 0, 1));
//vec4 textureScales = vec4(3.0, 3.0, 4.0, 3.0);
//vec4 normalScales = vec4(3.0, 19.5, 12.25, 3.0);
//vec3 lightDirection = normalize(vec3(1, 0, 3));
void main()
{
    vec3 normalColor;
    normalColor.xy = texture(normalMap, fragTexCoord).ag * 2.0 - 1.0;
    normalColor.z = sqrt(1.0 - normalColor.x * normalColor.x - normalColor.y * normalColor.y);
    //stratum #0
    vec3 layerColor = texture(terrainTextures[0], fragTexCoord*albedoScales.scale[0]).rgb;
    vec3 layerNormal = texture(terrainNormals[0], fragTexCoord*4.0).rgb * 2.0 - 1.0;
    
    int textureIndex = 1;
    int splatMapIndex = 0;
    int splat_channel = 0;
    vec4 splat_alpha;

    //if(mapPushConstants.data[1] == 1)
    {
    for(int i = 0; i < mapPushConstants.data[1]; i++)
    {
        splat_alpha = texture(textureMasks[splatMapIndex], fragTexCoord);
        int splatChannelCycle = 5 - textureIndex;
        if(splatChannelCycle > 4)
            splatChannelCycle = 4;
        while(splat_channel < 4)
        {
            vec3 currentLayerColor = texture(terrainTextures[textureIndex], fragTexCoord*albedoScales.scale[textureIndex]).rgb;
            layerColor = mix(layerColor, currentLayerColor, splat_alpha[splat_channel]);
            textureIndex+=1;
            splat_channel+=1;
        }
        splat_channel=0;
        //splatMapIndex+=1;
    }
    }
    //normalColor = normalize(normalColor+layerNormal);
    float diff = max(dot(normalColor, lightDirection), 0.0);
    
    outColor = vec4(layerColor*diff, 1.0);
}
