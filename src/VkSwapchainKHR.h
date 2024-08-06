#ifndef VkSwapshainHKR_H
#define VkSwapshainHKR_H
#include "volk.h"
void createVkSwapchainKHR(SDL_Window* _window, VkExtent2D &_windowExtent, VkSurfaceKHR _surface, VkSwapchainKHR& _swapChain, VkImage* &_swapChainImages, VkImageView* &_swapChainImageViews);

void cleanupSwapChain(VkImageView& depthImageView, VkImage& depthImage, VkDeviceMemory& depthImageMemory, VkImageView* &_swapchainImageViews, VkFramebuffer* &_framebuffers, VkSwapchainKHR &_swapchain);

void recreateSwapchain(SDL_Window* _window, VkExtent2D &_windowExtent, VkSurfaceKHR _surface,
                       VkImage& depthImage, VkImageView& depthImageView, VkDeviceMemory& depthDeviceMemory, VkFormat _depthFormat,
                       VkSwapchainKHR &_swapchain, VkImage* &_swapchainImages, VkImageView* &_swapchainImageViews,
                       VkFramebuffer* &_framebuffers, VkRenderPass &_renderPass);
#endif
