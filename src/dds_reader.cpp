#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Image.h"

void readFileInBuffer(const char* file_name, uint8_t*& file_buffer)
{
    size_t file_size = 0;
    FILE *file = fopen(file_name, "rb");
    if(file == NULL)
    {
        printf("Не удалось открыть файл:\n");
        printf("%s\n", file_name);
        exit(-1);
    }
    fseek(file, 0, 2);
    file_size = ftell(file);
    fseek(file, 0, 0);
    file_buffer = new uint8_t[file_size];
    fread(file_buffer, 1, file_size, file);
    fclose(file);
}
void read_DDS_albedo_from_file(const char* file_name, Image& texture)
{
    uint8_t* file_buffer;
    readFileInBuffer(file_name, file_buffer);

    constexpr uint32_t ZERO = 0;
    constexpr uint32_t DXT1 = 0x31545844;
    constexpr uint32_t DXT3 = 0x33545844;
    constexpr uint32_t DXT5 = 0x35545844;

    texture.height = *(uint32_t*)(file_buffer+12);
    texture.width = *(uint32_t*)(file_buffer+16);
    uint32_t dwPitchOrLinearSize = *(uint32_t*)(file_buffer+20);
    uint32_t ddspf_dwFourCC = *(uint32_t*)(file_buffer+0x54);
    switch(ddspf_dwFourCC)
    {
        case ZERO:
            texture.format = VK_FORMAT_B8G8R8A8_SRGB;
            printf("%d\n", dwPitchOrLinearSize);
            break;
        case DXT1:
            texture.format = VK_FORMAT_BC1_RGB_SRGB_BLOCK;
            break;
        case DXT3:
            texture.format = VK_FORMAT_BC2_SRGB_BLOCK;
        case DXT5:
            texture.format = VK_FORMAT_BC3_SRGB_BLOCK;
            break;
        default:
            printf("unknown DDS image format:\n");
            printf("%s\n", file_name);
            exit(-1);
    }
    texture.size = dwPitchOrLinearSize;
    texture.data = new uint8_t[dwPitchOrLinearSize];
    memcpy(texture.data, file_buffer+128, dwPitchOrLinearSize);
    delete[] file_buffer;
}
void read_DDS_normal_from_file(const char* file_name, Image& texture)
{
    uint8_t* file_buffer;
    readFileInBuffer(file_name, file_buffer);

    constexpr uint32_t ZERO = 0;
    constexpr uint32_t DXT1 = 0x31545844;
    constexpr uint32_t DXT3 = 0x33545844;
    constexpr uint32_t DXT5 = 0x35545844;

    texture.height = *(uint32_t*)(file_buffer+12);
    texture.width = *(uint32_t*)(file_buffer+16);
    uint32_t dwPitchOrLinearSize = *(uint32_t*)(file_buffer+20);
    uint32_t ddspf_dwFourCC = *(uint32_t*)(file_buffer+0x54);
    switch(ddspf_dwFourCC)
    {
        case ZERO:
            /*if(dwPitchOrLinearSize < texture.height*texture.width*4)
             *               texture.format = VK_FORMAT_R8G8B8_UNORM;
             *           else
             *               texture.format = VK_FORMAT_B8G8R8A8_UNORM;*/
            texture.format = VK_FORMAT_B8G8R8A8_UNORM;
            printf("%d\n", dwPitchOrLinearSize);
            //dwPitchOrLinearSize = texture.height*texture.width*4;
            //printf("%d\n", dwPitchOrLinearSize);
            //printf("%d\n", texture.width);
            //printf("%d\n", texture.height);
            break;
        case DXT1:
            texture.format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
            break;
        case DXT3:
            texture.format = VK_FORMAT_BC2_UNORM_BLOCK;
            break;
        case DXT5:
            texture.format = VK_FORMAT_BC3_UNORM_BLOCK;
            break;
        default:
            printf("unknown DDS image format:\n");
            printf("%s\n", file_name);
            exit(-1);
    }
    texture.size = dwPitchOrLinearSize;
    texture.data = new uint8_t[dwPitchOrLinearSize];
    memcpy(texture.data, file_buffer+128, dwPitchOrLinearSize);
    delete[] file_buffer;
}
