#include <stdint.h>
#include "algebra.h"
#include "volk.h"

struct AoW3_Mesh
{
    uint8_t* verticesBuffer = 0;
    uint8_t* indicesBuffer = 0;
    uint32_t verticesBufferSize = 0;
    uint32_t indicesBufferSize = 0;

    VkBuffer vertexVkBuffer = 0;
    VkDeviceMemory vertexVkDeviceMemory = 0;
    VkBuffer indexVkBuffer = 0;
    VkDeviceMemory indexVkDeviceMemory = 0;

    ~AoW3_Mesh()
    {
        delete[] verticesBuffer;
        delete[] indicesBuffer;
    }
    void GPU_memory_unload();
};
void AoW3_Create_PNUT_Pipeline(VkExtent2D _windowExtent, VkRenderPass _renderPass, VkDescriptorSetLayout cameraDescriptorSetLayout, VkDescriptorSetLayout textureDescriptorSetLayout, VkPipelineLayout& pipelineLayout, VkPipeline& pipeline);
void AoW3_Create_PNUCIIIWWT_Pipeline(VkExtent2D _windowExtent, VkRenderPass _renderPass, VkDescriptorSetLayout cameraDescriptorSetLayout, VkDescriptorSetLayout textureDescriptorSetLayout, VkPipelineLayout& pipelineLayout, VkPipeline& pipeline);
void AoW3_Create_Mountain_Pipeline(VkExtent2D _windowExtent, VkRenderPass _renderPass, VkDescriptorSetLayout cameraDescriptorSetLayout, VkPipelineLayout &mountainPipelineLayout, VkPipeline &mountainPipeline);
