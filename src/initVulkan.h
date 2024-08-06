#ifndef INITVULKAN
#define INITVULKAN

//#include <SDL2/SDL.h>
//#include <SDL2/SDL_vulkan.h>
#include "volk.h"

#include <vector>

void init_default_renderpass(VkFormat _swapchainImageFormat, VkFormat _depthFormat, VkRenderPass &_renderPass);

void init_framebuffers(VkExtent2D _windowExtent, VkRenderPass _renderPass, VkImage* _swapchainImages, VkImageView* _swapchainImageViews, VkImageView _depthImageView, VkFramebuffer*& _framebuffers);

void init_commands(uint32_t &_graphicsQueueFamily, VkCommandPool &_commandPool, VkCommandBuffer* _commandBuffers);

void init_sync_structures(VkFence* _renderFence, VkSemaphore* _presentSemaphore, VkSemaphore* _renderSemaphore);

#endif
