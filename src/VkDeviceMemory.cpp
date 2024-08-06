#include "VulkanGlobalState.h"
#include "vk_check.h"

uint32_t findMemoryType(VkPhysicalDevice _physicalDevice, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        //(typeFilter & (1 << i)) && 
        if ((memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    printf("failed to find suitable memory type!\n");
    exit(-1);
}
uint32_t findMemoryType(VkPhysicalDevice _physicalDevice, VkPhysicalDeviceMemoryProperties memProperties, VkMemoryPropertyFlags properties)
{
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        //(typeFilter & (1 << i)) && 
        if ((memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    printf("failed to find suitable memory type!\n");
    exit(-1);
}
void createVkDeviceMemory(uint32_t size, VkMemoryPropertyFlags properties, VkDeviceMemory& deviceMemory)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);
    uint32_t memoryTypeIndex;

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            memoryTypeIndex = i;
            goto skip;
        }
    }
    printf("failed to find suitable memory type!\n");
    exit(-1);
    skip:

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    VK_CHECK(vkAllocateMemory(_device, &allocInfo, nullptr, &deviceMemory));
}
void createVkDeviceMemory(VkMemoryRequirements memRequirements, VkMemoryPropertyFlags properties, VkDeviceMemory& deviceMemory)
{
    /*typedef struct VkMemoryRequirements{
    VkDeviceSize    size;
    VkDeviceSize    alignment;
    uint32_t        memoryTypeBits;
    }
    
    typedef struct VkPhysicalDeviceMemoryProperties
    {
        uint32_t        memoryTypeCount;
        VkMemoryType    memoryTypes[VK_MAX_MEMORY_TYPES];
        uint32_t        memoryHeapCount;
        VkMemoryHeap    memoryHeaps[VK_MAX_MEMORY_HEAPS];
    }*/
    
    /*typedef struct VkMemoryType
    {
        VkMemoryPropertyFlags    propertyFlags;
        uint32_t                 heapIndex;
    }*/

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);
    uint32_t memoryTypeIndex;

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((memRequirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            memoryTypeIndex = i;
            goto skip;
        }
    }
    printf("failed to find suitable memory type!\n");
    exit(-1);
    skip:
    
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = memoryTypeIndex;

    VK_CHECK(vkAllocateMemory(_device, &allocInfo, nullptr, &deviceMemory));
}
