#ifndef VkImage_H
#define VkImage_H
#include "volk.h"
#include "Image.h"

void createVkImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImage& image);

void createVkImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView);

void createVkImage__VkImageView__VkDeviceMemory(VkQueue _graphicsQueue, VkCommandBuffer _commandBuffer, Image image, VkImage& vkImage, VkImageView& vkImageView, VkDeviceMemory& deviceMemory);
void createVkImages__VkImageViews__VkDeviceMemory_AoS(VkQueue _graphicsQueue, VkCommandBuffer _commandBuffer, Image* images, uint8_t* descriptors, uint16_t offset, uint16_t numImages, VkDeviceMemory& deviceMemory);
void createVkImages__VkImageViews__VkDeviceMemory(VkQueue _graphicsQueue, VkCommandBuffer _commandBuffer, Image* images, VkImage* vkImages, VkImageView* vkImageViews, uint8_t numImages, VkDeviceMemory& deviceMemory);

void createVkImage_and_VkDeviceMemory(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& deviceMemory);

void createDepthResources(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkImageView& imageView, VkDeviceMemory& deviceMemory);
#endif
