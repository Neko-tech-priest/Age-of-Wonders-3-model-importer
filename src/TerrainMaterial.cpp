#include <stdio.h>
#include <vector>
#include "VulkanGlobalState.h"
#include "vk_check.h"
#include "VkPipeline.h"
#include "TerrainMaterial.h"

struct Vertex
{
    vec3 position;
    vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};

void TerrainMaterial::GPU_memory_unload()
{
    vkDestroyImageView(_device, textureVkImageView, nullptr);
    vkDestroyImage(_device, textureVkImage, nullptr);
}
void TerrainMaterial::Create_TerrainMaterial_VkDescriptorSet(VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkSampler textureSampler)
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    VK_CHECK(vkAllocateDescriptorSets(_device, &allocInfo, &descriptorSet));

    VkDescriptorImageInfo textureInfo{};
    textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    textureInfo.imageView = textureVkImageView;
    textureInfo.sampler = textureSampler;

    std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = descriptorSet;
    descriptorWrites[0].dstBinding = 1;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pImageInfo = &textureInfo;

    vkUpdateDescriptorSets(_device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
}
void Create_TerrainMaterial_VkDescriptorSetLayout(VkDescriptorSetLayout &descriptorSetLayout)
{
    std::array<VkDescriptorSetLayoutBinding, 1> descriptorSetLayoutBindings{};

    descriptorSetLayoutBindings[0].binding = 1;
    descriptorSetLayoutBindings[0].descriptorCount = 1;
    descriptorSetLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorSetLayoutBindings[0].pImmutableSamplers = nullptr;
    descriptorSetLayoutBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    /*descriptorSetLayoutBindings[1].binding = 2;
    descriptorSetLayoutBindings[1].descriptorCount = 1;
    descriptorSetLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorSetLayoutBindings[1].pImmutableSamplers = nullptr;
    descriptorSetLayoutBindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;*/

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = descriptorSetLayoutBindings.size();
    layoutInfo.pBindings = descriptorSetLayoutBindings.data();
    //layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;

    VK_CHECK(vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &descriptorSetLayout));
}
void Create_TerrainMaterial_VkDescriptorPool(VkDescriptorPool &descriptorPool, uint32_t texturesCount)
{
    std::array<VkDescriptorPoolSize, 1> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[0].descriptorCount = texturesCount;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = texturesCount;

    VK_CHECK(vkCreateDescriptorPool(_device, &poolInfo, nullptr, &descriptorPool));
}
/*void Create_TerrainMaterial_VkDescriptorSets(TerrainMaterial* terrainMaterials, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, VkSampler textureSampler)
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 2;
    allocInfo.pSetLayouts = &descriptorSetLayout;

    VK_CHECK(vkAllocateDescriptorSets(_device, &allocInfo, &descriptorSet));
}*/
void Create_TerrainMaterial_Pipeline(VkExtent2D _windowExtent, VkRenderPass _renderPass, VkDescriptorSetLayout cameraDescriptorSetLayout, VkDescriptorSetLayout terrainDescriptorSetLayout, VkPipelineLayout &terrainPipelineLayout, VkPipeline &terrainPipeline)
{
    VkShaderModule vertShaderModule = createShaderModule(_device, "shaders/terrainShader.vert.spv");
    VkShaderModule fragShaderModule = createShaderModule(_device, "shaders/terrainShader.frag.spv");

    VkPipelineShaderStageCreateInfo shaderStages[2]{};

    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = vertShaderModule;
    shaderStages[0].pName = "main";
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fragShaderModule;
    shaderStages[1].pName = "main";

    VkPipelineVertexInputStateCreateInfo VertexInputState{};
    VertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    std::array<VkVertexInputBindingDescription, 1> bindingDescriptions{};
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, texCoord);

    VertexInputState.vertexBindingDescriptionCount = bindingDescriptions.size();
    VertexInputState.vertexAttributeDescriptionCount = attributeDescriptions.size();
    VertexInputState.pVertexBindingDescriptions = bindingDescriptions.data();
    VertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Provided by VK_VERSION_1_0
    //typedef struct VkPipelineInputAssemblyStateCreateInfo
    //{
    //    VkStructureType                            sType;
    //    const void*                                pNext;
    //    VkPipelineInputAssemblyStateCreateFlags    flags;
    //    VkPrimitiveTopology                        topology;
    //    VkBool32                                   primitiveRestartEnable;
    //};

    VkPipelineInputAssemblyStateCreateInfo InputAssemblyState{};
    InputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssemblyState.primitiveRestartEnable = VK_FALSE;

    //VkPipelineTessellationStateCreateInfo TessellationState{};

    //make viewport state from our stored viewport and scissor.
    VkPipelineViewportStateCreateInfo ViewportState{};
    ViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportState.viewportCount = 1;
    //viewportState.pViewports = &_viewport;
    ViewportState.scissorCount = 1;
    //viewportState.pScissors = &_scissor;

    //configure the rasterizer to draw filled triangles
    VkPipelineRasterizationStateCreateInfo RasterizationState{};
    RasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

    RasterizationState.depthClampEnable = VK_FALSE;
    //discards all primitives before the rasterization stage if enabled which we don't want
    RasterizationState.rasterizerDiscardEnable = VK_FALSE;

    RasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    RasterizationState.lineWidth = 1.0f;
    RasterizationState.cullMode = VK_CULL_MODE_NONE;
    RasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
    //no depth bias
    RasterizationState.depthBiasEnable = VK_FALSE;
    RasterizationState.depthBiasConstantFactor = 0.0f;
    RasterizationState.depthBiasClamp = 0.0f;
    RasterizationState.depthBiasSlopeFactor = 0.0f;

    //we dont use multisampling, so just run the default one
    VkPipelineMultisampleStateCreateInfo MultisampleState{};
    MultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

    MultisampleState.sampleShadingEnable = VK_FALSE;
    MultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    //multisampling defaulted to no multisampling (1 sample per pixel)
    MultisampleState.minSampleShading = 1.0f;
    MultisampleState.pSampleMask = nullptr;
    MultisampleState.alphaToCoverageEnable = VK_FALSE;
    MultisampleState.alphaToOneEnable = VK_FALSE;

    //DepthStencilState
    VkPipelineDepthStencilStateCreateInfo DepthStencilState{};
    DepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    DepthStencilState.depthTestEnable = VK_TRUE;
    DepthStencilState.depthWriteEnable = VK_TRUE;
    DepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;//VK_COMPARE_OP_GREATER
    DepthStencilState.depthBoundsTestEnable = VK_FALSE;
    DepthStencilState.minDepthBounds = 0.0f; // Optional
    DepthStencilState.maxDepthBounds = 1.0f; // Optional
    DepthStencilState.stencilTestEnable = VK_FALSE;

    //setup dummy color blending. We arent using transparent objects yet
    //the blending is just "no blend", but we do write to the color attachment

    //a single blend attachment with no blending and writing to RGBA
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo ColorBlendState{};
    ColorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

    ColorBlendState.logicOpEnable = VK_FALSE;
    ColorBlendState.logicOp = VK_LOGIC_OP_COPY;
    ColorBlendState.attachmentCount = 1;
    ColorBlendState.pAttachments = &colorBlendAttachment;

    std::vector<VkDynamicState> dynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo DynamicState{};
    DynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    DynamicState.pDynamicStates = dynamicStates.data();

    VkDescriptorSetLayout descriptorSetLayouts[2];
    descriptorSetLayouts[0] = cameraDescriptorSetLayout;
    descriptorSetLayouts[1] = terrainDescriptorSetLayout;

    //setup push constants
    /*VkPushConstantRange push_constant{};
    //this push constant range starts at the beginning
    push_constant.offset = 0;
    //this push constant range takes up the size of a MeshPushConstants struct
    push_constant.size = 8;
    //this push constant range is accessible only in the vertex shader
    push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;*/

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //pipelineLayoutInfo.flags = 0;
    pipelineLayoutInfo.setLayoutCount = 2;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;
    //pipelineLayoutInfo.pushConstantRangeCount = 1;
    //pipelineLayoutInfo.pPushConstantRanges = &push_constant;

    VK_CHECK(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &terrainPipelineLayout));

    //we now use all of the info structs we have been writing into into this one to create the pipeline
    /*
     t ypedef struct VkGraphicsPipelineCreateInfo               *
     {
     VkStructureType                                  sType;
     const void*                                      pNext;
     VkPipelineCreateFlags                            flags;
     uint32_t                                         stageCount;
     const VkPipelineShaderStageCreateInfo*           pStages;
     const VkPipelineVertexInputStateCreateInfo*      pVertexInputState;
     const VkPipelineInputAssemblyStateCreateInfo*    pInputAssemblyState;
     const VkPipelineTessellationStateCreateInfo*     pTessellationState;
     const VkPipelineViewportStateCreateInfo*         pViewportState;
     const VkPipelineRasterizationStateCreateInfo*    pRasterizationState;
     const VkPipelineMultisampleStateCreateInfo*      pMultisampleState;
     const VkPipelineDepthStencilStateCreateInfo*     pDepthStencilState;
     const VkPipelineColorBlendStateCreateInfo*       pColorBlendState;
     const VkPipelineDynamicStateCreateInfo*          pDynamicState;
     VkPipelineLayout                                 layout;
     VkRenderPass                                     renderPass;
     uint32_t                                         subpass;
     VkPipeline                                       basePipelineHandle;
     int32_t                                          basePipelineIndex;
} VkGraphicsPipelineCreateInfo;
*/
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    //pipelineInfo.flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;

    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &VertexInputState;
    pipelineInfo.pInputAssemblyState = &InputAssemblyState;
    pipelineInfo.pViewportState = &ViewportState;
    pipelineInfo.pRasterizationState = &RasterizationState;
    pipelineInfo.pMultisampleState = &MultisampleState;
    pipelineInfo.pDepthStencilState = &DepthStencilState;
    pipelineInfo.pColorBlendState = &ColorBlendState;
    pipelineInfo.pDynamicState = &DynamicState;
    pipelineInfo.layout = terrainPipelineLayout;
    pipelineInfo.renderPass = _renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VK_CHECK(vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &terrainPipeline));

    vkDestroyShaderModule(_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(_device, vertShaderModule, nullptr);
}
