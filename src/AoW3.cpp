#include "VulkanGlobalState.h"
#include "vk_check.h"
#include "VkPipeline.h"
#include "AoW3.h"

extern VkFormat _depthFormat;
extern VkFormat _swapchainImageFormat;
void AoW3_Mesh::GPU_memory_unload()
{
    vkDestroyBuffer(_device, vertexVkBuffer, nullptr);
    vkFreeMemory(_device, vertexVkDeviceMemory, nullptr);
    vkDestroyBuffer(_device, indexVkBuffer, nullptr);
    vkFreeMemory(_device, indexVkDeviceMemory, nullptr);
}
void AoW3_Create_PNUT_Pipeline(VkExtent2D _windowExtent, VkRenderPass _renderPass, VkDescriptorSetLayout cameraDescriptorSetLayout, VkDescriptorSetLayout textureDescriptorSetLayout, VkPipelineLayout& pipelineLayout, VkPipeline& pipeline)
{
    VkShaderModule vertShaderModule = createShaderModule(_device, "shaders/AoW3_PNUT.vert.spv");
    VkShaderModule fragShaderModule = createShaderModule(_device, "shaders/AoW3_Diffuse.frag.spv");

    const VkPipelineShaderStageCreateInfo shaderStages[2]
    {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertShaderModule,
            .pName = "main",
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragShaderModule,
            .pName = "main",
        }
    };

    std::array<VkVertexInputBindingDescription, 1> bindingDescriptions{};
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = 48;
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = 0;

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 2;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = 24;

    VkPipelineVertexInputStateCreateInfo VertexInputState
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,

        .vertexBindingDescriptionCount = bindingDescriptions.size(),
        .vertexAttributeDescriptionCount = attributeDescriptions.size(),
        .pVertexBindingDescriptions = bindingDescriptions.data(),
        .pVertexAttributeDescriptions = attributeDescriptions.data(),
    };

    //typedef struct VkPipelineInputAssemblyStateCreateInfo
    //{
    //    VkStructureType                            sType;
    //    const void*                                pNext;
    //    VkPipelineInputAssemblyStateCreateFlags    flags;
    //    VkPrimitiveTopology                        topology;
    //    VkBool32                                   primitiveRestartEnable;
    //};

    VkPipelineInputAssemblyStateCreateInfo InputAssemblyState
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    //VkPipelineTessellationStateCreateInfo TessellationState{};

    //make viewport state from our stored viewport and scissor.
    VkPipelineViewportStateCreateInfo ViewportState{};
    ViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportState.viewportCount = 1;
    //viewportState.pViewports = &_viewport;
    ViewportState.scissorCount = 1;
    //viewportState.pScissors = &_scissor;

    //configure the rasterizer to draw filled triangles
    VkPipelineRasterizationStateCreateInfo RasterizationState
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,

        .depthClampEnable = VK_FALSE,
        //discards all primitives before the rasterization stage if enabled which we don't want
        .rasterizerDiscardEnable = VK_FALSE,

        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        //no depth bias
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
    };

    //we dont use multisampling, so just run the default one
    VkPipelineMultisampleStateCreateInfo MultisampleState
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,

        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        //multisampling defaulted to no multisampling (1 sample per pixel)
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    //DepthStencilState
    VkPipelineDepthStencilStateCreateInfo DepthStencilState
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,

        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,//VK_COMPARE_OP_GREATER
        .depthBoundsTestEnable = VK_FALSE,
        .minDepthBounds = 0.0f, // Optional
        .maxDepthBounds = 1.0f, // Optional
        .stencilTestEnable = VK_FALSE,
    };

    //setup dummy color blending. We arent using transparent objects yet
    //the blending is just "no blend", but we do write to the color attachment

    //a single blend attachment with no blending and writing to RGBA
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo ColorBlendState
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,

        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
    };

    std::array<VkDynamicState, 2> dynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo DynamicState
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data(),
    };

    VkDescriptorSetLayout descriptorSetLayouts[2];
    descriptorSetLayouts[0] = cameraDescriptorSetLayout;
    descriptorSetLayouts[1] = textureDescriptorSetLayout;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        //.flags = 0,
        .setLayoutCount = 2,
        .pSetLayouts = descriptorSetLayouts,
        //.pushConstantRangeCount = 1,
        //.pPushConstantRanges = &push_constant,
    };

    VK_CHECK(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

    const VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &_swapchainImageFormat,
        .depthAttachmentFormat = _depthFormat,
    };

    VkGraphicsPipelineCreateInfo pipelineInfo
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        //.flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT,

        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &VertexInputState,
        .pInputAssemblyState = &InputAssemblyState,
        .pViewportState = &ViewportState,
        .pRasterizationState = &RasterizationState,
        .pMultisampleState = &MultisampleState,
        .pDepthStencilState = &DepthStencilState,
        .pColorBlendState = &ColorBlendState,
        .pDynamicState = &DynamicState,
        .layout = pipelineLayout,
        //.renderPass = _renderPass,
        .subpass = 0,
        .basePipelineHandle = nullptr,

        .renderPass = nullptr,
        .pNext = &pipeline_rendering_create_info,
    };

    VK_CHECK(vkCreateGraphicsPipelines(_device, nullptr, 1, &pipelineInfo, nullptr, &pipeline));

    vkDestroyShaderModule(_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(_device, vertShaderModule, nullptr);
}
void AoW3_Create_PNUCIIIWWT_Pipeline(VkExtent2D _windowExtent, VkRenderPass _renderPass, VkDescriptorSetLayout cameraDescriptorSetLayout, VkDescriptorSetLayout textureDescriptorSetLayout, VkPipelineLayout& pipelineLayout, VkPipeline& pipeline)
{
    VkShaderModule vertShaderModule = createShaderModule(_device, "shaders/AoW3_PNUT.vert.spv");
    VkShaderModule fragShaderModule = createShaderModule(_device, "shaders/AoW3_Diffuse.frag.spv");

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
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = 60;
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = 0;

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 2;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = 24;

    VertexInputState.vertexBindingDescriptionCount = bindingDescriptions.size();
    VertexInputState.vertexAttributeDescriptionCount = attributeDescriptions.size();
    VertexInputState.pVertexBindingDescriptions = bindingDescriptions.data();
    VertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();

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
    //InputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    //InputAssemblyState.primitiveRestartEnable = VK_TRUE;

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

    std::array<VkDynamicState, 2> dynamicStates =
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
    descriptorSetLayouts[1] = textureDescriptorSetLayout;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //pipelineLayoutInfo.flags = 0;
    pipelineLayoutInfo.setLayoutCount = 2;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;
    //pipelineLayoutInfo.pushConstantRangeCount = 1;
    //pipelineLayoutInfo.pPushConstantRanges = &push_constant;

    VK_CHECK(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

    const VkPipelineRenderingCreateInfoKHR pipeline_rendering_create_info {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &_swapchainImageFormat,
    };

    /*const VkGraphicsPipelineCreateInfo pipeline_create_info {
        // ...
        .pNext = &pipeline_rendering_create_info,
        // ...
        .renderPass = nullptr, // We no longer need a render pass
        // ...
    };*/
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
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = _renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    pipelineInfo.pNext = &pipeline_rendering_create_info;
    pipelineInfo.renderPass = nullptr;
    VK_CHECK(vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));

    vkDestroyShaderModule(_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(_device, vertShaderModule, nullptr);
}
void AoW3_Create_Mountain_Pipeline(VkExtent2D _windowExtent, VkRenderPass _renderPass, VkDescriptorSetLayout cameraDescriptorSetLayout, VkPipelineLayout &mountainPipelineLayout, VkPipeline &mountainPipeline)
{
    VkShaderModule vertShaderModule = createShaderModule(_device, "shaders/AoW3_MountainShader.vert.spv");
    VkShaderModule fragShaderModule = createShaderModule(_device, "shaders/AoW3_MountainShader.frag.spv");

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
    std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions{};
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = 52;
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = 0;

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = 12;

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = 24;

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32_UINT;
    attributeDescriptions[3].offset = 32;

    attributeDescriptions[4].binding = 0;
    attributeDescriptions[4].location = 4;
    attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[4].offset = 36;

    attributeDescriptions[5].binding = 0;
    attributeDescriptions[5].location = 5;
    attributeDescriptions[5].format = VK_FORMAT_R32_SFLOAT;
    attributeDescriptions[5].offset = 48;

    VertexInputState.vertexBindingDescriptionCount = bindingDescriptions.size();
    VertexInputState.vertexAttributeDescriptionCount = attributeDescriptions.size();
    VertexInputState.pVertexBindingDescriptions = bindingDescriptions.data();
    VertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();

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

    std::array<VkDynamicState, 2> dynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo DynamicState{};
    DynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    DynamicState.pDynamicStates = dynamicStates.data();

    VkDescriptorSetLayout descriptorSetLayouts[1];
    descriptorSetLayouts[0] = cameraDescriptorSetLayout;

    //setup push constants
    /*VkPushConstantRange push_constant{};
     / /this push constant range starts at the beginning
     push_constant.offset = 0;
     //this push constant range takes up the size of a MeshPushConstants struct
     push_constant.size = 8;
     //this push constant range is accessible only in the vertex shader
     push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;*/

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //pipelineLayoutInfo.flags = 0;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;
    //pipelineLayoutInfo.pushConstantRangeCount = 1;
    //pipelineLayoutInfo.pPushConstantRanges = &push_constant;

    VK_CHECK(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &mountainPipelineLayout));

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
    pipelineInfo.layout = mountainPipelineLayout;
    pipelineInfo.renderPass = _renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    VK_CHECK(vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &mountainPipeline));

    vkDestroyShaderModule(_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(_device, vertShaderModule, nullptr);
}
