#pragma once
#define GLFW_INCLUDE_VULKAN
#include <iostream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

#ifdef _WIN32
#define COSMOS_API __declspec(dllexport)
#else
#define COSMOS_API __attribute__((visibility("default")))
#endif