#ifndef PIPELINE_H_INCLUDED
#define PIPELINE_H_INCLUDED

VkShaderModule createShaderModule(VkDevice _device, const char * name);
VkShaderModuleCreateInfo ShaderModuleCreateInfo(const char * name);

void init_pipeline(VkDevice _device, VkExtent2D _windowExtent, VkRenderPass _renderPass, VkDescriptorSetLayout _descriptorSetLayout, VkPipelineLayout &_pipelineLayout, VkPipeline &_trianglePipeline);
#endif // PIPELINE_H_INCLUDED
