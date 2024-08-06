#ifndef Image_H
#define Image_H
#include "volk.h"

struct Image
{
    uint8_t* data = 0;
    uint32_t size = 0;
    VkFormat format = VK_FORMAT_UNDEFINED;
    uint32_t width = 0;
    uint32_t height = 0;
    //VkFormat format = VK_FORMAT_UNDEFINED;
    ~Image()
    {
        delete[] data;
    }
};
struct ImageHeader
{
    uint32_t size = 0;
    VkFormat format = VK_FORMAT_UNDEFINED;
    uint32_t width = 0;
    uint32_t height = 0;
};
void readImageHeader(uint8_t*& file_bufferPtrIterator, ImageHeader& imageHeader);
void read_Image_without_copy(uint8_t*& file_bufferPtrIterator, Image& image);
#endif
