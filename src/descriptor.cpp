#include <cstring>
#include <cstdlib>
#include <array>
#include "VulkanGlobalState.h"
#include "vk_check.h"
#include "frames.h"
#include "cameraBufferObject.h"
void createDescriptorSetLayout(VkDevice _device, VkDescriptorSetLayout& _descriptorSetLayout)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.pImmutableSamplers = nullptr;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 2;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS)
    {
            printf("failed to create descriptor set layout!\n");
            exit(-1);
    }
}
void createCameraVkDescriptorPool(VkDevice _device, VkDescriptorPool& _descriptorPool)
{
    std::array<VkDescriptorPoolSize, 1> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(FRAME_OVERLAP);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(FRAME_OVERLAP);

    VK_CHECK(vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool));
}
void createCameraVkDescriptorSets(VkDevice _device, VkBuffer* uniformBuffers, VkDescriptorSetLayout _descriptorSetLayout, VkDescriptorPool _descriptorPool, VkDescriptorSet* _descriptorSets)
{
    std::array<VkDescriptorSetLayout, FRAME_OVERLAP> layouts{_descriptorSetLayout};
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(FRAME_OVERLAP);
    allocInfo.pSetLayouts = layouts.data();

    VK_CHECK(vkAllocateDescriptorSets(_device, &allocInfo, _descriptorSets));

    for (size_t i = 0; i < FRAME_OVERLAP; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(CameraBufferObject);

        std::array<VkWriteDescriptorSet, 1> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = _descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
    
    
    
    /*VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &_descriptorSetLayout;

    _descriptorSets.resize(1);
    if (vkAllocateDescriptorSets(_device, &allocInfo, _descriptorSets.data()) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate descriptor sets!");
    
    for (size_t i = 0; i < 1; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(float)*16*3;

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = _descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(_device, 1, &descriptorWrite, 0, nullptr);
    }*/
}

void createCameraVkDescriptorSetLayout(VkDevice _device, VkDescriptorSetLayout& _descriptorSetLayout)
{
    VkDescriptorSetLayoutBinding cameraLayoutBinding{};
    cameraLayoutBinding.binding = 0;
    cameraLayoutBinding.descriptorCount = 1;
    cameraLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    cameraLayoutBinding.pImmutableSamplers = nullptr;
    cameraLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    std::array<VkDescriptorSetLayoutBinding, 1> bindings = {cameraLayoutBinding};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &_descriptorSetLayout) != VK_SUCCESS)
    {
        printf("failed to create descriptor set layout!\n");
        exit(-1);
    }
}
void allocateDescriptors(VkCommandBuffer _cmd, VkDescriptorSetLayout _descriptorSetLayout, VkImageView vkImageView, VkSampler vkSampler)
{
    VkDeviceSize descriptorLayoutSize{};
    vkGetDescriptorSetLayoutSizeEXT(_device, _descriptorSetLayout, &descriptorLayoutSize);
    printf("%s%ld\n", "descriptorLayoutSize: ", descriptorLayoutSize);
    VkDeviceSize layoutOffset{};
    vkGetDescriptorSetLayoutBindingOffsetEXT(_device, _descriptorSetLayout, 0, &layoutOffset);
    printf("%s%ld\n", "layoutOffset: ", layoutOffset);

    //input data
    VkDescriptorImageInfo descriptorImageInfo{};
    descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    descriptorImageInfo.imageView = vkImageView;
    descriptorImageInfo.sampler = vkSampler;

    VkDescriptorGetInfoEXT descriptorGetInfo{};
    descriptorGetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
    descriptorGetInfo.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    descriptorGetInfo.data.pCombinedImageSampler = &descriptorImageInfo;

    //output data

    //vkGetDescriptorEXT(_device, &descriptorGetInfo, 0, data);
}
/*void init_descriptor(VkDevice _device, VkCommandBuffer _cmd, VkDescriptorSetLayout& _descriptorSetLayout, VkPipelineLayout _pipelineLayout, VkBuffer uniformBuffer, VkBuffer& resourceBuffer, VkDeviceMemory& resourceBufferMemory)
{
    //получение некоторой информации перед созданием буфера
    VkDeviceSize descriptorLayoutSize{};
    vkGetDescriptorSetLayoutSizeEXT(_device, _descriptorSetLayout, &descriptorLayoutSize);
    printf("%s%ld\n", "descriptorLayoutSize: ", descriptorLayoutSize);
    VkDeviceSize layoutOffset{};
    vkGetDescriptorSetLayoutBindingOffsetEXT(_device, _descriptorSetLayout, 0, &layoutOffset);
    printf("%s%ld\n", "layoutOffset: ", layoutOffset);
    VkDeviceAddress deviceAdress{};
    
    //typedef struct VkBufferDeviceAddressInfo{
    //VkStructureType    sType;
    //const void*        pNext;
    //VkBuffer           buffer;
    //};
    VkBufferDeviceAddressInfo bufferDeviceAddressInfo{};
    bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    bufferDeviceAddressInfo.buffer = uniformBuffer;
    
    deviceAdress = vkGetBufferDeviceAddress(_device, &bufferDeviceAddressInfo);
    printf("%s%ld\n", "deviceAdress: ", deviceAdress);
    
    //typedef struct VkDescriptorAddressInfoEXT{
    //VkStructureType    sType;
    //void*              pNext;
    //VkDeviceAddress    address;
    //VkDeviceSize       range;
    //VkFormat           format;
    //};
    VkDescriptorAddressInfoEXT descriptorAddressInfo{};
    descriptorAddressInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT;
    descriptorAddressInfo.address = deviceAdress;
    descriptorAddressInfo.range = 4*16*3;
    //typedef union VkDescriptorDataEXT{
    //const VkSampler*                     pSampler;
    //const VkDescriptorImageInfo*         pCombinedImageSampler;
    //const VkDescriptorImageInfo*         pInputAttachmentImage;
    //const VkDescriptorImageInfo*         pSampledImage;
    //const VkDescriptorImageInfo*         pStorageImage;
    //const VkDescriptorAddressInfoEXT*    pUniformTexelBuffer;
    //const VkDescriptorAddressInfoEXT*    pStorageTexelBuffer;
    //const VkDescriptorAddressInfoEXT*    pUniformBuffer;
    //const VkDescriptorAddressInfoEXT*    pStorageBuffer;
    //VkDeviceAddress                      accelerationStructure;
    //}
    //typedef struct VkDescriptorGetInfoEXT {
    //VkStructureType        sType;
    //const void*            pNext;
    //VkDescriptorType       type;
    //VkDescriptorDataEXT    data;
    //}
    VkDescriptorGetInfoEXT descriptorGetInfo{};
    descriptorGetInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
    descriptorGetInfo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorGetInfo.data.pUniformBuffer = &descriptorAddressInfo;
    
    void* data;
    vkMapMemory(_device, resourceBufferMemory, 0, descriptorLayoutSize, 0, &data);
    //void vkGetDescriptorEXT(
    //VkDevice                                    device,
    //const VkDescriptorGetInfoEXT*               pDescriptorInfo,
    //size_t                                      dataSize,
    //void*                                       pDescriptor);
    
    //получение(наконец-то) данных дескриптора в буфер для дескриптора
    vkGetDescriptorEXT(_device, &descriptorGetInfo, descriptorLayoutSize, data);
    
    VkCommandBufferBeginInfo cmdBeginInfo{};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(_cmd, &cmdBeginInfo));
    
    //typedef struct VkDescriptorBufferBindingInfoEXT{
    //VkStructureType                             sType;
    //const void*                                 pNext;
    //VkDeviceAddress                             address;
    //VkBufferUsageFlags                          usage;
    //}
    bufferDeviceAddressInfo.buffer = resourceBuffer;
    deviceAdress = vkGetBufferDeviceAddress(_device, &bufferDeviceAddressInfo);
    VkDescriptorBufferBindingInfoEXT bindingInfo{};
    bindingInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
    bindingInfo.address = deviceAdress;
    bindingInfo.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    //void vkCmdBindDescriptorBuffersEXT(
    //VkCommandBuffer                             commandBuffer,
    //uint32_t                                    bufferCount,
    //const VkDescriptorBufferBindingInfoEXT*     pBindingInfos
    //);
    vkCmdBindDescriptorBuffersEXT(_cmd, 1, &bindingInfo);
    //void vkCmdSetDescriptorBufferOffsetsEXT(
    //VkCommandBuffer                             commandBuffer,
    //VkPipelineBindPoint                         pipelineBindPoint,
    //VkPipelineLayout                            layout,
    //uint32_t                                    firstSet,
    //uint32_t                                    setCount,
    //const uint32_t*                             pBufferIndices,
    //const VkDeviceSize*                         pOffsets);
    
    const uint32_t bufferIndices[] = {0};
    const VkDeviceSize offsets[] = {0};
    vkCmdSetDescriptorBufferOffsetsEXT(_cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, bufferIndices, offsets);
    
    VK_CHECK(vkEndCommandBuffer(_cmd));
    vkDeviceWaitIdle(_device);
}*/
