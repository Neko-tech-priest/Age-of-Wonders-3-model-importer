 #include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <iostream>
#include <vector>
#include <cstring>
#include <cstdlib>
#include "volk.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

struct VulkanStruct {
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

//private:
    VkExtent2D _windowExtent{ 500 , 500 };

	struct SDL_Window* _window{ nullptr };

    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debugMessenger;

    void initWindow();
    void initVulkan();

    void mainLoop()
    {
        SDL_Event e;
        bool bQuit = false;

        //main loop
        while (!bQuit)
        {
            //Handle events on queue
            while (SDL_PollEvent(&e) != 0)
            {
                //close the window when user alt-f4s or clicks the X button
                if (e.type == SDL_QUIT) bQuit = true;
            }
            SDL_Delay(50);
            //draw();
        }
    }

    void cleanup() {
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
        }

        vkDestroyInstance(_instance, nullptr);

        SDL_DestroyWindow(_window);
        SDL_Quit();
    }
};
