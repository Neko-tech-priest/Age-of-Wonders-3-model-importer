#include <cstring>
#include <cstdlib>
#include <memory>
#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"
#include "VulkanGlobalState.h"
#include "vk_check.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const char* validationLayers[] =
{
    "VK_LAYER_KHRONOS_validation"
};
const char* deviceExtensions[] =
{
    "VK_KHR_swapchain",
    "VK_EXT_descriptor_indexing",
    "VK_KHR_dynamic_rendering"
    //"VK_KHR_synchronization2",
    //"VK_EXT_descriptor_buffer"
    /*"VK_KHR_create_renderpass2",
    "VK_KHR_depth_stencil_resolve",
    "VK_KHR_dynamic_rendering",
    "VK_KHR_maintenance5"*/
    //"VK_EXT_descriptor_buffer"
};
//основные свойства устройства, такие как имя, тип и поддерживаемая версия Vulkan
extern VkPhysicalDeviceProperties deviceProperties;
//поддержка дополнительных функций, таких как сжатие текстур, 64-битные плавающие значения и рендеринг с несколькими окнами просмотра
extern VkPhysicalDeviceFeatures deviceFeatures;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    //std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    printf("%s%s\n", "validation layer: ", pCallbackData->pMessage);

    return VK_FALSE;
}
bool check_support_device_features(VkPhysicalDevice _physicalDevice)
{
    VkPhysicalDeviceDescriptorBufferFeaturesEXT VkPhysicalDeviceDescriptorBufferFeaturesEXT{};
    VkPhysicalDeviceDescriptorBufferFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;
    VkPhysicalDeviceFeatures2 VkPhysicalDeviceFeatures2{};
    VkPhysicalDeviceFeatures2.sType =  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    VkPhysicalDeviceFeatures2.pNext = &VkPhysicalDeviceDescriptorBufferFeaturesEXT;
    vkGetPhysicalDeviceFeatures2(_physicalDevice, &VkPhysicalDeviceFeatures2);
    printf("%d\n", VkPhysicalDeviceDescriptorBufferFeaturesEXT.descriptorBuffer);
    printf("%d\n", VkPhysicalDeviceDescriptorBufferFeaturesEXT.descriptorBufferCaptureReplay);
    printf("%d\n", VkPhysicalDeviceDescriptorBufferFeaturesEXT.descriptorBufferImageLayoutIgnored);
    printf("%d\n", VkPhysicalDeviceDescriptorBufferFeaturesEXT.descriptorBufferPushDescriptors);
    printf("\n");
    return true;
}
void activateFeatures(VkPhysicalDeviceFeatures2& VkPD_Features2)
{
    //фичи из Vulkan 1.0
    VkPD_Features2.features.samplerAnisotropy = true;

    VkPhysicalDeviceDescriptorBufferFeaturesEXT PhysicalDeviceDescriptorBufferFeaturesEXT{};
    PhysicalDeviceDescriptorBufferFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;
    PhysicalDeviceDescriptorBufferFeaturesEXT.descriptorBuffer = true;
    VkPD_Features2.pNext = &PhysicalDeviceDescriptorBufferFeaturesEXT;
    printf("%s%f\n", "deviceProperties.limits.maxSamplerAnisotropy: ", deviceProperties.limits.maxSamplerAnisotropy);
    /*void* currentPtrExtension = &EXT_structsList;
     *   void** pNextPtr = &VkPD_Features2.pNext;
     *   uint8_t indexStruct = 0;
     *   uint8_t indexFeature = 0;
     *   while(indexStruct < std::size(sizeof_EXT_structs_array))
     *   {
     *pNextPtr = currentPtrExtension;
     *       while(ptr_required_EXT_features[indexFeature] < currentPtrExtension + sizeof_EXT_structs_array[indexStruct])
     *       {
     *(VkBool32*)(ptr_required_EXT_features[indexFeature]) = true;
     *           indexFeature+=1;
     *           if(indexFeature == std::size(ptr_required_EXT_features))
     *               break;
}
pNextPtr = &((reinterpret_cast<VkPhysicalDeviceFeatures2*>(currentPtrExtension))->pNext);
currentPtrExtension += sizeof_EXT_structs_array[indexStruct];
indexStruct+=1;
}*/
}
void initBaseVulkan(SDL_Window* _window, VkExtent2D& _windowExtent, VkQueue& _graphicsQueue, uint32_t& _graphicsQueueFamily, VkSurfaceKHR& _surface)
{
    volkInitialize();
    if(enableValidationLayers)
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        VkLayerProperties* availableLayers = new VkLayerProperties[layerCount];
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

        for(size_t requiredLayerIndex = 0; requiredLayerIndex < sizeof(validationLayers)/sizeof(size_t); requiredLayerIndex+=1)
        {
            bool layerFound = false;
            for (size_t layerIndex = 0; layerIndex < layerCount; layerIndex+=1)
            {
                if (strcmp(validationLayers[requiredLayerIndex], availableLayers[layerIndex].layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound)
            {
                printf("validation layers requested, but not available!\n");
                exit(-1);
            }
        }
        delete[] availableLayers;
    }
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    uint32_t usedNumberOfInstanceExtensions = 0;
    SDL_Vulkan_GetInstanceExtensions(_window, &usedNumberOfInstanceExtensions, nullptr);
    const char* requiredExtensions[3];
    SDL_Vulkan_GetInstanceExtensions(_window, &usedNumberOfInstanceExtensions, requiredExtensions);
    if(enableValidationLayers)
    {
        requiredExtensions[usedNumberOfInstanceExtensions] = "VK_EXT_debug_utils";
        usedNumberOfInstanceExtensions+=1;
    }
    printf("used instance extensions:\n");
    for(size_t i = 0; i < usedNumberOfInstanceExtensions; i++)
    {
        printf("%s\n", requiredExtensions[i]);
    }
    VkInstanceCreateInfo instanseCreateInfo{};
    instanseCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanseCreateInfo.pApplicationInfo = &appInfo;

    instanseCreateInfo.enabledExtensionCount = usedNumberOfInstanceExtensions;
    instanseCreateInfo.ppEnabledExtensionNames = requiredExtensions;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

    if (enableValidationLayers)
    {
        instanseCreateInfo.enabledLayerCount = sizeof(validationLayers)/sizeof(size_t);
        instanseCreateInfo.ppEnabledLayerNames = validationLayers;

        //populateDebugMessengerCreateInfo(debugCreateInfo);
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;

        instanseCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
    } else
    {
        instanseCreateInfo.enabledLayerCount = 0;
        instanseCreateInfo.pNext = nullptr;
    }

    VK_CHECK(vkCreateInstance(&instanseCreateInfo, nullptr, &_instance));
    volkLoadInstance(_instance);
    //setupDebugMessenger();
    if (enableValidationLayers)
        VK_CHECK(vkCreateDebugUtilsMessengerEXT(_instance, &debugCreateInfo, nullptr, &_debugMessenger));
    //CreateSurface
    if (!SDL_Vulkan_CreateSurface(_window, _instance, &_surface))
    {
        printf("failed to create window surface!\n");
        exit(-1);
    }
    //PickPhysicalDevice
    uint32_t physicaldeviceCount;
    //список необходимых расширений устройства
    //const char* extensionPropertiesList[] ={VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    vkEnumeratePhysicalDevices(_instance, &physicaldeviceCount, nullptr);
    if (physicaldeviceCount == 0)
    {
        printf("failed to find GPUs with Vulkan support!\n");
        exit(-1);
    }
    VkPhysicalDevice physicalDevices[2];
    vkEnumeratePhysicalDevices(_instance, &physicaldeviceCount, physicalDevices);
    //поддерживаемые расширения
    VkExtensionProperties* extensionProperties;
    uint32_t extensionCount;
    //поддерживаемые семейства очередей
    VkQueueFamilyProperties* queueFamiliesProperties;
    uint32_t queueFamilyCount;

    /*for(size_t i = 0; i < physicaldeviceCount; i++)
     {                                                                   *
     printf("%s%ld%c\n", "GPU", i, ':');
     printf("%s\n", deviceProperties[i].deviceName);
     for(size_t j = 0; j < extensionCounts[i]; j++)
     {
     printf("%s\n", extensionProperties[i][j].extensionName);
}
}*/

    for(uint8_t indexDevice = 0; indexDevice < physicaldeviceCount; indexDevice++)
    {
        //получение сведений с устройства
        vkGetPhysicalDeviceProperties(physicalDevices[indexDevice], &deviceProperties);
        vkGetPhysicalDeviceFeatures(physicalDevices[indexDevice], &deviceFeatures);

        vkEnumerateDeviceExtensionProperties(physicalDevices[indexDevice], nullptr, &(extensionCount), nullptr);
        extensionProperties = new VkExtensionProperties[extensionCount];
        vkEnumerateDeviceExtensionProperties(physicalDevices[indexDevice], nullptr, &extensionCount, extensionProperties);

        //проверка на поддержку нужных расширений устройства
        bool extensionFound = false;
        for(uint8_t indexRequiredExtension = 0; indexRequiredExtension < sizeof(deviceExtensions)/sizeof(size_t); indexRequiredExtension++)
        {
            extensionFound = false;
            for(uint8_t indexAvailableExtension = 0; indexAvailableExtension < extensionCount; indexAvailableExtension++)
            {
                if(strcmp(deviceExtensions[indexRequiredExtension], extensionProperties[indexAvailableExtension].extensionName) == 0)
                {
                    extensionFound = true;
                    break;
                }
            }
            if(!extensionFound)
            {
                printf("%s%s!\n", "Extension not found: ", deviceExtensions[indexRequiredExtension]);
                break;
            }
        }
        if(!extensionFound)
        {
            delete[] extensionProperties;
            continue;
        }
        //проверка поддержки нужных фич

        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[indexDevice], &queueFamilyCount, nullptr);
        queueFamiliesProperties = new VkQueueFamilyProperties[queueFamilyCount];
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevices[indexDevice], &queueFamilyCount, queueFamiliesProperties);

        //проверка на наличие нужного семейства очередей
        bool checkQueueFamily = false;
        for(uint8_t indexQueueFamily = 0; indexQueueFamily < queueFamilyCount; indexQueueFamily++)
        {
            if(queueFamiliesProperties[indexQueueFamily].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                //проверка, может ли семейство очередей быть представлено на поверхности окна
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevices[indexDevice], indexQueueFamily, _surface, &presentSupport);
                if(presentSupport)
                {
                    checkQueueFamily = true;
                    _graphicsQueueFamily = indexQueueFamily;
					printf("%s%d\n", "_graphicsQueueFamily = ", _graphicsQueueFamily);
                    break;
                }
            }
        }
        if(!checkQueueFamily)
        {
            delete[] queueFamiliesProperties;
            continue;
        }

        _physicalDevice = physicalDevices[indexDevice];
        printf("%s%s\n", "chosen gpu: ",deviceProperties.deviceName);
        delete[] extensionProperties;
        delete[] queueFamiliesProperties;
        // VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU
        // VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
        if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            break;
    }
    if(_physicalDevice == VK_NULL_HANDLE)
    {
        printf("failed to find suidable device!\n");
        exit(-1);
    }

    //создание одной графической очереди
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = _graphicsQueueFamily;
    queueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    printf("\n");

    //активация фич
    VkPhysicalDeviceFeatures2 VkPD_Features2{};
    VkPD_Features2.sType =  VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    VkPD_Features2.features.samplerAnisotropy = true;

    VkPhysicalDeviceDynamicRenderingFeaturesKHR PhysicalDeviceDynamicRenderingFeaturesKHR{};
    PhysicalDeviceDynamicRenderingFeaturesKHR.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    PhysicalDeviceDynamicRenderingFeaturesKHR.dynamicRendering = VK_TRUE;
    VkPD_Features2.pNext = &PhysicalDeviceDynamicRenderingFeaturesKHR;
    /*VkPhysicalDeviceDescriptorBufferFeaturesEXT PhysicalDeviceDescriptorBufferFeaturesEXT{};
    PhysicalDeviceDescriptorBufferFeaturesEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT;
    PhysicalDeviceDescriptorBufferFeaturesEXT.descriptorBuffer = true;
    VkPD_Features2.pNext = &PhysicalDeviceDescriptorBufferFeaturesEXT;*/
    printf("%s%f\n", "deviceProperties.limits.maxSamplerAnisotropy: ", deviceProperties.limits.maxSamplerAnisotropy);
    //activateFeatures(VkPD_Features2);

    //создание устройства
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = &VkPD_Features2;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;

    deviceCreateInfo.enabledExtensionCount = sizeof(deviceExtensions)/sizeof(size_t);
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;

    VK_CHECK(vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_device));
    //создание одной очереди из семейства очередей
    vkGetDeviceQueue(_device, _graphicsQueueFamily, 0, &_graphicsQueue);
}
