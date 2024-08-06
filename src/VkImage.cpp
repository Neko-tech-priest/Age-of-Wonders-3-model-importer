#include <cstring>
#include "VulkanGlobalState.h"
#include "vk_check.h"
#include "VkDeviceMemory.h"
#include "VkBuffer.h"
#include "Image.h"

void createVkImage(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImage& image)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateImage(_device, &imageInfo, nullptr, &image));
}
void createVkImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView& imageView)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(_device, &viewInfo, nullptr, &imageView));
}
void createVkImage__VkImageView__VkDeviceMemory__HV_DL__Depth()
{

}
void createVkImage__VkImageView__VkDeviceMemory(VkQueue _graphicsQueue, VkCommandBuffer _commandBuffer, Image image, VkImage& vkImage, VkImageView& vkImageView, VkDeviceMemory& deviceMemory)
{
    uint32_t sizeDeviceMemory = 0;
    //uint32_t image_full_size;
    VkMemoryRequirements memRequirements;
    createVkImage(image.width, image.height, image.format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, vkImage);
    vkGetImageMemoryRequirements(_device, vkImage, &memRequirements);
    sizeDeviceMemory = (memRequirements.size + ((memRequirements.alignment - memRequirements.size % memRequirements.alignment) % memRequirements.alignment));

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingDeviceMemory;

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);
    uint32_t memoryTypeIndex;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = sizeDeviceMemory;

    memoryTypeIndex = findMemoryType(_physicalDevice, memProperties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    VK_CHECK(vkAllocateMemory(_device, &allocInfo, nullptr, &stagingDeviceMemory));
    createVkBuffer(sizeDeviceMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer);
    VK_CHECK(vkBindBufferMemory(_device, stagingBuffer, stagingDeviceMemory, 0));

    memoryTypeIndex = findMemoryType(_physicalDevice, memProperties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    VK_CHECK(vkAllocateMemory(_device, &allocInfo, nullptr, &deviceMemory));

    void* data;
    vkMapMemory(_device, stagingDeviceMemory, 0, sizeDeviceMemory, 0, &data);
    VK_CHECK(vkBindImageMemory(_device, vkImage, deviceMemory, 0));
    createVkImageView(vkImage, image.format, VK_IMAGE_ASPECT_COLOR_BIT, vkImageView);
    memcpy(data, image.data, image.size);
    vkUnmapMemory(_device, stagingDeviceMemory);

    //
    VkCommandBufferBeginInfo cmdBeginInfo{};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    //
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffer;
    //первый барьер
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //barrier.image = textureImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;


    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    VK_CHECK(vkBeginCommandBuffer(_commandBuffer, &cmdBeginInfo));

    barrier.image = vkImage;
    vkCmdPipelineBarrier(
        _commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    VK_CHECK(vkEndCommandBuffer(_commandBuffer));
    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphicsQueue);
    //копирование
    VkBufferImageCopy region{};
    //region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent.depth = 1;

    VK_CHECK(vkBeginCommandBuffer(_commandBuffer, &cmdBeginInfo));

    region.bufferOffset = 0;
    region.imageExtent.width = image.width;
    region.imageExtent.height = image.height;
    vkCmdCopyBufferToImage(_commandBuffer, stagingBuffer, vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    VK_CHECK(vkEndCommandBuffer(_commandBuffer));
    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphicsQueue);
    //второй барьер
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    VK_CHECK(vkBeginCommandBuffer(_commandBuffer, &cmdBeginInfo));

    barrier.image = vkImage;
    vkCmdPipelineBarrier(
        _commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    VK_CHECK(vkEndCommandBuffer(_commandBuffer));
    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphicsQueue);

    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingDeviceMemory, nullptr);
}
void createVkImages__VkImageViews__VkDeviceMemory_AoS(VkQueue _graphicsQueue, VkCommandBuffer _commandBuffer, Image* images, uint8_t* descriptors, uint16_t offset, uint16_t numImages, VkDeviceMemory& deviceMemory)
{
    uint32_t sizeDeviceMemory = 0;
    uint32_t* images_full_sizes = new uint32_t[numImages];
    for(size_t i = 0; i < numImages; i++)
    {
        createVkImage(images[i].width, images[i].height, images[i].format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, *(VkImage*)(descriptors+offset*i));

        VkMemoryRequirements memRequirements{};
        vkGetImageMemoryRequirements(_device, *(VkImage*)(descriptors+offset*i), &memRequirements);
        images_full_sizes[i] = (memRequirements.size + ((memRequirements.alignment - memRequirements.size % memRequirements.alignment) % memRequirements.alignment));
        sizeDeviceMemory += images_full_sizes[i];
    }
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingDeviceMemory;

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);
    uint32_t memoryTypeIndex;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = sizeDeviceMemory;

    memoryTypeIndex = findMemoryType(_physicalDevice, memProperties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    VK_CHECK(vkAllocateMemory(_device, &allocInfo, nullptr, &stagingDeviceMemory));
    createVkBuffer(sizeDeviceMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer);
    VK_CHECK(vkBindBufferMemory(_device, stagingBuffer, stagingDeviceMemory, 0));

    memoryTypeIndex = findMemoryType(_physicalDevice, memProperties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    VK_CHECK(vkAllocateMemory(_device, &allocInfo, nullptr, &deviceMemory));

    uint32_t deviceOffset = 0;
    void* data;
    vkMapMemory(_device, stagingDeviceMemory, 0, sizeDeviceMemory, 0, &data);
    for(size_t i = 0; i < numImages; i++)
    {
        VK_CHECK(vkBindImageMemory(_device, *(VkImage*)(descriptors+offset*i), deviceMemory, deviceOffset));
        createVkImageView(*(VkImage*)(descriptors+offset*i), images[i].format, VK_IMAGE_ASPECT_COLOR_BIT, *(VkImageView*)(descriptors+offset*i+8));
        memcpy((uint8_t*)data+deviceOffset, images[i].data, images[i].size);
        deviceOffset += images_full_sizes[i];
    }
    vkUnmapMemory(_device, stagingDeviceMemory);

    //
    VkCommandBufferBeginInfo cmdBeginInfo{};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    //
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffer;
    //первый барьер
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //barrier.image = textureImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;


    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    VK_CHECK(vkBeginCommandBuffer(_commandBuffer, &cmdBeginInfo));

    for(size_t i = 0; i < numImages; i++)
    {
        barrier.image = *(VkImage*)(descriptors+offset*i);
        vkCmdPipelineBarrier(
            _commandBuffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    VK_CHECK(vkEndCommandBuffer(_commandBuffer));
    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphicsQueue);
    //копирование
    VkBufferImageCopy region{};
    //region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent.depth = 1;

    VK_CHECK(vkBeginCommandBuffer(_commandBuffer, &cmdBeginInfo));

    deviceOffset = 0;
    for(size_t i = 0; i < numImages; i++)
    {
        region.bufferOffset = deviceOffset;
        region.imageExtent.width = images[i].width;
        region.imageExtent.height = images[i].height;
        vkCmdCopyBufferToImage(_commandBuffer, stagingBuffer, *(VkImage*)(descriptors+offset*i), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        deviceOffset += images_full_sizes[i];
    }

    VK_CHECK(vkEndCommandBuffer(_commandBuffer));
    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphicsQueue);
    //второй барьер
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    VK_CHECK(vkBeginCommandBuffer(_commandBuffer, &cmdBeginInfo));

    for(size_t i = 0; i < numImages; i++)
    {
        barrier.image = *(VkImage*)(descriptors+offset*i);
        vkCmdPipelineBarrier(
            _commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    VK_CHECK(vkEndCommandBuffer(_commandBuffer));
    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphicsQueue);

    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingDeviceMemory, nullptr);

    delete[] images_full_sizes;
}
void createVkImages__VkImageViews__VkDeviceMemory(VkQueue _graphicsQueue, VkCommandBuffer _commandBuffer, Image* images, VkImage* vkImages, VkImageView* vkImageViews, uint8_t numImages, VkDeviceMemory& deviceMemory)
{
    uint32_t sizeDeviceMemory = 0;
    uint16_t prefersDedicatedAllocationCount = 0;
    uint32_t* images_full_sizes = new uint32_t[numImages];
    VkMemoryRequirements2* memRequirements = new VkMemoryRequirements2[numImages]{};
    /*for(size_t i = 0; i < numImages; i++)
    {
        createVkImage(images[i].width, images[i].height, images[i].format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, vkImages[i]);

        //VkMemoryDedicatedRequirements memoryDedicatedRequirements{};
        VkImageMemoryRequirementsInfo2 imageMemoryRequirementsInfo2{};
        imageMemoryRequirementsInfo2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
        imageMemoryRequirementsInfo2.image = vkImages[i];
        vkGetImageMemoryRequirements2(_device, &imageMemoryRequirementsInfo2, &memRequirements[i]);
        images_full_sizes[i] = (memRequirements[i].memoryRequirements.size + ((memRequirements[i].memoryRequirements.alignment - memRequirements[i].memoryRequirements.size % memRequirements[i].memoryRequirements.alignment) % memRequirements[i].memoryRequirements.alignment));
        sizeDeviceMemory += images_full_sizes[i];
        //printf("%s%ld\n", "image memRequirements: ",memRequirements[i].size);
        //printf("%s%ld\n", "image size: ",images[i].size);
    }*/

    for(size_t i = 0; i < numImages; i++)
    {
        createVkImage(images[i].width, images[i].height, images[i].format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, vkImages[i]);

        VkMemoryDedicatedRequirements MemoryDedicatedRequirements{};
        MemoryDedicatedRequirements.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_REQUIREMENTS;
        VkMemoryRequirements2 MemoryRequirements2{};
        MemoryRequirements2.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
        MemoryRequirements2.pNext = &MemoryDedicatedRequirements;

        VkImageMemoryRequirementsInfo2 imageMemoryRequirementsInfo2{};
        imageMemoryRequirementsInfo2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_REQUIREMENTS_INFO_2;
        imageMemoryRequirementsInfo2.image = vkImages[i];

        vkGetImageMemoryRequirements2(_device, &imageMemoryRequirementsInfo2, &MemoryRequirements2);
        if(MemoryDedicatedRequirements.prefersDedicatedAllocation)
        {
            prefersDedicatedAllocationCount+=1;
        }

        images_full_sizes[i] = (MemoryRequirements2.memoryRequirements.size + ((MemoryRequirements2.memoryRequirements.alignment - MemoryRequirements2.memoryRequirements.size % MemoryRequirements2.memoryRequirements.alignment) % MemoryRequirements2.memoryRequirements.alignment));
        sizeDeviceMemory += images_full_sizes[i];
    }
    printf("%s%d\n", "prefersDedicatedAllocationCount: ", prefersDedicatedAllocationCount);
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingDeviceMemory;

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);
    uint32_t memoryTypeIndex;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = sizeDeviceMemory;

    memoryTypeIndex = findMemoryType(_physicalDevice, memProperties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    VK_CHECK(vkAllocateMemory(_device, &allocInfo, nullptr, &stagingDeviceMemory));
    createVkBuffer(sizeDeviceMemory, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer);
    VK_CHECK(vkBindBufferMemory(_device, stagingBuffer, stagingDeviceMemory, 0));

    memoryTypeIndex = findMemoryType(_physicalDevice, memProperties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    VK_CHECK(vkAllocateMemory(_device, &allocInfo, nullptr, &deviceMemory));

    uint32_t deviceOffset = 0;
    void* data;
    vkMapMemory(_device, stagingDeviceMemory, 0, sizeDeviceMemory, 0, &data);
    for(size_t i = 0; i < numImages; i++)
    {
        VK_CHECK(vkBindImageMemory(_device, vkImages[i], deviceMemory, deviceOffset));
        createVkImageView(vkImages[i], images[i].format, VK_IMAGE_ASPECT_COLOR_BIT, vkImageViews[i]);
        memcpy((uint8_t*)data+deviceOffset, images[i].data, images[i].size);
        deviceOffset += images_full_sizes[i];
    }
    vkUnmapMemory(_device, stagingDeviceMemory);

    //
    VkCommandBufferBeginInfo cmdBeginInfo{};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    //
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_commandBuffer;
    //первый барьер
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    //barrier.image = textureImage;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;


    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    VK_CHECK(vkBeginCommandBuffer(_commandBuffer, &cmdBeginInfo));

    for(size_t i = 0; i < numImages; i++)
    {
        barrier.image = vkImages[i];
        vkCmdPipelineBarrier(
            _commandBuffer,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    VK_CHECK(vkEndCommandBuffer(_commandBuffer));
    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphicsQueue);
    //копирование
    VkBufferImageCopy region{};
    //region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent.depth = 1;

    VK_CHECK(vkBeginCommandBuffer(_commandBuffer, &cmdBeginInfo));

    deviceOffset = 0;
    for(size_t i = 0; i < numImages; i++)
    {
        region.bufferOffset = deviceOffset;
        region.imageExtent.width = images[i].width;
        region.imageExtent.height = images[i].height;
        vkCmdCopyBufferToImage(_commandBuffer, stagingBuffer, vkImages[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        deviceOffset += images_full_sizes[i];
    }

    VK_CHECK(vkEndCommandBuffer(_commandBuffer));
    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphicsQueue);
    //второй барьер
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    VK_CHECK(vkBeginCommandBuffer(_commandBuffer, &cmdBeginInfo));

    for(size_t i = 0; i < numImages; i++)
    {
        barrier.image = vkImages[i];
        vkCmdPipelineBarrier(
            _commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    VK_CHECK(vkEndCommandBuffer(_commandBuffer));
    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphicsQueue);

    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingDeviceMemory, nullptr);
    delete[] memRequirements;
    delete[] images_full_sizes;
}
void createVkImage_and_VkDeviceMemory(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& deviceMemory)
{
    createVkImage(width, height, format, usage, image);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_device, image, &memRequirements);

    createVkDeviceMemory(memRequirements, properties, deviceMemory);

    VK_CHECK(vkBindImageMemory(_device, image, deviceMemory, 0));
}
void createDepthResources(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkImageView& imageView, VkDeviceMemory& deviceMemory)
{
    createVkImage_and_VkDeviceMemory(width, height, format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, deviceMemory);

    createVkImageView(image, format, VK_IMAGE_ASPECT_DEPTH_BIT, imageView);
}
