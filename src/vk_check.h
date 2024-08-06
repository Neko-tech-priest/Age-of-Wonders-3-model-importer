#ifndef VK_CHECK_H_INCLUDED
#define VK_CHECK_H_INCLUDED
#include <stdio.h>
#include <cstdlib>
#include "volk.h"
//#include <vulkan/vulkan.h>
inline void VK_CHECK(VkResult err)
{
    if (err)
    {
        //std::cout <<"Detected Vulkan error: " << err << std::endl;
        printf("%s%d\n", "Detected Vulkan error: ", err);
        exit(-1);
    }
}
#endif // VK_CHECK_H_INCLUDED
