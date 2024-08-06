#include "Image.h"

void readImageHeader(uint8_t*& file_bufferPtrIterator, ImageHeader& imageHeader)
{
    imageHeader.size = *(uint32_t*)(file_bufferPtrIterator);
    imageHeader.format = *(VkFormat*)(file_bufferPtrIterator+4);
    imageHeader.width = *(uint16_t*)(file_bufferPtrIterator+8);
    imageHeader.height = *(uint16_t*)(file_bufferPtrIterator+10);
    file_bufferPtrIterator+=12;
}
void read_Image_without_copy(uint8_t*& file_bufferPtrIterator, Image& image)
{
    image.size = *(uint32_t*)(file_bufferPtrIterator);
    image.format = *(VkFormat*)(file_bufferPtrIterator+4);
    image.width = *(uint16_t*)(file_bufferPtrIterator+8);
    image.height = *(uint16_t*)(file_bufferPtrIterator+10);
    file_bufferPtrIterator+=12;
    image.data = file_bufferPtrIterator;
}
