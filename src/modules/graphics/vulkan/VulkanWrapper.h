#ifndef LOVE_GRAPHICS_VULKAN_VULKANWRAPPER_H
#define LOVE_GRAPHICS_VULKAN_VULKANWRAPPER_H

#include "common/config.h"

#ifdef LOVE_ANDROID
#include "volk.h"
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#else
#include <vulkan/vulkan.h>
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#endif
#include "vk_mem_alloc.h"

#endif //LOVE_GRAPHICS_VULKAN_VULKANWRAPPER_H
