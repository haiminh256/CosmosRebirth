#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <set>
#include <optional>
#include <cstring>
#include <algorithm>
#include <fstream>

#ifdef _WIN32
#define COSMOS_API __declspec(dllexport)
#else
#define COSMOS_API __attribute__((visibility("default")))
#endif

#ifdef _DEBUG
constexpr bool EnableValidationLayers = true;
#else
constexpr bool EnableValidationLayers = false;
#endif

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;

    std::vector<VkSurfaceFormatKHR> formats;

    std::vector<VkPresentModeKHR> presentModes;
};