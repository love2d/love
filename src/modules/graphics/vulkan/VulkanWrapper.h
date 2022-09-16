#pragma once

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#ifndef VK_MAKE_API_VERSION 
#define VK_MAKE_API_VERSION(variant, major, minor, patch) \
    ((((uint32_t)(variant)) << 29) | (((uint32_t)(major)) << 22) | (((uint32_t)(minor)) << 12) | ((uint32_t)(patch)))
#endif

#ifndef VK_API_VERSION_MAJOR
#define VK_API_VERSION_MAJOR(version) (((uint32_t)(version) >> 22) & 0x7FU)
#endif

#ifndef VK_API_VERSION_MINOR
#define VK_API_VERSION_MINOR(version) (((uint32_t)(version) >> 12) & 0x3FFU)
#endif

#ifndef VK_API_VERSION_PATCH
#define VK_API_VERSION_PATCH(version) ((uint32_t)(version) & 0xFFFU)
#endif

#include "libraries/volk/volk.h"
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "libraries/vma/vk_mem_alloc.h"
