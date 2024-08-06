#include <stdint.h>
//#include <memory.h>
#include "algebra.h"
#include "volk.h"

struct TerrainMaterial
{
    VkImage textureVkImage;
    VkImageView textureVkImageView;

    VkDescriptorSet descriptorSet;

    void GPU_memory_unload();
    void Create_TerrainMaterial_VkDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkSampler textureSampler);
};

void Create_TerrainMaterial_VkDescriptorSetLayout(VkDescriptorSetLayout &descriptorSetLayout);
void Create_TerrainMaterial_VkDescriptorPool(VkDescriptorPool &descriptorPool, uint32_t texturesCount);

void Create_TerrainMaterial_Pipeline(VkExtent2D _windowExtent, VkRenderPass _renderPass, VkDescriptorSetLayout cameraDescriptorSetLayout, VkDescriptorSetLayout terrainDescriptorSetLayout, VkPipelineLayout &terrainPipelineLayout, VkPipeline &terrainPipeline);
