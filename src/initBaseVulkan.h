#ifndef INITBASEVULKAN
#define INITBASEVULKAN

#include "SDL2/SDL.h"
//#include <SDL2/SDL_vulkan.h>
#include "volk.h"

void initBaseVulkan(SDL_Window* _window, VkExtent2D& _windowExtent, VkQueue& _graphicsQueue, uint32_t& _graphicsQueueFamily, VkSurfaceKHR& _surface);

#endif
