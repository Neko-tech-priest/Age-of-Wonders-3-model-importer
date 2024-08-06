#include <stdint.h>
#include <cstdlib>
#include <vector>
#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"
#include "VulkanGlobalState.h"
#include "vk_check.h"
#include "VkImage.h"
#include "initVulkan.h"

extern uint32_t _swapchainImagesCount;
void createVkSwapchainKHR(SDL_Window* _window, VkExtent2D &_windowExtent, VkSurfaceKHR _surface, VkSwapchainKHR& _swapChain, VkImage* &_swapChainImages, VkImageView* &_swapChainImageViews)
{
    //SwapChain
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR* formats;
    VkPresentModeKHR* presentModes;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        formats = new VkSurfaceFormatKHR[formatCount];
        vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, formats);
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        presentModes = new VkPresentModeKHR[presentModeCount];
        vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &presentModeCount, presentModes);
    }
    bool formatFound = false;
    for(uint8_t i = 0; i < formatCount; i++)
    {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            formatFound = true;
            break;
        }
    }
    if(!formatFound)
    {
        printf("swapchain format not found!\n");
        exit(-1);
    }
    bool presentModeFound = false;
    for(uint8_t i = 0; i < presentModeCount; i++)
    {
        //VK_PRESENT_MODE_IMMEDIATE_KHR
		//VK_PRESENT_MODE_MAILBOX_KHR
		//VK_PRESENT_MODE_FIFO_KHR
		//VK_PRESENT_MODE_FIFO_RELAXED_KHR
        if(presentModes[i] == VK_PRESENT_MODE_FIFO_KHR)
        {
            presentModeFound = true;
            break;
        }
    }
    if(!presentModeFound)
    {
        printf("swapchain present mode not found!\n");
        exit(-1);
    }
    //chooseSwapExtent

    VkSurfaceFormatKHR surfaceFormat{VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    VkPresentModeKHR presentMode{VK_PRESENT_MODE_FIFO_KHR};
    _windowExtent = capabilities.currentExtent;
    printf("%d\n%d\n", _windowExtent.width, _windowExtent.height);
    //_windowExtent.width = pixelWidth;
    //_windowExtent.height = pixelHeight;
    //surfaceFormat.format = VK_FORMAT_B8G8R8A8_SRGB;
    //surfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

    _swapchainImagesCount = capabilities.minImageCount;

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = _surface;
    swapchainCreateInfo.minImageCount = _swapchainImagesCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = capabilities.currentExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    //за графику и представление отвечает одна и та же очередь
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount = 0; // Optional
    swapchainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
    //никаких трансформаций
    swapchainCreateInfo.preTransform = capabilities.currentTransform;
    //игнорирование альфы
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    //отсечение пикселей, которых не видно для лучшей производительности
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    VK_CHECK(vkCreateSwapchainKHR(_device, &swapchainCreateInfo, nullptr, &_swapChain));
    delete[] formats;
    delete[] presentModes;

    vkGetSwapchainImagesKHR(_device, _swapChain, &_swapchainImagesCount, nullptr);
    printf("%s%d\n", "image count: ", _swapchainImagesCount);
    _swapChainImages = new VkImage[_swapchainImagesCount];
    vkGetSwapchainImagesKHR(_device, _swapChain, &_swapchainImagesCount, _swapChainImages);

    //createImageViews()
    _swapChainImageViews = new VkImageView[_swapchainImagesCount];
    for (size_t i = 0; i < _swapchainImagesCount; i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = _swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
        //переключения цветовых каналов, значение по умолчанию
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        //к какой части изображения стоит обращаться
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        VK_CHECK(vkCreateImageView(_device, &createInfo, nullptr, &_swapChainImageViews[i]));
    }
}
void cleanupSwapChain(VkImageView& depthImageView, VkImage& depthImage, VkDeviceMemory& depthImageMemory, VkImageView* &_swapchainImageViews, VkFramebuffer* &_framebuffers, VkSwapchainKHR &_swapchain)
{
    vkDestroyImageView(_device, depthImageView, nullptr);
    vkDestroyImage(_device, depthImage, nullptr);
    vkFreeMemory(_device, depthImageMemory, nullptr);
    for (uint32_t i = 0; i < _swapchainImagesCount; i++)
    {
        vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
        vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
    }
    vkDestroySwapchainKHR(_device, _swapchain, nullptr);
}
void recreateSwapchain(SDL_Window* _window, VkExtent2D &_windowExtent, VkSurfaceKHR _surface,
                       VkImage& depthImage, VkImageView& depthImageView, VkDeviceMemory& depthDeviceMemory, VkFormat _depthFormat,
                       VkSwapchainKHR &_swapchain, VkImage* &_swapchainImages, VkImageView* &_swapchainImageViews,
                       VkFramebuffer* &_framebuffers, VkRenderPass &_renderPass)
{
    vkDeviceWaitIdle(_device);

    cleanupSwapChain(depthImageView, depthImage, depthDeviceMemory, _swapchainImageViews, _framebuffers, _swapchain);
    createVkSwapchainKHR(_window, _windowExtent, _surface, _swapchain, _swapchainImages, _swapchainImageViews);
    createDepthResources(_windowExtent.width, _windowExtent.height, _depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageView, depthDeviceMemory);
    init_framebuffers(_windowExtent, _renderPass, _swapchainImages, _swapchainImageViews, depthImageView, _framebuffers);
}
