#ifndef VkBuffer_H
#define VkBuffer_H
#include <stdint.h>
#include "VulkanGlobalState.h"
void createVkBuffer(uint32_t size, VkBufferUsageFlags usage, VkBuffer& buffer);

void createVkBuffer__VkDeviceMemory__HV_DL(VkBufferUsageFlags usage, uint32_t size, VkBuffer& vkBuffer, VkDeviceMemory& deviceMemory);

void createVkBuffer__VkDeviceMemory(VkQueue _graphicsQueue, VkCommandBuffer _commandBuffer, VkBufferUsageFlags usage, void* srcData, uint32_t size, VkBuffer& vkBuffer, VkDeviceMemory& dstDeviceMemory);
void createVkBuffers__VkDeviceMemory(VkQueue _graphicsQueue, VkCommandBuffer _commandBuffer, VkBufferUsageFlags usage, void** data, uint32_t* sizes, uint32_t numBuffers, VkBuffer* vkBuffers, VkDeviceMemory& deviceMemory);
void createVkBuffers__VkDeviceMemory_AoS(VkQueue _graphicsQueue, VkCommandBuffer _commandBuffer, VkBufferUsageFlags usage, uint8_t* srcStructArray, uint32_t structSize, uint32_t sizeOffset, uint32_t vkBufferOffset, uint32_t numBuffers, VkDeviceMemory& dstDeviceMemory);
#endif
