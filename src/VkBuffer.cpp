#include <cstring>
#include "VulkanGlobalState.h"
#include "vk_check.h"
#include "VkDeviceMemory.h"

void createVkBuffer(uint32_t size, VkBufferUsageFlags usage, VkBuffer& buffer)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(_device, &bufferInfo, nullptr, &buffer));
}
void createVkBuffer__VkDeviceMemory__HV_DL(VkBufferUsageFlags usage, uint32_t size, VkBuffer& vkBuffer, VkDeviceMemory& deviceMemory)
{
    size_t sizeDeviceMemory = 0;
    VkMemoryRequirements memRequirements;
    createVkBuffer(size, usage, vkBuffer);
    vkGetBufferMemoryRequirements(_device, vkBuffer, &memRequirements);
    sizeDeviceMemory = (memRequirements.size + ((memRequirements.alignment - memRequirements.size % memRequirements.alignment) % memRequirements.alignment));

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);
    uint32_t memoryTypeIndex;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = sizeDeviceMemory;

    memoryTypeIndex = findMemoryType(_physicalDevice, memProperties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    VK_CHECK(vkAllocateMemory(_device, &allocInfo, nullptr, &deviceMemory));
    //createVkBuffer(sizeDeviceMemory, VK_BUFFER_USAGE_TRANSFER_DST_BIT, vkBuffer);
    VK_CHECK(vkBindBufferMemory(_device, vkBuffer, deviceMemory, 0));

    /*void* data;
    vkMapMemory(_device, deviceMemory, 0, sizeDeviceMemory, 0, &data);
    memcpy(data, srcData, size);*/
}
void createVkBuffer__VkDeviceMemory(VkQueue _graphicsQueue, VkCommandBuffer _commandBuffer, VkBufferUsageFlags usage, void* srcData, uint32_t size, VkBuffer& vkBuffer, VkDeviceMemory& dstDeviceMemory)
{
    size_t sizeDeviceMemory = 0;
    VkMemoryRequirements memRequirements;
    createVkBuffer(size, usage, vkBuffer);
    vkGetBufferMemoryRequirements(_device, vkBuffer, &memRequirements);
    //printf("%s%lu\n", "memRequirements.alignment: ", memRequirements.alignment);

    sizeDeviceMemory = (memRequirements.size + ((memRequirements.alignment - memRequirements.size % memRequirements.alignment) % memRequirements.alignment));

    VkBuffer dstBuffer;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingDeviceMemory;

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);
    uint32_t memoryTypeIndex;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = sizeDeviceMemory;

    memoryTypeIndex = findMemoryType(_physicalDevice, memProperties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    VK_CHECK(vkAllocateMemory(_device, &allocInfo, nullptr, &stagingDeviceMemory));
    createVkBuffer(sizeDeviceMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer);
    VK_CHECK(vkBindBufferMemory(_device, stagingBuffer, stagingDeviceMemory, 0));

    memoryTypeIndex = findMemoryType(_physicalDevice, memProperties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    VK_CHECK(vkAllocateMemory(_device, &allocInfo, nullptr, &dstDeviceMemory));
    createVkBuffer(sizeDeviceMemory, VK_BUFFER_USAGE_TRANSFER_DST_BIT, dstBuffer);
    VK_CHECK(vkBindBufferMemory(_device, dstBuffer, dstDeviceMemory, 0));

    void* data;
    vkMapMemory(_device, stagingDeviceMemory, 0, sizeDeviceMemory, 0, &data);
    VK_CHECK(vkBindBufferMemory(_device, vkBuffer, dstDeviceMemory, 0));
    memcpy(data, srcData, size);
    vkUnmapMemory(_device, stagingDeviceMemory);

    // копирование из промежуточного в конечный
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(_commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    //copyRegion.srcOffset = 0; // Optional
    //copyRegion.dstOffset = 0; // Optional
    copyRegion.size = sizeDeviceMemory;
    vkCmdCopyBuffer(_commandBuffer, stagingBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(_commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffer;

    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphicsQueue);

    // очистка временных ресурсов
    vkDestroyBuffer(_device, dstBuffer, nullptr);
    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingDeviceMemory, nullptr);
}
void createVkBuffers__VkDeviceMemory(VkQueue _graphicsQueue, VkCommandBuffer _commandBuffer, VkBufferUsageFlags usage, void** srcData, uint32_t* sizes, uint32_t numBuffers, VkBuffer* vkBuffers, VkDeviceMemory& dstDeviceMemory)
{
    size_t sizeDeviceMemory = 0;
    uint32_t* buffers_full_sizes = new uint32_t[numBuffers];
    for(size_t i = 0; i < numBuffers; i++)
    {
        createVkBuffer(sizes[i], usage, vkBuffers[i]);

        VkMemoryRequirements memRequirements{};
        vkGetBufferMemoryRequirements(_device, vkBuffers[i], &memRequirements);

        buffers_full_sizes[i] = (memRequirements.size + ((memRequirements.alignment - memRequirements.size % memRequirements.alignment) % memRequirements.alignment));
        sizeDeviceMemory += buffers_full_sizes[i];
        //sizeDeviceMemory += memRequirements[i].size;
    }
    VkBuffer dstBuffer;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingDeviceMemory;

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);
    uint32_t memoryTypeIndex;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = sizeDeviceMemory;

    memoryTypeIndex = findMemoryType(_physicalDevice, memProperties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    VK_CHECK(vkAllocateMemory(_device, &allocInfo, nullptr, &stagingDeviceMemory));
    createVkBuffer(sizeDeviceMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer);
    VK_CHECK(vkBindBufferMemory(_device, stagingBuffer, stagingDeviceMemory, 0));

    memoryTypeIndex = findMemoryType(_physicalDevice, memProperties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    VK_CHECK(vkAllocateMemory(_device, &allocInfo, nullptr, &dstDeviceMemory));
    createVkBuffer(sizeDeviceMemory, VK_BUFFER_USAGE_TRANSFER_DST_BIT, dstBuffer);
    VK_CHECK(vkBindBufferMemory(_device, dstBuffer, dstDeviceMemory, 0));

    // биндинг буферов в конечную и копирование данных в промежуточную память
    size_t deviceOffset = 0;
    void* data;
    vkMapMemory(_device, stagingDeviceMemory, 0, sizeDeviceMemory, 0, &data);
    for(size_t i = 0; i < numBuffers; i++)
    {
        VK_CHECK(vkBindBufferMemory(_device, vkBuffers[i], dstDeviceMemory, deviceOffset));
        //createVkImageView(vkImages[i], images[i].format, VK_IMAGE_ASPECT_COLOR_BIT, vkImageViews[i]);
        memcpy((uint8_t*)data+deviceOffset, srcData[i], sizes[i]);
        deviceOffset += buffers_full_sizes[i];
    }
    vkUnmapMemory(_device, stagingDeviceMemory);

    // копирование из промежуточного в конечный
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(_commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    //copyRegion.srcOffset = 0; // Optional
    //copyRegion.dstOffset = 0; // Optional
    copyRegion.size = sizeDeviceMemory;
    vkCmdCopyBuffer(_commandBuffer, stagingBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(_commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffer;

    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphicsQueue);

    // очистка временных ресурсов
    vkDestroyBuffer(_device, dstBuffer, nullptr);
    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingDeviceMemory, nullptr);

    delete[] buffers_full_sizes;
}
void createVkBuffers__VkDeviceMemory_AoS(VkQueue _graphicsQueue, VkCommandBuffer _commandBuffer, VkBufferUsageFlags usage, uint8_t* srcStructArray, uint32_t structSize, uint32_t sizeOffset, uint32_t vkBufferOffset, uint32_t numBuffers, VkDeviceMemory& dstDeviceMemory)
{
    size_t sizeDeviceMemory = 0;
    uint32_t* buffers_full_sizes = new uint32_t[numBuffers];
    for(size_t i = 0; i < numBuffers; i++)
    {
        createVkBuffer(*(uint32_t*)(srcStructArray+structSize*i+sizeOffset), usage, *(VkBuffer*)(srcStructArray+structSize*i+vkBufferOffset));
        VkMemoryRequirements memRequirements{};
        vkGetBufferMemoryRequirements(_device, *(VkBuffer*)(srcStructArray+structSize*i+vkBufferOffset), &memRequirements);

        buffers_full_sizes[i] = (memRequirements.size + ((memRequirements.alignment - memRequirements.size % memRequirements.alignment) % memRequirements.alignment));
        sizeDeviceMemory += buffers_full_sizes[i];
        //printf("%d\n", (buffers_full_sizes[i]));
    }
    VkBuffer dstBuffer;
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingDeviceMemory;

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);
    uint32_t memoryTypeIndex;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = sizeDeviceMemory;

    memoryTypeIndex = findMemoryType(_physicalDevice, memProperties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    VK_CHECK(vkAllocateMemory(_device, &allocInfo, nullptr, &stagingDeviceMemory));
    createVkBuffer(sizeDeviceMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer);
    VK_CHECK(vkBindBufferMemory(_device, stagingBuffer, stagingDeviceMemory, 0));

    memoryTypeIndex = findMemoryType(_physicalDevice, memProperties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    VK_CHECK(vkAllocateMemory(_device, &allocInfo, nullptr, &dstDeviceMemory));
    createVkBuffer(sizeDeviceMemory, VK_BUFFER_USAGE_TRANSFER_DST_BIT, dstBuffer);
    VK_CHECK(vkBindBufferMemory(_device, dstBuffer, dstDeviceMemory, 0));

    // биндинг буферов в конечную и копирование данных в промежуточную память
    size_t deviceOffset = 0;
    void* data;
    vkMapMemory(_device, stagingDeviceMemory, 0, sizeDeviceMemory, 0, &data);
    for(size_t i = 0; i < numBuffers; i++)
    {
        struct PtrStruct
        {
            uint8_t* data;
        };
        VK_CHECK(vkBindBufferMemory(_device, *(VkBuffer*)(srcStructArray+structSize*i+vkBufferOffset), dstDeviceMemory, deviceOffset));
        memcpy((uint8_t*)data+deviceOffset, ((PtrStruct*)(srcStructArray+structSize*i))->data, *(uint32_t*)(srcStructArray+structSize*i+sizeOffset));
        deviceOffset += buffers_full_sizes[i];
    }
    vkUnmapMemory(_device, stagingDeviceMemory);

    // копирование из промежуточного в конечный
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(_commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    //copyRegion.srcOffset = 0; // Optional
    //copyRegion.dstOffset = 0; // Optional
    copyRegion.size = sizeDeviceMemory;
    vkCmdCopyBuffer(_commandBuffer, stagingBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(_commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffer;

    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphicsQueue);

    // очистка временных ресурсов
    vkDestroyBuffer(_device, dstBuffer, nullptr);
    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingDeviceMemory, nullptr);

    delete[] buffers_full_sizes;
    //exit(-1);
}
