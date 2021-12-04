#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <vulkanx_SDL.h>

static void kill(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

static char* deepCopyString(const char* str) {
    if (str == NULL)
        return NULL;
    else {
        char* dup = malloc(strlen(str) + 1);
        memcpy(dup, str, strlen(str) + 1);
        return dup;
    }
}

static VkBool32 isPhysicalDeviceOkForSDL(
    VkPhysicalDevice physicalDevice, void* pUserData) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyCount, NULL);
    for (uint32_t queueFamilyIndex = 0; queueFamilyIndex < queueFamilyCount;
         queueFamilyIndex++) {
        // It is okay if it has a queue family which supports present
        // operations.
        VkBool32 supported = VK_FALSE;
        VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(
            physicalDevice, queueFamilyIndex, *(VkSurfaceKHR*)pUserData,
            &supported);
        if (result == VK_SUCCESS && supported == VK_TRUE) return VK_TRUE;
    }
    return VK_FALSE;
}

void vkxCreateSDLWindowOrExit(
    const VkxSDLWindowCreateInfo* pCreateInfo, VkxSDLWindow* pWindow) {
    assert(pCreateInfo);
    assert(pWindow);

    // Create SDL window.
    pWindow->window = SDL_CreateWindow(
        pCreateInfo->pInstanceCreateInfo->pApplicationName,
        pCreateInfo->positionX, pCreateInfo->positionY, pCreateInfo->sizeX,
        pCreateInfo->sizeY, pCreateInfo->windowFlags | SDL_WINDOW_VULKAN);
    if (!pWindow->window)
        kill(
            "failed to create SDL window\n"
            "SDL_GetError(): %s\n",
            SDL_GetError());

    // Get SDL instance extension count.
    unsigned int requiredExtensionCount = 0;
    uint32_t requestedExtensionCount =
        pCreateInfo->pInstanceCreateInfo->requestedExtensionCount;
    if (!SDL_Vulkan_GetInstanceExtensions(
            pWindow->window, &requiredExtensionCount, NULL))
        kill("failed to get SDL Vulkan instance extensions\n");
    // Allocate space for SDL extensions and user-requested extensions,
    // then get SDL extension names.
    uint32_t extensionCount = requiredExtensionCount;
    const char** ppExtensionNames = VKX_LOCAL_MALLOC(
        sizeof(const char*) *
        (requiredExtensionCount + requestedExtensionCount));
    if (!SDL_Vulkan_GetInstanceExtensions(
            pWindow->window, &extensionCount, ppExtensionNames))
        kill("failed to get SDL Vulkan instance extensions\n");

    // Add user-requested extension names that aren't duplicates of
    // SDL extension names.
    for (uint32_t requestIndex = 0; requestIndex < requestedExtensionCount;
         requestIndex++) {
        const char* pRequestedExtensionName =
            pCreateInfo->pInstanceCreateInfo
                ->ppRequestedExtensionNames[requestIndex];
        VkBool32 alreadyExists = VK_FALSE;
        for (const char** ppExtensionName = ppExtensionNames;
             ppExtensionName < ppExtensionNames + extensionCount;
             ppExtensionName++)
            if (!strcmp(*ppExtensionName, pRequestedExtensionName)) {
                alreadyExists = VK_TRUE;
                break;
            }
        if (alreadyExists == VK_FALSE)
            ppExtensionNames[extensionCount++] = pRequestedExtensionName;
    }

    // User-requested layers.
    uint32_t layerCount =
        pCreateInfo->pInstanceCreateInfo->requestedLayerCount;
    const char* const* ppLayerNames =
        pCreateInfo->pInstanceCreateInfo->ppRequestedLayerNames;

    // Allocate layer and extension enabled flags.
    VkBool32* pLayersEnabled = VKX_LOCAL_MALLOC(sizeof(VkBool32) * layerCount);
    VkBool32* pExtensionsEnabled =
        VKX_LOCAL_MALLOC(sizeof(VkBool32) * extensionCount);
    {
        // Create instance.
        VkxInstanceCreateInfo instanceCreateInfo;
        memcpy(
            &instanceCreateInfo, pCreateInfo->pInstanceCreateInfo,
            sizeof(VkxInstanceCreateInfo));
        instanceCreateInfo.requestedExtensionCount = extensionCount;
        instanceCreateInfo.ppRequestedExtensionNames = ppExtensionNames;
        VkResult result = vkxCreateInstance(
            &instanceCreateInfo, NULL, pLayersEnabled, pExtensionsEnabled,
            &pWindow->instance);
        if (result != VK_SUCCESS)
            kill(
                "failed to create Vulkan instance (%s)\n",
                vkxResultName(result));
    }

    // Initialize enabled layers.
    pWindow->enabledLayerCount = 0;
    pWindow->ppEnabledLayerNames =
        (char**)malloc(sizeof(char*) * extensionCount);
    for (uint32_t layerIndex = 0; layerIndex < layerCount; layerIndex++)
        if (pLayersEnabled[layerIndex] == VK_TRUE) {
            pWindow->ppEnabledLayerNames[pWindow->enabledLayerCount] =
                deepCopyString(ppLayerNames[layerIndex]);
            pWindow->enabledLayerCount++;
        }

    // Initialize enabled extensions.
    pWindow->enabledExtensionCount = 0;
    pWindow->ppEnabledExtensionNames =
        (char**)malloc(sizeof(char*) * extensionCount);
    for (uint32_t extensionIndex = 0; extensionIndex < extensionCount;
         extensionIndex++) {
        if (pExtensionsEnabled[extensionIndex] == VK_TRUE) {
            pWindow->ppEnabledExtensionNames[pWindow->enabledExtensionCount] =
                deepCopyString(ppExtensionNames[extensionIndex]);
            pWindow->enabledExtensionCount++;
            continue; // Skip ahead.
        }
        // If extension is required but not enabled, report error and exit.
        if (extensionIndex < requiredExtensionCount)
            kill(
                "can't enable Vulkan extension \"%s\" required by SDL\n",
                ppExtensionNames[extensionIndex]);
    }

    // Free locals.
    VKX_LOCAL_FREE(pLayersEnabled);
    VKX_LOCAL_FREE(pExtensionsEnabled);
    VKX_LOCAL_FREE(ppExtensionNames);

    // Create surface.
    if (!SDL_Vulkan_CreateSurface(
            pWindow->window, pWindow->instance, &pWindow->swapchainSurface))
        kill("failed to create Vulkan swapchain surface\n");

    {
        // Swapchain extension name.
        const char* swapchainExtensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        // Select physical device which supports present
        // operations.
        VkxPhysicalDeviceSelectInfo physicalDeviceSelectInfo = {
            .pRequestedName = NULL,
            .pRequestedFeatures = NULL,
            .pIsPhysicalDeviceOk = isPhysicalDeviceOkForSDL,
            .pUserData = &pWindow->swapchainSurface};
        // Use 1 graphics queue family, with up to 4 queues,
        // which supports present operations.
        VkxDeviceQueueFamilyCreateInfo queueFamilyCreateInfo = {
            .queueFlags = VK_QUEUE_GRAPHICS_BIT,
            .queueCount = 4,
            .minQueueCount = 1,
            .useEqualPriority = VK_FALSE,
            .presentSurface = pWindow->swapchainSurface,
            .commandPoolCount = pCreateInfo->overrideCommandPoolCount,
            .pCommandPoolCreateFlags =
                pCreateInfo->pOverrideCommandPoolCreateFlags};
        // If no command pools, create 1 command pool with no special
        // flags by default.
        VkCommandPoolCreateFlags defaultCommandPoolCreateFlags = 0;
        if (queueFamilyCreateInfo.commandPoolCount == 0) {
            queueFamilyCreateInfo.commandPoolCount = 1;
            queueFamilyCreateInfo.pCommandPoolCreateFlags =
                &defaultCommandPoolCreateFlags;
        }
        // Create device.
        VkxDeviceCreateInfo deviceCreateInfo = {
            .pSelectInfo = &physicalDeviceSelectInfo,
            .queueFamilyCreateInfoCount = 1,
            .pQueueFamilyCreateInfos = &queueFamilyCreateInfo,
            .enabledExtensionCount = 1,
            .ppEnabledExtensionNames = &swapchainExtensionName};
        VkResult result = vkxCreateDevice(
            pWindow->instance, &deviceCreateInfo, NULL, &pWindow->device);
        if (result != VK_SUCCESS)
            kill(
                "failed to create Vulkan device (%s)\n",
                vkxResultName(result));
    }

    {
        // Create swapchain.
        int drawableWidth = 0;
        int drawableHeight = 0;
        SDL_Vulkan_GetDrawableSize(
            pWindow->window, &drawableWidth, &drawableHeight);
        VkExtent2D surfaceExtent = {
            .width = (uint32_t)drawableWidth,
            .height = (uint32_t)drawableHeight};
        VkResult result = vkxCreateSwapchain(
            pWindow->device.physicalDevice, pWindow->device.device,
            pWindow->device.pQueueFamilies[0].queueFamilyIndex,
            pWindow->device.pQueueFamilies[0].queueFamilyIndex,
            pWindow->swapchainSurface, surfaceExtent, NULL,
            &pWindow->swapchain);
        if (result != VK_SUCCESS)
            kill(
                "failed to create Vulkan swapchain (%s)\n",
                vkxResultName(result));
    }

    {
        // Create default sampler.
        VkSamplerCreateInfo samplerCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0,
            .anisotropyEnable = VK_FALSE,
            .maxAnisotropy = 1,
            .compareEnable = VK_FALSE,
            .compareOp = (VkCompareOp)0,
            .minLod = -1000,
            .maxLod = +1000,
            .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
            .unnormalizedCoordinates = VK_FALSE};
        VkResult result = vkCreateSampler(
            pWindow->device.device, &samplerCreateInfo, NULL,
            &pWindow->defaultSampler);
        if (result != VK_SUCCESS)
            kill(
                "failed to create default sampler (%s)\n",
                vkxResultName(result));
    }
}

void vkxDestroySDLWindow(VkxSDLWindow* pWindow) {
    if (pWindow) {
        // Free layer names.
        for (uint32_t nameIndex = 0; nameIndex < pWindow->enabledLayerCount;
             nameIndex++)
            free(pWindow->ppEnabledLayerNames[nameIndex]);
        free(pWindow->ppEnabledLayerNames);
        // Free extension names.
        for (uint32_t nameIndex = 0;
             nameIndex < pWindow->enabledExtensionCount; nameIndex++)
            free(pWindow->ppEnabledExtensionNames[nameIndex]);
        free(pWindow->ppEnabledExtensionNames);
        // Destroy default sampler.
        vkDestroySampler(
            pWindow->device.device, pWindow->defaultSampler, NULL);
        // Destroy swapchain.
        vkxDestroySwapchain(&pWindow->swapchain, NULL);
        // Destroy swapchain surface.
        vkDestroySurfaceKHR(
            pWindow->instance, pWindow->swapchainSurface, NULL);
        // Destroy device.
        vkxDestroyDevice(&pWindow->device, NULL);
        // Destroy instance.
        vkDestroyInstance(pWindow->instance, NULL);
        // Destroy window.
        SDL_DestroyWindow(pWindow->window);
        // Nullify.
        memset(pWindow, 0, sizeof(VkxSDLWindow));
    }
}

void vkxSDLWindowResizeSwapchainOrExit(VkxSDLWindow* pWindow) {
    assert(pWindow);
    vkDeviceWaitIdle(pWindow->device.device);
    int drawableWidth = 0;
    int drawableHeight = 0;
    SDL_Vulkan_GetDrawableSize(
        pWindow->window, &drawableWidth, &drawableHeight);
    VkExtent2D surfaceExtent = {
        .width = (uint32_t)drawableWidth, .height = (uint32_t)drawableHeight};
    VkResult result = vkxRecreateSwapchain(
        pWindow->swapchainSurface, surfaceExtent, NULL, &pWindow->swapchain);
    if (result != VK_SUCCESS)
        kill("failed to recreate swapchain (%s)\n", vkxResultName(result));
}
