#include "vk_check.h"
//#include "vertex.h"
//#include "SupCom_model.h"
VkShaderModule createShaderModule(VkDevice _device, const char * name)
{
    FILE *shader_file = fopen(name, "rb");
    if(shader_file == NULL)
    {
        printf("Не удалось открыть spv файл\n");
        exit(-1);
    }
    fseek(shader_file, 0, 2);
    uint32_t shader_size = ftell(shader_file);
    fseek(shader_file, 0, 0);
    uint8_t *shader_buffer = new uint8_t[shader_size + ((4 - shader_size % 4) % 4)];//выравнивание
    fread(shader_buffer, 1, shader_size, shader_file);
    fclose(shader_file);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shader_size;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shader_buffer);

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        printf("failed to create shader module!\n");
        exit(-1);
    }

    delete[] shader_buffer;
    return shaderModule;
}
VkShaderModuleCreateInfo ShaderModuleCreateInfo(const char * name)
{
     FILE *shader_file = fopen(name, "rb");
    if(shader_file == NULL)
    {
        printf("Не удалось открыть spv файл\n");
        exit(-1);
    }
    fseek(shader_file, 0, 2);
    uint32_t shader_size = ftell(shader_file);
    fseek(shader_file, 0, 0);
    uint8_t *shader_buffer = new uint8_t[shader_size + ((4 - shader_size % 4) % 4)];//выравнивание
    fread(shader_buffer, 1, shader_size, shader_file);
    fclose(shader_file);

    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shader_size;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(shader_buffer);

    //delete[] shader_buffer;
    return createInfo;
}
/*void init_pipeline(VkDevice _device, VkExtent2D _windowExtent, VkRenderPass _renderPass, VkDescriptorSetLayout _descriptorSetLayout, VkPipelineLayout &_pipelineLayout, VkPipeline &_trianglePipeline)
{
    VkShaderModule vertShaderModule = createShaderModule(_device, "shaders/shader.vert.spv");
    VkShaderModule fragShaderModule = createShaderModule(_device, "shaders/shader.frag.spv");

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo VertexInputState{};
    VertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    //vertex_input_info.vertexBindingDescriptionCount = 0;
    //vertex_input_info.vertexAttributeDescriptionCount = 0;

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VertexInputState.vertexBindingDescriptionCount = 1;
    VertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    VertexInputState.pVertexBindingDescriptions = &bindingDescription;
    VertexInputState.pVertexAttributeDescriptions = attributeDescriptions.data();


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
    DepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;//VK_COMPARE_OP_LESS
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

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	//pipelineLayoutInfo.flags = 0;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;
	//pipelineLayoutInfo.pushConstantRangeCount = 0;
	//pipelineLayoutInfo.pPushConstantRanges = nullptr;

	VK_CHECK(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout));

	//we now use all of the info structs we have been writing into into this one to create the pipeline
    /*
    typedef struct VkGraphicsPipelineCreateInfo
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
	/*VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    //pipelineInfo.flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;

	pipelineInfo.stageCount = std::size(shaderStages);
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &VertexInputState;
	pipelineInfo.pInputAssemblyState = &InputAssemblyState;
	pipelineInfo.pViewportState = &ViewportState;
	pipelineInfo.pRasterizationState = &RasterizationState;
	pipelineInfo.pMultisampleState = &MultisampleState;
    pipelineInfo.pDepthStencilState = &DepthStencilState;
	pipelineInfo.pColorBlendState = &ColorBlendState;
    pipelineInfo.pDynamicState = &DynamicState;
	pipelineInfo.layout = _pipelineLayout;
	pipelineInfo.renderPass = _renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	VK_CHECK(vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_trianglePipeline));

	vkDestroyShaderModule(_device, fragShaderModule, nullptr);
    vkDestroyShaderModule(_device, vertShaderModule, nullptr);
}*/
