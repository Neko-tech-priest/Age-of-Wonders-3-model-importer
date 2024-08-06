#include <memory>
#include <chrono>
#include <string>
#include "algebra.h"
#include "frames.h"
#include "vk_check.h"
#include "initBaseVulkan.h"
#include "initVulkan.h"
#include "VkSwapchainKHR.h"
#include "VkPipeline.h"
#include "VkBuffer.h"
#include "VkImage.h"
#include "descriptor.h"
#include "cameraBufferObject.h"
#include "TerrainMaterial.h"
#include "AoW3.h"
#include "AoW3_clb_importer.h"
#include "dds_reader.h"
//#include "jxl_decode.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkSampler textureSampler;
VkBuffer uniformBuffers[FRAME_OVERLAP];
VkDeviceMemory uniformBuffersMemory[FRAME_OVERLAP];
void* uniformBuffersMapped[FRAME_OVERLAP];

VkBuffer cameraBuffers[FRAME_OVERLAP];
VkDeviceMemory cameraBuffersMemory[FRAME_OVERLAP];
void* cameraBuffersMapped[FRAME_OVERLAP];

VkImage depthImage;
VkImageView depthImageView;
VkDeviceMemory depthDeviceMemory;
extern VkFormat _depthFormat = VK_FORMAT_D32_SFLOAT;

uint32_t _currentFrame = 0;
VkExtent2D _windowExtent{512, 512};
struct SDL_Window* _window{ nullptr };

SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

VkInstance _instance{};
VkDebugUtilsMessengerEXT _debugMessenger{};
VkPhysicalDevice _physicalDevice{};
VkDevice _device{};

VkQueue _graphicsQueue;
uint32_t _graphicsQueueFamily;

VkSemaphore _presentSemaphores[FRAME_OVERLAP];
VkSemaphore _renderSemaphores[FRAME_OVERLAP];
VkFence _renderFences[FRAME_OVERLAP];

VkSurfaceKHR _surface;
VkSwapchainKHR _swapchain;
extern VkFormat _swapchainImageFormat = VK_FORMAT_B8G8R8A8_SRGB;

VkFramebuffer* _framebuffers;
VkImage* _swapchainImages;
VkImageView* _swapchainImageViews;
extern uint32_t _swapchainImagesCount = 0;

VkCommandPool _commandPool;
//VkCommandBuffer _commandBuffers[_currentFrame];
VkCommandBuffer _commandBuffers[FRAME_OVERLAP];

VkRenderPass _renderPass;

VkDescriptorSetLayout cameraDescriptorSetLayout;
VkDescriptorPool cameraDescriptorPool;
VkDescriptorSet cameraDescriptorSets[FRAME_OVERLAP];

struct UniformBufferObject
{
    mat4 model;
};
VkPhysicalDeviceProperties deviceProperties;
VkPhysicalDeviceFeatures deviceFeatures;
void createTextureSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    // VK_FILTER_NEAREST
    // VK_FILTER_LINEAR
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    // VK_SAMPLER_MIPMAP_MODE_NEAREST
    // VK_SAMPLER_MIPMAP_MODE_LINEAR
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VK_CHECK(vkCreateSampler(_device, &samplerInfo, nullptr, &textureSampler));
}
void createCameraBuffers()
{
    VkDeviceSize bufferSize = sizeof(CameraBufferObject);

    for (size_t i = 0; i < FRAME_OVERLAP; i++)
    {
        createVkBuffer__VkDeviceMemory__HV_DL(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, bufferSize, cameraBuffers[i], cameraBuffersMemory[i]);
        //createBuffer__DeviceMemory(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize, cameraBuffers[i], cameraBuffersMemory[i]);

        vkMapMemory(_device, cameraBuffersMemory[i], 0, bufferSize, 0, &cameraBuffersMapped[i]);
    }
}
float camera_translate_x = 0;
float camera_translate_y = 0;
float camera_translate_z = 0;
float camera_rotate_x = 0;
float camera_rotate_y = 0;
float camera_rotate_z = 0;
void updateCameraBuffer(uint32_t currentImage, VkExtent2D& _windowExtent)
{
    //static auto startTime = std::chrono::high_resolution_clock::now();
    //auto currentTime = std::chrono::high_resolution_clock::now();
    //float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    mat4 CoordinateSystem{};
    CoordinateSystem.rotate(90, 'x');

    CameraBufferObject camera{};
    mat4 cameraScale;
    mat4 cameraRotate;
    mat4 cameraTranslate;
    cameraScale.scale(1, 1, 1);
    cameraRotate.rotate(camera_rotate_x, 'x');
    cameraTranslate.translate(0+camera_translate_x, 0+camera_translate_y, 2+camera_translate_z);
    //cameraTranslate.translate(0, 0, 64);
    //camera.view = cameraTranslate * cameraRotate;
    camera.view = cameraRotate * cameraTranslate;
    //camera.view = camera.view * CoordinateSystem;
    //camera.view = CoordinateSystem * camera.view;

    camera.proj.perspective(90.0f, (float)_windowExtent.width /(float)_windowExtent.height, 1.0/64, 64.0f);
    memcpy(cameraBuffersMapped[currentImage], &camera, sizeof(camera));
}
/*void updateUnitBuffer(const Map& map, SC_model& model)
{
    int32_t mapWidthHalf = map.widthInt/2;
    int32_t mapHeightHalf = map.heightInt/2;
    int32_t unit_position_x_int = model.unit_position_x;
    int32_t unit_position_y_int = model.unit_position_y;
    float unit_position_x_remainder = model.unit_position_x - unit_position_x_int;
    float unit_position_y_remainder = model.unit_position_y - unit_position_y_int;

    float height_x0y0 = map.heightMapData[(unit_position_y_int+256)*(map.widthInt+1)+unit_position_x_int+256]*map.heightMapScale;
    float height_x1y0 = map.heightMapData[(unit_position_y_int+256)*(map.widthInt+1)+unit_position_x_int+256+1]*map.heightMapScale;
    float height_x0y1 = map.heightMapData[(unit_position_y_int+256+1)*(map.widthInt+1)+unit_position_x_int+256]*map.heightMapScale;
    float height_x1y1 = map.heightMapData[(unit_position_y_int+256+1)*(map.widthInt+1)+unit_position_x_int+256+1]*map.heightMapScale;

    printf("%s%f\n", "height_x0y0: ", height_x0y0);
    printf("%s%f\n", "height_x1y0: ", height_x1y0);
    printf("%s%f\n", "height_x0y1: ", height_x0y1);
    printf("%s%f\n", "height_x1y1: ", height_x1y1);
    printf("\n");

    float height_difference_U = height_x1y0-height_x0y0;
    float height_difference_D = height_x1y1-height_x0y1;
    float height_difference_L = height_x0y1-height_x0y0;
    float height_difference_R = height_x1y1-height_x1y0;
    printf("%f\n", height_difference_U);
    printf("%f\n", height_difference_D);
    printf("%f\n", height_difference_L);
    printf("%f\n", height_difference_R);
    printf("\n");
    float average_height_difference_W = mix(height_difference_U, height_difference_D, unit_position_y_remainder);
    float average_height_difference_H = mix(height_difference_L, height_difference_R, unit_position_x_remainder);

    vec3 vector_height_difference_W(1, 0, average_height_difference_W);
    vec3 vector_height_difference_H(0, 1, average_height_difference_H);
    vector_height_difference_W.normalize();
    vector_height_difference_H.normalize();
    vec3 normal = vector_multiplication(vector_height_difference_W, vector_height_difference_H);
    printf("%f %f %f \n", vector_height_difference_W[0], vector_height_difference_W[1], vector_height_difference_W[2]);
    printf("%f %f %f \n", vector_height_difference_H[0], vector_height_difference_H[1], vector_height_difference_H[2]);
    printf("%f %f %f \n", normal[0], normal[1], normal[2]);
    printf("\n");

    float averageHeight_U = (height_x0y0*unit_position_x_remainder + height_x1y0*(1.0-unit_position_x_remainder));
    float averageHeight_D = (height_x0y1*unit_position_x_remainder + height_x1y1*(1.0-unit_position_x_remainder));
    model.unit_position_z = (averageHeight_U*unit_position_y_remainder + averageHeight_D*(1.0-unit_position_y_remainder));

    model.positionMatrix.translate(model.unit_position_x, model.unit_position_y, model.unit_position_z);
    memcpy(model.matrixMapMemory, &model.positionMatrix, sizeof(mat4));
}*/
//extern GlobalFace globalFaces[6];
int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    _window = SDL_CreateWindow(
		"Vulkan Engine",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		_windowExtent.width,
		_windowExtent.height,
		window_flags
	);
	initBaseVulkan(_window, _windowExtent, _graphicsQueue, _graphicsQueueFamily, _surface);
    createVkSwapchainKHR(_window, _windowExtent, _surface, _swapchain, _swapchainImages, _swapchainImageViews);
    createDepthResources(_windowExtent.width, _windowExtent.height, _depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageView, depthDeviceMemory);
    init_default_renderpass(_swapchainImageFormat, _depthFormat, _renderPass);
    init_framebuffers(_windowExtent, _renderPass, _swapchainImages, _swapchainImageViews, depthImageView, _framebuffers);
    init_commands(_graphicsQueueFamily, _commandPool, _commandBuffers);
    init_sync_structures(_renderFences, _presentSemaphores, _renderSemaphores);

    createTextureSampler();
    createCameraBuffers();
    createCameraVkDescriptorSetLayout(_device, cameraDescriptorSetLayout);
    createCameraVkDescriptorPool(_device, cameraDescriptorPool);
    createCameraVkDescriptorSets(_device, cameraBuffers, cameraDescriptorSetLayout, cameraDescriptorPool, cameraDescriptorSets);

    // Age of Wonders 3
    // квадрат
    struct TerrainVertex
    {
        vec3 position;
        vec2 uv;
    };
    TerrainVertex square_vertices[4] =
    {
        {{-1, -1, 0},{0, 0}},
        {{1, -1, 0},{1, 0}},
        {{1, 1, 0},{1, 1}},
        {{-1, 1, 0},{0, 1}}
    };
    uint16_t square_indices[2][3] =
    {
        {0, 1, 2},
        {2, 3, 0}
    };
    VkBuffer squareVertexVkBuffer = 0;
    VkDeviceMemory squareVertexVkDeviceMemory = 0;
    VkBuffer squareIndexVkBuffer = 0;
    VkDeviceMemory squareIndexVkDeviceMemory = 0;
    createVkBuffer__VkDeviceMemory(_graphicsQueue, _commandBuffers[0], VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, (void*)square_vertices, sizeof(TerrainVertex)*4, squareVertexVkBuffer, squareVertexVkDeviceMemory);
    createVkBuffer__VkDeviceMemory(_graphicsQueue, _commandBuffers[0], VK_BUFFER_USAGE_INDEX_BUFFER_BIT, (void*)square_indices, sizeof(uint16_t)*6, squareIndexVkBuffer, squareIndexVkDeviceMemory);

    AoW3_Mesh* AoW3_meshes = 0;
    uint32_t AoW3_meshesCount = 0;
    VkDeviceMemory AoW3_meshes_vertexVkDeviceMemory = 0;
    VkDeviceMemory AoW3_meshes_indexVkDeviceMemory = 0;
    //constexpr size_t AoW3_TEXTURES_COUNT = 1;
    Image* AoW3_images = 0;
    uint32_t AoW3_imagesCount = 0;

    int32_t AoW3_meshIndex = 0;
    int32_t AoW3_textureIndex = 0;

    TerrainMaterial* AoW3_textures;
    VkDeviceMemory textureVkDeviceMemory;

    // Characters/
        // Skin_DirePenguin
    // Pickups/
    // Structures/
        // City_NonSpecific
        // City_Elves
        // Pickup_Chest
        // Resource_GoldOre
        // Resource_ManaNode
    // Terrain/
        // All_Mountains_Round

    //Arc_RiverOrigin
    //Rocks

    //All_TerrainTextures
    //Arc_DenseVegetation
    //Wat_TerrainTextures
    AoW3_clb_read("/run/media/dima/sda4/Games/Age of Wonders 3/Content/Title/Libraries/Pickups/Pickups.clb", AoW3_meshes, AoW3_meshesCount, AoW3_images, AoW3_imagesCount);
    if(AoW3_meshesCount > 0)
    {
        createVkBuffers__VkDeviceMemory_AoS(_graphicsQueue, _commandBuffers[0], VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, (uint8_t*)AoW3_meshes, sizeof(AoW3_Mesh), offsetof(AoW3_Mesh, verticesBufferSize), offsetof(AoW3_Mesh, vertexVkBuffer), AoW3_meshesCount, AoW3_meshes_vertexVkDeviceMemory);
        createVkBuffers__VkDeviceMemory_AoS(_graphicsQueue, _commandBuffers[0], VK_BUFFER_USAGE_INDEX_BUFFER_BIT, ((uint8_t*)(AoW3_meshes))+8, sizeof(AoW3_Mesh), offsetof(AoW3_Mesh, indicesBufferSize)-8, offsetof(AoW3_Mesh, indexVkBuffer)-8, AoW3_meshesCount, AoW3_meshes_indexVkDeviceMemory);
        //createVkBuffer__VkDeviceMemory(_graphicsQueue, _commandBuffers[0], VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, (void*)AoW3_meshes[AoW3_meshIndex].verticesBuffer, AoW3_meshes[AoW3_meshIndex].verticesBufferSize, AoW3_meshes[AoW3_meshIndex].vertexVkBuffer, AoW3_meshes[AoW3_meshIndex].vertexVkDeviceMemory);
        //createVkBuffer__VkDeviceMemory(_graphicsQueue, _commandBuffers[0], VK_BUFFER_USAGE_INDEX_BUFFER_BIT, (void*)AoW3_meshes[AoW3_meshIndex].indicesBuffer, AoW3_meshes[AoW3_meshIndex].indicesCount<<1, AoW3_meshes[AoW3_meshIndex].indexVkBuffer, AoW3_meshes[AoW3_meshIndex].indexVkDeviceMemory);
    }
    //return 0;
    //printf("%lu\n", AoW3_meshes[AoW3_meshIndex].vertexVkBuffer);
    AoW3_textures = new TerrainMaterial[AoW3_imagesCount];
    createVkImages__VkImageViews__VkDeviceMemory_AoS(_graphicsQueue, _commandBuffers[0], AoW3_images, (uint8_t*)AoW3_textures, sizeof(TerrainMaterial), AoW3_imagesCount, textureVkDeviceMemory);

    VkDescriptorSetLayout terrainDescriptorSetLayout;
    VkDescriptorPool terrainDescriptorPool;

    VkPipeline AoW3_PNUT_Pipeline = 0;
    VkPipelineLayout AoW3_PNUT_PipelineLayout = 0;
    VkPipeline AoW3_PNUCIIIWWT_Pipeline = 0;
    VkPipelineLayout AoW3_PNUCIIIWWT_PipelineLayout = 0;

    VkPipeline terrainPipeline = 0;
    VkPipelineLayout terrainPipelineLayout = 0;

    //return 0;
    Create_TerrainMaterial_VkDescriptorSetLayout(terrainDescriptorSetLayout);
    //allocateDescriptors(_commandBuffers[0], terrainDescriptorSetLayout);
    //return 0;

    Create_TerrainMaterial_VkDescriptorPool(terrainDescriptorPool, AoW3_imagesCount);
    //return 0;
    for(size_t i = 0; i < AoW3_imagesCount; i+=1)
    {
        AoW3_textures[i].Create_TerrainMaterial_VkDescriptorSet(terrainDescriptorPool, terrainDescriptorSetLayout, textureSampler);
    }

    AoW3_Create_PNUT_Pipeline(_windowExtent, _renderPass, cameraDescriptorSetLayout, terrainDescriptorSetLayout, AoW3_PNUT_PipelineLayout, AoW3_PNUT_Pipeline);
    //AoW3_Create_PNUCIIIWWT_Pipeline(_windowExtent, _renderPass, cameraDescriptorSetLayout, terrainDescriptorSetLayout, AoW3_PNUCIIIWWT_PipelineLayout, AoW3_PNUCIIIWWT_Pipeline);
    //Create_TerrainMaterial_Pipeline(_windowExtent, _renderPass, cameraDescriptorSetLayout, terrainDescriptorSetLayout, terrainPipelineLayout, terrainPipeline);

    //updateUnitBuffer(map, model);

    SDL_Event e;
    bool bQuit = false;
    bQuit = false;
    bool windowPresent = true;
    //uint32_t swapchainImageIndex = 0;
    while (!bQuit)
    {
        //Handle events on queue
        while (SDL_PollEvent(&e) != 0)
        {
            switch(e.type)
            {
            case SDL_QUIT:
                bQuit = true;
                break;
            case SDL_WINDOWEVENT:
            {
                switch(e.window.event)
                {
                case SDL_WINDOWEVENT_SHOWN:
                    windowPresent = true;
                    break;
                case SDL_WINDOWEVENT_HIDDEN:
                    windowPresent = false;
                    break;
                }
                break;
            }
            case SDL_KEYDOWN:
            {
                switch(e.key.keysym.scancode)
                {
                // камера
                // X
                case SDL_SCANCODE_D:
                {
                    camera_translate_x+=0.5;
                    break;
                }
                case SDL_SCANCODE_A:
                {
                    camera_translate_x-=0.5;
                    break;
                }
                // Y
                case SDL_SCANCODE_W:
                {
                    camera_translate_z+=0.5;
                    break;
                }
                case SDL_SCANCODE_S:
                {
                    camera_translate_z-=0.5;
                    break;
                }
                // Z
                case SDL_SCANCODE_E:
                {
                    camera_translate_y+=0.5;
                    break;
                }
                case SDL_SCANCODE_Q:
                {
                    camera_translate_y-=0.5;
                    break;
                }
                case SDL_SCANCODE_O:
                {
                    AoW3_meshIndex-=1;
                    if(AoW3_meshIndex < 0)
                        AoW3_meshIndex = 0;
                    break;
                }
                case SDL_SCANCODE_P:
                {
                    AoW3_meshIndex+=1;
                    if(AoW3_meshIndex == AoW3_meshesCount)
                        AoW3_meshIndex = AoW3_meshIndex-=1;
                    break;
                }
                case SDL_SCANCODE_K:
                {
                    AoW3_textureIndex-=1;
                    if(AoW3_textureIndex < 0)
                        AoW3_textureIndex = 0;
                    break;
                }
                case SDL_SCANCODE_L:
                {
                    AoW3_textureIndex+=1;
                    if(AoW3_textureIndex == AoW3_imagesCount)
                        AoW3_textureIndex = AoW3_textureIndex-=1;
                    break;
                }
                // повороты
                case SDL_SCANCODE_UP:
                {
                    /*AoW3_textureIndex-=1;
                    if(AoW3_textureIndex < 0)
                        AoW3_textureIndex = 0;*/
                    camera_rotate_x-=5;
                    break;
                }
                case SDL_SCANCODE_DOWN:
                {
                    /*AoW3_textureIndex+=1;
                    if(AoW3_textureIndex == AoW3_imagesCount)
                        AoW3_textureIndex = AoW3_textureIndex-=1;*/
                    camera_rotate_x+=5;
                    break;
                }
                case SDL_SCANCODE_LEFT:
                {
                    camera_rotate_z-=5;
                    break;
                }
                case SDL_SCANCODE_RIGHT:
                {
                    camera_rotate_z+=5;
                    break;
                }
                // юнит
                /*case SDL_SCANCODE_KP_6:
                {
                    model.unit_position_x+=4;
                    break;
                }
                case SDL_SCANCODE_KP_4:
                {
                    model.unit_position_x-=4;
                    break;
                }
                case SDL_SCANCODE_KP_8:
                {
                    model.unit_position_y-=4;
                    break;
                }
                case SDL_SCANCODE_KP_2:
                {
                    model.unit_position_y+=4;
                    break;
                }*/
                }
                break;
            }
            }
                //close the window when user alt-f4s or clicks the X button
            //if (e.type == SDL_QUIT) bQuit = true;
        }
        if (!windowPresent)//SDL_GetWindowFlags(_window) & SDL_WINDOW_MINIMIZED
            SDL_Delay(50);
        else
        {

            //wait until the gpu has finished rendering the last frame
            VK_CHECK(vkWaitForFences(_device, 1, &_renderFences[_currentFrame], true, UINT64_MAX));
            //request image from the swapchain
            uint32_t swapchainImageIndex;
            VkResult result;
            //VK_CHECK
            result = (vkAcquireNextImageKHR(_device, _swapchain, UINT64_MAX, _presentSemaphores[_currentFrame], nullptr, &swapchainImageIndex));
            if (result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                recreateSwapchain(_window, _windowExtent, _surface,
                                  depthImage, depthImageView, depthDeviceMemory, _depthFormat,
                                  _swapchain, _swapchainImages, _swapchainImageViews,
                                  _framebuffers, _renderPass);
                //vkAcquireNextImageKHR(_device, _swapchain, UINT64_MAX, _presentSemaphores[_currentFrame], nullptr, &swapchainImageIndex);
            }
            else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            {
                printf("failed to acquire swap chain image!\n");
                exit(-1);
            }
            //updateUniformBuffer(_currentFrame, _windowExtent);
            updateCameraBuffer(_currentFrame, _windowExtent);
            //updateUnitBuffer(map, model);
            VK_CHECK(vkResetFences(_device, 1, &_renderFences[_currentFrame]));
            //now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
            VK_CHECK(vkResetCommandBuffer(_commandBuffers[_currentFrame], 0));
            //begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
            VkCommandBufferBeginInfo cmdBeginInfo{};
            cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            VK_CHECK(vkBeginCommandBuffer(_commandBuffers[_currentFrame], &cmdBeginInfo));

            const VkImageMemoryBarrier image_memory_barrierBegin {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .image = _swapchainImages[swapchainImageIndex],
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                }
            };

            vkCmdPipelineBarrier(
                _commandBuffers[_currentFrame],
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,  // srcStageMask
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // dstStageMask
                0,
                0,
                nullptr,
                0,
                nullptr,
                1, // imageMemoryBarrierCount
                &image_memory_barrierBegin // pImageMemoryBarriers
            );

            VkClearValue clearValue = {{{ 0.5f, 0.5f, 1.0f, 1.0f }}};
            VkClearValue depthClear;
            depthClear.depthStencil.depth = 1.0f;

            const VkRect2D renderArea
            {
                .offset = {0, 0},
                .extent = _windowExtent,
            };
            /*typedef struct VkRenderingInfoKHR
            {
                VkStructureType                        sType;
                const void*                            pNext;
                VkRenderingFlagsKHR                    flags;
                VkRect2D                               renderArea;
                uint32_t                               layerCount;
                uint32_t                               viewMask;
                uint32_t                               colorAttachmentCount;
                const VkRenderingAttachmentInfoKHR*    pColorAttachments;
                const VkRenderingAttachmentInfoKHR*    pDepthAttachment;
                const VkRenderingAttachmentInfoKHR*    pStencilAttachment;
            }
            */
            /*typedef struct VkRenderingAttachmentInfoKHR
            {
                VkStructureType          sType;
                const void*              pNext;
                VkImageView              imageView;
                VkImageLayout            imageLayout;
                VkResolveModeFlagBits    resolveMode;
                VkImageView              resolveImageView;
                VkImageLayout            resolveImageLayout;
                VkAttachmentLoadOp       loadOp;
                VkAttachmentStoreOp      storeOp;
                VkClearValue             clearValue;
            }
            */

            // VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR
            const VkRenderingAttachmentInfoKHR colorAttachmentInfo
            {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = _swapchainImageViews[swapchainImageIndex],
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = clearValue,
            };
            const VkRenderingAttachmentInfoKHR depthAttachmentInfo
            {
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .imageView = depthImageView,
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,//VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = depthClear,
            };
            const VkRenderingInfoKHR renderInfo
            {
                .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
                .renderArea = renderArea,
                .layerCount = 1,
                .colorAttachmentCount = 1,
                .pColorAttachments = &colorAttachmentInfo,
                .pDepthAttachment = &depthAttachmentInfo,
            };

            vkCmdBeginRenderingKHR(_commandBuffers[_currentFrame], &renderInfo);

            //start the main renderpass.
            //We will use the clear color from above, and the framebuffer of the index the swapchain gave us
            /*VkRenderPassBeginInfo rpInfo{};
            rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            rpInfo.renderPass = _renderPass;
            rpInfo.renderArea.offset = {0, 0};
            rpInfo.renderArea.extent = _windowExtent;
            rpInfo.framebuffer = _framebuffers[swapchainImageIndex];
            //connect clear values
            rpInfo.clearValueCount = 2;
            VkClearValue clearValues[] = {clearValue, depthClear};
            rpInfo.pClearValues = clearValues;*/

            //vkCmdBeginRenderPass(_commandBuffers[_currentFrame], &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float) _windowExtent.width;
            viewport.height = (float) _windowExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(_commandBuffers[_currentFrame], 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = _windowExtent;
            vkCmdSetScissor(_commandBuffers[_currentFrame], 0, 1, &scissor);

            //once we start adding rendering commands, they will go here

            /*vkCmdBindPipeline(_commandBuffers[_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, mountainPipeline);
            VkBuffer vertexBuffers[] = {mountainMesh.vertexVkBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(_commandBuffers[_currentFrame], 0, 1, &mountainMesh.vertexVkBuffer, offsets);
            vkCmdBindIndexBuffer(_commandBuffers[_currentFrame], mountainMesh.indexVkBuffer, 0, VK_INDEX_TYPE_UINT16);

            VkDescriptorSet descriptorSets[1];
            descriptorSets[0] = cameraDescriptorSets[_currentFrame];
            vkCmdBindDescriptorSets(_commandBuffers[_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, mountainPipelineLayout, 0, 1, descriptorSets, 0, nullptr);


            vkCmdDrawIndexed(_commandBuffers[_currentFrame], 195, 1, 0, 0, 0);*/

            //vkCmdBindPipeline(_commandBuffers[_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, terrainPipeline);
            vkCmdBindPipeline(_commandBuffers[_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, AoW3_PNUT_Pipeline);
            //vkCmdBindPipeline(_commandBuffers[_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, AoW3_PNUCIIIWWT_Pipeline);

            //VkBuffer vertexBuffers[] = {squareVertexVkBuffer};
            VkDeviceSize offsets[] = {0};
            //vkCmdBindVertexBuffers(_commandBuffers[_currentFrame], 0, 1, &squareVertexVkBuffer, offsets);
            //vkCmdBindIndexBuffer(_commandBuffers[_currentFrame], squareIndexVkBuffer, 0, VK_INDEX_TYPE_UINT16);
            vkCmdBindVertexBuffers(_commandBuffers[_currentFrame], 0, 1, &AoW3_meshes[AoW3_meshIndex].vertexVkBuffer, offsets);
            vkCmdBindIndexBuffer(_commandBuffers[_currentFrame], AoW3_meshes[AoW3_meshIndex].indexVkBuffer, 0, VK_INDEX_TYPE_UINT16);

            VkDescriptorSet descriptorSets[2];
            descriptorSets[0] = cameraDescriptorSets[_currentFrame];
            //descriptorSets[1] = terrainTextures[0].descriptorSet;
            descriptorSets[1] = AoW3_textures[AoW3_textureIndex].descriptorSet;

            //vkCmdBindDescriptorSets(_commandBuffers[_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, terrainPipelineLayout, 0, 2, descriptorSets, 0, nullptr);
            vkCmdBindDescriptorSets(_commandBuffers[_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, AoW3_PNUT_PipelineLayout, 0, 2, descriptorSets, 0, nullptr);
            //vkCmdDrawIndexed(_commandBuffers[_currentFrame], 3*2, 1, 0, 0, 0);
            vkCmdDrawIndexed(_commandBuffers[_currentFrame], AoW3_meshes[AoW3_meshIndex].indicesBufferSize>>1, 1, 0, 0, 0);

            vkCmdEndRenderingKHR(_commandBuffers[_currentFrame]);

            const VkImageMemoryBarrier imageMemoryBarrierEnd {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .image = _swapchainImages[swapchainImageIndex],
                .subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                }
            };

            vkCmdPipelineBarrier(
                _commandBuffers[_currentFrame],
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,  // srcStageMask
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // dstStageMask
                0,
                0,
                nullptr,
                0,
                nullptr,
                1, // imageMemoryBarrierCount
                &imageMemoryBarrierEnd // pImageMemoryBarriers
            );

            //finalize the render pass
            //vkCmdEndRenderPass(_commandBuffers[_currentFrame]);
            //finalize the command buffer (we can no longer add commands, but it can now be executed)
            VK_CHECK(vkEndCommandBuffer(_commandBuffers[_currentFrame]));

            //prepare the submission to the queue.
            //we want to wait on the _presentSemaphores[_currentFrame], as that semaphore is signaled when the swapchain is ready
            //we will signal the _renderSemaphores[_currentFrame], to signal that rendering has finished

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            //VkSemaphore waitSemaphores[] = _presentSemaphores[_currentFrame];
            VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            submitInfo.pWaitDstStageMask = &waitStage;

            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &_presentSemaphores[_currentFrame];

            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &_renderSemaphores[_currentFrame];

            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &_commandBuffers[_currentFrame];

            //submit command buffer to the queue and execute it.
            // _renderFence will now block until the graphic commands finish execution
            VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _renderFences[_currentFrame]));

            // this will put the image we just rendered into the visible window.
            // we want to wait on the _renderSemaphores[_currentFrame] for that,
            // as it's necessary that drawing commands have finished before the image is displayed to the user
            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

            presentInfo.pSwapchains = &_swapchain;
            presentInfo.swapchainCount = 1;

            presentInfo.pWaitSemaphores = &_renderSemaphores[_currentFrame];
            presentInfo.waitSemaphoreCount = 1;

            presentInfo.pImageIndices = &swapchainImageIndex;
            //VK_CHECK
            result = (vkQueuePresentKHR(_graphicsQueue, &presentInfo));
            if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
            {
                recreateSwapchain(_window, _windowExtent, _surface,
                                  depthImage, depthImageView, depthDeviceMemory, _depthFormat,
                                  _swapchain, _swapchainImages, _swapchainImageViews,
                                  _framebuffers, _renderPass);
            }

            //increase the number of frames drawn
            //_currentFrame = (_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
            _currentFrame+=1;
            if(_currentFrame == FRAME_OVERLAP)
                _currentFrame = 0;
            //SDL_Delay(500);
            //swapchainImageIndex+=1;
            //if(swapchainImageIndex == 2)
                //swapchainImageIndex = 0;
                //return 0;
        }
        //return 0;
    }
    //make sure the gpu has stopped doing its things
    vkDeviceWaitIdle(_device);

    vkDestroyBuffer(_device, squareVertexVkBuffer, nullptr);
    vkFreeMemory(_device, squareVertexVkDeviceMemory, nullptr);
    vkDestroyBuffer(_device, squareIndexVkBuffer, nullptr);
    vkFreeMemory(_device, squareIndexVkDeviceMemory, nullptr);

    delete[] AoW3_meshes;
    vkFreeMemory(_device, AoW3_meshes_vertexVkDeviceMemory, nullptr);
    vkFreeMemory(_device, AoW3_meshes_indexVkDeviceMemory, nullptr);
    for(size_t i = 0; i < AoW3_meshesCount; i++)
        AoW3_meshes[i].GPU_memory_unload();
    delete[] AoW3_images;
    for(size_t i = 0; i < AoW3_imagesCount; i++)
        AoW3_textures[i].GPU_memory_unload();
    delete[] AoW3_textures;
    vkFreeMemory(_device, textureVkDeviceMemory, nullptr);

    vkDestroyDescriptorSetLayout(_device, terrainDescriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(_device, terrainDescriptorPool, nullptr);

    vkDestroyPipeline(_device, AoW3_PNUT_Pipeline, nullptr);
    vkDestroyPipelineLayout(_device, AoW3_PNUT_PipelineLayout, nullptr);
    vkDestroyPipeline(_device, AoW3_PNUCIIIWWT_Pipeline, nullptr);
    vkDestroyPipelineLayout(_device, AoW3_PNUCIIIWWT_PipelineLayout, nullptr);
    vkDestroyPipeline(_device, terrainPipeline, nullptr);
    vkDestroyPipelineLayout(_device, terrainPipelineLayout, nullptr);
    /*mountainMesh.GPU_memory_unload();
    vkDestroyPipeline(_device, mountainPipeline, nullptr);
    vkDestroyPipelineLayout(_device, mountainPipelineLayout, nullptr);*/

    /*for(size_t i = 0; i < TEXTURES_COUNT; i++)
    {
       terrainTextures[i].GPU_memory_unload();
    }
    vkFreeMemory(_device, textureVkDeviceMemory, nullptr);

    vkDestroyDescriptorSetLayout(_device, terrainDescriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(_device, terrainDescriptorPool, nullptr);

    vkDestroyPipeline(_device, terrainPipeline, nullptr);
    vkDestroyPipelineLayout(_device, terrainPipelineLayout, nullptr);*/

    // Chunk

    /*cobblestoneTexture.GPU_memory_unload();
    cobblestone_chunk.GPU_memory_unload();

    vkDestroyDescriptorSetLayout(_device, textureDescriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(_device, textureDescriptorPool, nullptr);

    vkDestroyPipeline(_device, chunkPipeline, nullptr);
    vkDestroyPipelineLayout(_device, chunkPipelineLayout, nullptr);*/

    vkDestroySampler(_device, textureSampler, nullptr);
    for (size_t i = 0; i < FRAME_OVERLAP; i++)
    {
        vkDestroyBuffer(_device, cameraBuffers[i], nullptr);
        vkFreeMemory(_device, cameraBuffersMemory[i], nullptr);
    }
    vkDestroyDescriptorSetLayout(_device, cameraDescriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(_device, cameraDescriptorPool, nullptr);

    vkDestroyCommandPool(_device, _commandPool, nullptr);

    for(uint8_t i = 0; i < FRAME_OVERLAP; i++)
    {
        vkDestroyFence(_device, _renderFences[i], nullptr);
        vkDestroySemaphore(_device, _renderSemaphores[i], nullptr);
        vkDestroySemaphore(_device, _presentSemaphores[i], nullptr);
    }

    vkDestroyRenderPass(_device, _renderPass, nullptr);

    //destroy swapchain resources
    cleanupSwapChain(depthImageView, depthImage, depthDeviceMemory, _swapchainImageViews, _framebuffers, _swapchain);
    delete[] _framebuffers;
    delete[] _swapchainImages;
    delete[] _swapchainImageViews;
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
    vkDestroyDevice(_device, nullptr);
    if (enableValidationLayers)
        vkDestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
    vkDestroyInstance(_instance, nullptr);

    SDL_DestroyWindow(_window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
