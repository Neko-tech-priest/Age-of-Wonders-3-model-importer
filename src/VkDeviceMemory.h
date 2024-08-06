#ifndef VkDeviceMemory_H
#define VkDeviceMemory_H

uint32_t findMemoryType(VkPhysicalDevice _physicalDevice, VkPhysicalDeviceMemoryProperties memProperties, VkMemoryPropertyFlags properties);

void createVkDeviceMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties, VkDeviceMemory& deviceMemory);

void createVkDeviceMemory(VkImage* images, uint8_t numImages, VkMemoryPropertyFlags properties, VkDeviceMemory& deviceMemory);
#endif
