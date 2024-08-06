#include <stdio.h>
#include <cstdlib>
#include <cstring>
#include "algebra.h"
#include "Image.h"
#include "AoW3.h"
struct AoW3_MeshChunk
{
    uint8_t* verticesBuffer = 0;
    uint8_t* indicesBuffer = 0;
    uint32_t verticesBufferSize = 0;
    uint32_t indicesBufferSize = 0;
    ~AoW3_MeshChunk()
    {
        delete[] verticesBuffer;
        delete[] indicesBuffer;
    }
};

//constexpr uint32_t TX   = 0x58540000;
//constexpr uint32_t OBJ  = 0x4a424f00;
//constexpr uint32_t MAT  = 0x54414d00;
//constexpr uint32_t MESH = 0x4853454d;

//const char* SYSTEM_EFFECTS = "[SYSTEM_EFFECTS]";
//const char* Default = "Default";
void read_ChunkName(uint8_t* fileBufferPtrIterator)
{
    uint32_t nameLength = *(uint32_t*)fileBufferPtrIterator;
    fileBufferPtrIterator+=4;
    printf("%.*s\n", nameLength, fileBufferPtrIterator);
    fileBufferPtrIterator += nameLength;
    nameLength = *(uint32_t*)fileBufferPtrIterator;
    fileBufferPtrIterator+=4;
    printf("%.*s\n", nameLength, fileBufferPtrIterator);
    fileBufferPtrIterator += nameLength;
}
void readChunkTable(uint8_t* file_buffer, uint8_t*& fileBufferPtrIterator, char* libraryName, uint8_t libraryNameLength, uint32_t*& offsetsTable, uint32_t& blocksCount)
{
    uint8_t* chunkHeaderPtr = fileBufferPtrIterator;
    uint8_t nearBlocksCount = *fileBufferPtrIterator;
    fileBufferPtrIterator+=1;
    uint32_t farBlocksCount = 0;
    uint8_t* nearBlocksPtr;
    uint8_t* farBlocksPtr;
    if(nearBlocksCount > 0x80)
    {
        farBlocksCount = *(uint32_t*)fileBufferPtrIterator;
        fileBufferPtrIterator+=4;
        nearBlocksCount = nearBlocksCount & 127;
    }
    nearBlocksPtr = fileBufferPtrIterator;
    fileBufferPtrIterator+=(nearBlocksCount<<1);
    farBlocksPtr = fileBufferPtrIterator;
    fileBufferPtrIterator+=(farBlocksCount<<3);

    blocksCount = nearBlocksCount+farBlocksCount;
    uint8_t* zeroOffsetPtr = fileBufferPtrIterator;
    offsetsTable = new uint32_t[blocksCount];
    for(size_t i = 0; i < nearBlocksCount; i+=1)
        offsetsTable[i] = *((nearBlocksPtr+(i<<1))+1);
    for(size_t i = 0; i < farBlocksCount; i+=1)
        offsetsTable[nearBlocksCount+i] = *(uint32_t*)((farBlocksPtr+(i<<3))+4);
    if(memcmp(fileBufferPtrIterator+5, libraryName, libraryNameLength) != 0)
    {
        printf("this is not library name!\n");
        printf("%lx\n", fileBufferPtrIterator-file_buffer);
        exit(-1);
    }
    read_ChunkName(fileBufferPtrIterator);
    //return blocksCount;
}
void read_Texture_Chunk(uint8_t* file_buffer, uint8_t* fileBufferPtrIterator, uint32_t* offsetsTable, uint32_t blocksCount, Image &texture)
{
    //printf("%lx\n", fileBufferPtrIterator-file_buffer);
    uint8_t* zeroOffsetPtr = fileBufferPtrIterator;

    for(size_t i = 2; i < blocksCount; i+=1)
    {
        fileBufferPtrIterator = zeroOffsetPtr + offsetsTable[i];
        //printf("%lx\n", fileBufferPtrIterator-file_buffer);
        if(*(uint16_t*)fileBufferPtrIterator == 0x0101)
        {
            fileBufferPtrIterator+=3;
            uint8_t* chunkHeaderPtr = fileBufferPtrIterator;
            uint8_t nearBlocksCount = *fileBufferPtrIterator;
            fileBufferPtrIterator+=1;
            uint32_t farBlocksCount = 0;
            uint8_t* nearBlocksPtr;
            uint8_t* farBlocksPtr;
            if(nearBlocksCount > 0x80)
            {
                farBlocksCount = *(uint32_t*)fileBufferPtrIterator;
                fileBufferPtrIterator+=4;
                nearBlocksCount = nearBlocksCount & 127;
            }
            nearBlocksPtr = fileBufferPtrIterator;
            fileBufferPtrIterator+=(nearBlocksCount<<1);
            farBlocksPtr = fileBufferPtrIterator;
            fileBufferPtrIterator+=(farBlocksCount<<3);

            uint32_t blocksCount = nearBlocksCount+farBlocksCount;
            uint8_t* zeroOffsetPtr = fileBufferPtrIterator;
            uint32_t* offsetsTable = new uint32_t[blocksCount];
            for(size_t i = 0; i < nearBlocksCount; i+=1)
                offsetsTable[i] = *((nearBlocksPtr+(i<<1))+1);
            for(size_t i = 0; i < farBlocksCount; i+=1)
                offsetsTable[nearBlocksCount+i] = *(uint32_t*)((farBlocksPtr+(i<<3))+4);
            if(*(uint32_t*)fileBufferPtrIterator != 0x00410024)
            {
                printf("!= 0x00410024\n");
                printf("%lx\n", fileBufferPtrIterator-file_buffer);
                exit(-1);
            }
            fileBufferPtrIterator+=4;
            uint8_t mipHeaderBlocksCount = *fileBufferPtrIterator;
            fileBufferPtrIterator+=1;
            fileBufferPtrIterator+=(mipHeaderBlocksCount<<1);
            uint32_t tex_width = *(uint32_t*)fileBufferPtrIterator;
            uint32_t tex_height = *(uint32_t*)(fileBufferPtrIterator+4);
            uint32_t tex_format = *(uint32_t*)(fileBufferPtrIterator+8);
            printf("%s%d\n%s%d\n", "width: ", tex_width, "height: ", tex_height);
            fileBufferPtrIterator+=(*(fileBufferPtrIterator-1));
            uint32_t texture_size = 0;
            switch(tex_format)
            {
                case 0x05:
                    texture_size = (tex_width*tex_height)*4;
                    texture.format = VK_FORMAT_A8B8G8R8_SRGB_PACK32;
                    break;
                case 0x07:
                    texture_size = (tex_width*tex_height)/2;
                    texture.format = VK_FORMAT_BC1_RGB_SRGB_BLOCK;
                    break;
                case 0x09:
                    texture_size = (tex_width*tex_height);
                    texture.format = VK_FORMAT_BC2_SRGB_BLOCK;
                    break;
                case 0x0b:
                    texture_size = (tex_width*tex_height);
                    texture.format = VK_FORMAT_BC3_UNORM_BLOCK;
                    break;
                default:
                    printf("unknown texture image format\n");
                    exit(-1);
            }
            printf("%s%d\n","DXT", tex_format-6);
            texture.data = new uint8_t[texture_size];
            texture.size = texture_size;
            texture.width = tex_width;
            texture.height = tex_height;
            memcpy(texture.data, fileBufferPtrIterator, texture_size);
            delete []offsetsTable;
            break;
        }
    }
    //uint8_t* chunkHeaderPtr = fileBufferPtrIterator;
    /*uint16_t chunkDataType = *(uint16_t*)(fileBufferPtrIterator);
    if(*fileBufferPtrIterator > 0x80 && *(fileBufferPtrIterator+1) == 1)
        fileBufferPtrIterator+=((*fileBufferPtrIterator)<<1)-0xf3;
    else if(chunkDataType == 0x1409)
        fileBufferPtrIterator+=19;
    else if(chunkDataType == 0x140A)
        fileBufferPtrIterator+=21;
    else if(chunkDataType == 0x140B)
        fileBufferPtrIterator+=23;
    else
    {
        printf("unknown header data\n");
        printf("%lx\n", fileBufferPtrIterator-file_buffer);
        exit(-1);
    }
    if(memcmp(libraryName, fileBufferPtrIterator+5, libraryNameLength) != 0)
    {
        printf("this is not library name!\n");
        printf("%lx\n", fileBufferPtrIterator-file_buffer);
        exit(-1);
    }
    read_ChunkName(fileBufferPtrIterator);*/

    /*uint32_t bytesOffset = 0;
    while(*(uint32_t*)(fileBufferPtrIterator+bytesOffset) != 0)
    {
        bytesOffset+=4;
    }
    printf("%s%d\n", "zeroBytesOffset: ", bytesOffset);
    while(*(uint16_t*)(fileBufferPtrIterator+bytesOffset) != 0x0101)
    {
        bytesOffset+=1;
    }
    printf("%s%d\n", "0x0101 offset: ", bytesOffset);
    //printf("%lx\n", fileBufferPtrIterator-file_buffer);
    fileBufferPtrIterator+=bytesOffset;
    fileBufferPtrIterator+=2;

    if(*fileBufferPtrIterator != 0)
    {
        printf("!= 0\n");
        exit(-1);
    }
    fileBufferPtrIterator+=1;
    if(*fileBufferPtrIterator > 0x80)
    {
        uint8_t dataCount = *(fileBufferPtrIterator+1);
        if(((*fileBufferPtrIterator) & 127) == 1)
        {
            fileBufferPtrIterator+=7;
            fileBufferPtrIterator+=(dataCount*8);
        }
        else if(((*fileBufferPtrIterator) & 127) == 4 && *(fileBufferPtrIterator+1) == 1)
        {
            fileBufferPtrIterator+=21;
        }
        else
        {
            printf("unknown data\n");
            printf("%lx\n", fileBufferPtrIterator-file_buffer);
            exit(-1);
        }
    }
    else
    {
        uint8_t dataCount = *fileBufferPtrIterator;
        fileBufferPtrIterator+=1;
        printf("%s%d\n", "childChunksCount: ", dataCount);
        fileBufferPtrIterator+=(dataCount*2);
    }
    if(*(uint16_t*)fileBufferPtrIterator != 0x0024)
    {
        printf("!= 0x0024\n");
        printf("%lx\n", fileBufferPtrIterator-file_buffer);
        exit(-1);
    }
    fileBufferPtrIterator+=4;
    uint8_t* mipHeaderPtr = fileBufferPtrIterator;
    fileBufferPtrIterator+=((*fileBufferPtrIterator)<<1)+1;
    uint32_t tex_width = *(uint32_t*)fileBufferPtrIterator;
    uint32_t tex_height = *(uint32_t*)(fileBufferPtrIterator+4);
    uint32_t maybe_format = *(uint32_t*)(fileBufferPtrIterator+8);
    fileBufferPtrIterator+=12;
    if(*mipHeaderPtr == 5)
        fileBufferPtrIterator+=1;
    //printf("%lx\n", fileBufferPtrIterator-file_buffer);
    printf("%s%d\n%s%d\n", "width: ", tex_width, "height: ", tex_height);
    uint32_t texture_size = 0;
    switch(maybe_format)
    {
        case 0x05:
            texture_size = (tex_width*tex_height)*4;
            texture.format = VK_FORMAT_A8B8G8R8_SRGB_PACK32;
            break;
        case 0x07:
            texture_size = (tex_width*tex_height)/2;
            texture.format = VK_FORMAT_BC1_RGB_SRGB_BLOCK;
            break;
        case 0x09:
            texture_size = (tex_width*tex_height);
            texture.format = VK_FORMAT_BC2_SRGB_BLOCK;
            break;
        case 0x0b:
            texture_size = (tex_width*tex_height);
            texture.format = VK_FORMAT_BC3_UNORM_BLOCK;
            break;
        default:
            printf("unknown texture image format\n");
            exit(-1);
    }
    printf("%s%d\n","DXT", maybe_format-6);
    texture.data = new uint8_t[texture_size];
    texture.size = texture_size;
    texture.width = tex_width;
    texture.height = tex_height;
    memcpy(texture.data, fileBufferPtrIterator, texture_size);*/
}
void read_Mesh_Chunk(uint8_t* file_buffer, uint8_t* fileBufferPtrIterator, uint32_t* offsetsTable, uint32_t blocksCount, AoW3_Mesh& mesh, uint32_t& meshesCount)
{
    //printf("%lx\n", fileBufferPtrIterator-file_buffer);
    uint8_t* zeroOffsetPtr = fileBufferPtrIterator;
    const uint8_t MAT_Default_Material[5]           = {0x02, 0x14, 0x00, 0x17, 0x19};
    const uint8_t MESH_BlockIndices[8]              = {0x03, 0x14, 0x00, 0x15, 0x01, 0x16, 0x05, 0x01};
    const uint8_t MESH_BlockVertices_PNUT[38]       = {0x03, 0x14, 0x00, 0x15, 0x1f, 0x16, 0x23, 0x03, 0x15, 0x00, 0x16, 0x04, 0x17, 0x08, 0x30, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x04, 0x02, 0x00, 0x00, 0x05, 0x01, 0x00, 0x00, 0x09, 0x03};
    const uint8_t MESH_BlockVertices_PNUCT[42]      = {0x03, 0x14, 0x00, 0x15, 0x23, 0x16, 0x27, 0x03, 0x15, 0x00, 0x16, 0x04, 0x17, 0x08, 0x34, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x04, 0x02, 0x00, 0x00, 0x05, 0x01, 0x00, 0x00, 0x06, 0x0f, 0x00, 0x00, 0x09, 0x03};
    //const uint8_t MESH_BlockVertices_PNUUT[42]    = {0x03, 0x14, 0x00, 0x15, 0x23, 0x16, 0x27, 0x03, 0x15, 0x00, 0x16, 0x04, 0x17, 0x08, 0x38, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x04, 0x02, 0x00, 0x00, 0x05, 0x01, 0x01, 0x00, 0x05, 0x01, 0x00, 0x00, 0x09, 0x03};
    //const uint8_t MESH_BlockVertices_PNUUCT[46]   = {0x03, 0x14, 0x00, 0x15, 0x27, 0x16, 0x2b, 0x03, 0x15, 0x00, 0x16, 0x04, 0x17, 0x08, 0x3c, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x04, 0x02, 0x00, 0x00, 0x05, 0x01, 0x01, 0x00, 0x05, 0x01, 0x00, 0x00, 0x06, 0x0f, 0x00, 0x00, 0x09, 0x03};
    const uint8_t MESH_BlockVertices_PNUCIIIWWT[62] = {0x03, 0x14, 0x00, 0x15, 0x37, 0x16, 0x3b, 0x03, 0x15, 0x00, 0x16, 0x04, 0x17, 0x08, 0x39, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0x04, 0x02, 0x00, 0x00, 0x05, 0x01, 0x00, 0x00, 0x06, 0x0f, 0x00, 0x00, 0x0b, 0x04, 0x01, 0x00, 0x0b, 0x04, 0x02, 0x00, 0x0b, 0x04, 0x00, 0x01, 0x0a, 0x07, 0x01, 0x01, 0x0a, 0x07, 0x00, 0x00, 0x09, 0x03};
    bool blockReadIndices = false;
    bool blockReadVertices = false;
    /*
        for(size_t i = 0; i < 62; i+=1)
        {
            printf("0x%x, ", *(fileBufferPtrIterator+i));
        }
        printf("\n");
    }*/
    for(size_t i = 2; i < blocksCount; i+=1)
    {
        fileBufferPtrIterator = zeroOffsetPtr + offsetsTable[i];
        if(*(uint64_t*)fileBufferPtrIterator == *(uint64_t*)MESH_BlockIndices)
        {
            if(blockReadIndices == false)
            {
                printf("%lx\n", fileBufferPtrIterator-file_buffer);
                fileBufferPtrIterator+=8;
                uint32_t indicesCount = *(uint32_t*)fileBufferPtrIterator;
                fileBufferPtrIterator+=4;
                printf("%s%d\n", "indices count: ", indicesCount);

                mesh.indicesBuffer = new uint8_t[indicesCount*2];
                mesh.indicesBufferSize = indicesCount*2;
                memcpy(mesh.indicesBuffer, fileBufferPtrIterator, indicesCount*2);
                blockReadIndices = true;
                //meshesCount+=1;
            }
        }
        else if(memcmp(fileBufferPtrIterator, MESH_BlockVertices_PNUT, 38) == 0)
        {
            printf("vertex format: PNUT\n");
            const uint16_t vertexSize =  *(uint16_t*)(fileBufferPtrIterator+14);
            printf("%s%d\n", "vertex size: ", vertexSize);
            fileBufferPtrIterator+=38;
            uint32_t verticesCount = *(uint32_t*)fileBufferPtrIterator;
            fileBufferPtrIterator+=4;
            printf("%s%d\n", "vertices count: ", verticesCount);
            if(blockReadVertices == false)
            {
                mesh.verticesBuffer = new uint8_t[verticesCount*vertexSize];
                mesh.verticesBufferSize = verticesCount*vertexSize;
                memcpy(mesh.verticesBuffer, fileBufferPtrIterator, verticesCount*vertexSize);
                for(size_t i = 0; i < mesh.verticesBufferSize; i+=vertexSize)
                    *(float*)(mesh.verticesBuffer+i+4) *= -1;
                blockReadVertices = true;
                meshesCount+=1;
            }
        }
        else if(memcmp(fileBufferPtrIterator, MESH_BlockVertices_PNUCT, 42) == 0)
        {
            printf("vertex format: PNUCT\n");
        }
        else if(memcmp(fileBufferPtrIterator, MESH_BlockVertices_PNUCIIIWWT, 62) == 0)
        {
            printf("vertex format: PNUCIIIWWT\n");
            const uint16_t vertexSize =  *(uint16_t*)(fileBufferPtrIterator+14);
            printf("%s%d\n", "vertex size: ", vertexSize);
            uint16_t vertexSizeAligned = vertexSize + ((4 - vertexSize % 4) % 4);
            fileBufferPtrIterator+=62;
            uint32_t verticesCount = *(uint32_t*)fileBufferPtrIterator;
            fileBufferPtrIterator+=4;
            printf("%s%d\n", "vertices count: ", verticesCount);
            /*if(blockReadVertices == false)
            {
                mesh.verticesBuffer = new uint8_t[verticesCount*vertexSizeAligned];
                mesh.verticesBufferSize = verticesCount*vertexSizeAligned;
                for(size_t i = 0; i < verticesCount; i+=1)
                {
                    memcpy(mesh.verticesBuffer+i*vertexSizeAligned, fileBufferPtrIterator, vertexSize);
                    fileBufferPtrIterator+=vertexSize;
                }
                for(size_t i = 0; i < mesh.verticesBufferSize; i+=vertexSizeAligned)
                {
                    *(float*)(mesh.verticesBuffer+i+4) *= -1;
                    *(float*)(mesh.verticesBuffer+i+8) *= -1;
                }
                blockReadVertices = true;
                meshesCount+=1;
            }*/
        }
    }
    //delete[] offsetsTable;
    /*
    const uint8_t MESH_blockAfterName_0[7]  = {0x02, 0x18, 0x00, 0x19, 0x01, 0x00, 0x00};
    const uint8_t MESH_blockAfterName_1[12] = {0x02, 0x18, 0x00, 0x19, 0x01, 0x00, 0x00, 0x02, 0x14, 0x00, 0x17, 0x19};
    const uint8_t MESH_blockAfterName_2[8]  = {0x02, 0x18, 0x00, 0x19, 0x01, 0x00, 0x00, 0x00};
    const uint8_t MESH_blockAfterName_3[12] = {0x83, 0x02, 0x00, 0x00, 0x00, 0x14, 0x00, 0x15, 0x04, 0x16, 0x08, 0x18};

    const uint8_t MAT_Default_Material[5] = {0x02, 0x14, 0x00, 0x17, 0x19};
    const uint8_t MAT_header_1[9]               = {0x00, 0x03, 0x14, 0x00, 0x15, 0x01, 0x16, 0x05, 0x01};
    const uint8_t MAT_header_vectors[9]         = {0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x14, 0x00, 0x01};
    const uint8_t MAT_header_ShadowVertices[9]  = {0x01, 0x01, 0x00, 0x00, 0x02, 0x16, 0x00, 0x17, 0x04};
    */
    //meshesCount+=1;
}
void read_Model_Chunk(uint8_t* file_buffer, uint8_t* &fileBufferPtrIterator, char* libraryName, uint8_t libraryNameLength)
{
    uint8_t* zeroOffsetPtr = fileBufferPtrIterator;
}

void AoW3_clb_read(const char* path, AoW3_Mesh*& meshes, uint32_t &meshesCount, Image*& textures, uint32_t &texturesCount)
{
    //constexpr uint32_t dds = 0x7364642e;

    char libraryName[256]{0};
    const char* path_ptr_iterator = path;
    while(*path_ptr_iterator != 0)
    {
        path_ptr_iterator+=1;
    }
    const char* path_ptr_null = path_ptr_iterator;
    while(*path_ptr_iterator != '/')
    {
        path_ptr_iterator-=1;
    }
    path_ptr_iterator+=1;
    uint8_t libraryNameLength = path_ptr_null-path_ptr_iterator-4;
    memcpy(libraryName, path_ptr_iterator, libraryNameLength);
    //printf("%s\n", libraryName);
    char* libraryName_ptr_iterator = libraryName;
    while(*libraryName_ptr_iterator != 0)
    {
        if(*libraryName_ptr_iterator > 0x60 && *libraryName_ptr_iterator < 0x7b)
            *libraryName_ptr_iterator -= 0x20;
        libraryName_ptr_iterator+=1;
    }
    //printf("%s\n", libraryName);

    uint8_t *file_buffer;uint8_t* fileBufferPtrIterator;
    size_t file_size = 0;
    FILE *file = fopen(path, "rb");
    if(file == NULL)
    {
        printf("error open file\n");
        exit(-1);
    }
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    file_buffer = new uint8_t[file_size];
    fread(file_buffer, 1, file_size, file);
    fclose(file);
    fileBufferPtrIterator = file_buffer;

    while(memcmp(libraryName, fileBufferPtrIterator, libraryNameLength) != 0)
        fileBufferPtrIterator+=1;
    fileBufferPtrIterator+=libraryNameLength;
    if(*(uint16_t*)fileBufferPtrIterator != 257)
    {
        printf("%s\n", "!= 0x 01 01");
        exit(-1);
    }
    if(*(fileBufferPtrIterator+2) != 0)
    {
        printf("!= 0\n");
        exit(-1);
    }
    fileBufferPtrIterator+=3;
    uint32_t chunks_count;
    uint8_t* Ptr_on_offsetsTable;
    uint8_t* Ptr_on_ZeroChunk;
    if(*(fileBufferPtrIterator) == 0x81)
    {
        chunks_count = *(uint8_t*)(fileBufferPtrIterator+1);
        Ptr_on_offsetsTable = fileBufferPtrIterator+3;
        fileBufferPtrIterator+=7;
        fileBufferPtrIterator+=(chunks_count*8);
        Ptr_on_ZeroChunk = fileBufferPtrIterator;
        /*printf("%s\n", "!= 0x81");
        printf("%lx\n", fileBufferPtrIterator-file_buffer);
        exit(-1);*/
    }
    else
    {
        printf("unknown header data\n");
        exit(-1);
    }

    uint32_t modelsCount = 0;
    //uint32_t Textures_count = 0;
    for(size_t chunk_index = 0; chunk_index <= chunks_count; chunk_index++)
    {
        fileBufferPtrIterator = Ptr_on_ZeroChunk + *(uint32_t*)(Ptr_on_offsetsTable+chunk_index*8);
        uint8_t* chunkHeaderPtr = fileBufferPtrIterator;
        const uint16_t chunkType =  *(uint16_t*)fileBufferPtrIterator;
        while(memcmp(fileBufferPtrIterator, libraryName, libraryNameLength) != 0)
            fileBufferPtrIterator+=1;
        fileBufferPtrIterator+=libraryNameLength+1;
        //uint8_t* Ptr_typeName = fileBufferPtrIterator;
        //uint32_t chunkType = (*(uint32_t*)fileBufferPtrIterator);
        switch(chunkType)
        {
            case 0x0005:
            {
                //printf("%lx\n", chunkHeaderPtr-file_buffer);
                //printf("ANIM\n");
                break;
            }
            case 0x004b://OBJ
            {
                //printf("%lx\n", chunkHeaderPtr-file_buffer);
                //printf("OBJ\n");
                modelsCount+=1;
                break;
            }
            case 0x166f://MAT
            {
                //printf("%lx\n", chunkHeaderPtr-file_buffer);
                //printf("MAT\n");
                break;
            }
            case 0x003d://TX
            {
                texturesCount+=1;
                break;
            }
            case 0x0035://MESH
            {
                meshesCount+=1;
                break;
            }
            default:
                printf("%s%lx\n", "unknown chunk type: ", chunkType);
                printf("%lx\n", chunkHeaderPtr-file_buffer);
                //exit(-1);
        }
    }
    modelsCount = 0;

    //meshChunks = new AoW3_MeshChunk[MESH_count];
    printf("%s%d\n", "meshesCount: ", meshesCount);
    meshes = new AoW3_Mesh[meshesCount];
    meshesCount = 0;

    printf("%s%d\n", "texturesCount: ", texturesCount);
    textures = new Image[texturesCount];
    texturesCount = 0;

    // Models
    for(size_t chunk_index = 0; chunk_index <= chunks_count; chunk_index++)
    {
        fileBufferPtrIterator = Ptr_on_ZeroChunk + *(uint32_t*)(Ptr_on_offsetsTable+chunk_index*8);
        uint8_t* chunkHeaderPtr = fileBufferPtrIterator;
        const uint16_t chunkType =  *(uint16_t*)fileBufferPtrIterator;
        fileBufferPtrIterator+=4;
        if(chunkType == 0x004b)
        {
            printf("%lx\n", chunkHeaderPtr-file_buffer);
            //printf("OBJ\n");
            uint32_t* offsetsTable;
            uint32_t blocksCount;
            readChunkTable(file_buffer, fileBufferPtrIterator, libraryName, libraryNameLength, offsetsTable, blocksCount);
            read_Model_Chunk(file_buffer, fileBufferPtrIterator, libraryName, libraryNameLength);
            printf("\n");
            delete[] offsetsTable;
        }
    }
    // Meshes
    for(size_t chunk_index = 0; chunk_index <= chunks_count; chunk_index++)
    {
        fileBufferPtrIterator = Ptr_on_ZeroChunk + *(uint32_t*)(Ptr_on_offsetsTable+chunk_index*8);
        uint8_t* chunkHeaderPtr = fileBufferPtrIterator;
        const uint16_t chunkType =  *(uint16_t*)fileBufferPtrIterator;
        fileBufferPtrIterator+=4;
        if(chunkType == 0x0035)
        {
            printf("%lx\n", chunkHeaderPtr-file_buffer);
            uint32_t* offsetsTable;
            uint32_t blocksCount;
            readChunkTable(file_buffer, fileBufferPtrIterator, libraryName, libraryNameLength, offsetsTable, blocksCount);
            read_Mesh_Chunk(file_buffer, fileBufferPtrIterator, offsetsTable, blocksCount, meshes[meshesCount], meshesCount);
            //meshesCount+=1;
            printf("\n");
            delete[] offsetsTable;
        }
    }
    // Textures
    for(size_t chunk_index = 0; chunk_index <= chunks_count; chunk_index++)
    {
        fileBufferPtrIterator = Ptr_on_ZeroChunk + *(uint32_t*)(Ptr_on_offsetsTable+chunk_index*8);
        uint8_t* chunkHeaderPtr = fileBufferPtrIterator;
        const uint16_t chunkType =  *(uint16_t*)fileBufferPtrIterator;
        fileBufferPtrIterator+=4;
        if(chunkType == 0x003d)
        {
            printf("%lx\n", chunkHeaderPtr-file_buffer);
            uint32_t* offsetsTable;
            uint32_t blocksCount;
            readChunkTable(file_buffer, fileBufferPtrIterator, libraryName, libraryNameLength, offsetsTable, blocksCount);
            read_Texture_Chunk(file_buffer, fileBufferPtrIterator, offsetsTable, blocksCount, textures[texturesCount]);
            texturesCount+=1;
            printf("\n");
        }
    }
    if(meshesCount == 0)
    {
        exit(-1);
    }
    if(texturesCount == 0)
    {
        exit(-1);
    }
}
