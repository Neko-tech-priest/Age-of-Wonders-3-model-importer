#ifndef DESCRIPTOR
#define DESCRIPTOR

#include "volk.h"
void createCameraVkDescriptorSetLayout(VkDevice _device, VkDescriptorSetLayout& _descriptorSetLayout);
void createCameraVkDescriptorPool(VkDevice _device, VkDescriptorPool& _descriptorPool);
//void createDescriptorSets(VkDevice _device, VkBuffer* uniformBuffers, VkImageView textureImageView, VkSampler textureSampler, VkDescriptorSetLayout _descriptorSetLayout, VkDescriptorPool _descriptorPool, VkDescriptorSet* _descriptorSets);
void createCameraVkDescriptorSets(VkDevice _device, VkBuffer* uniformBuffers, VkDescriptorSetLayout _descriptorSetLayout, VkDescriptorPool _descriptorPool, VkDescriptorSet* _descriptorSets);

void allocateDescriptors(VkCommandBuffer _cmd, VkDescriptorSetLayout _descriptorSetLayout);

//void init_descriptor(VkDevice _device, VkCommandBuffer _cmd, VkDescriptorSetLayout& _descriptorSetLayout, VkPipelineLayout _pipelineLayout, VkBuffer uniformBuffer, VkBuffer& resourceBuffer, VkDeviceMemory& resourceBufferMemory);

#endif
