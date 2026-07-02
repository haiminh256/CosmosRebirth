#pragma once

#include <Cosmos/EngineEnv.h>

namespace Cosmos
{
    class Swapchain
    {
    public:
        void COSMOS_API CreateSwapchain(
            VkPhysicalDevice physicalDevice,
            VkDevice device,
            VkSurfaceKHR surface,
            GLFWwindow* window,
            QueueFamilyIndices& indices);

        void COSMOS_API CreateImageViews();

        void COSMOS_API Destroy(VkDevice device);

        VkSwapchainKHR COSMOS_API GetSwapChain() const { return swapChain; }

        VkFormat COSMOS_API GetImageFormat() const { return swapChainImageFormat; }

        VkExtent2D GetExtent() const { return swapChainExtent; }

        const COSMOS_API std::vector<VkImageView>& GetImageViews() const
        {
            return swapChainImageViews;
        }
        SwapChainSupportDetails COSMOS_API QuerySwapChainSupport(
            VkPhysicalDevice physicalDevice,
            VkSurfaceKHR surface);
    private:

        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& formats);

        VkPresentModeKHR ChooseSwapPresentMode(
            const std::vector<VkPresentModeKHR>& presentModes);

        VkExtent2D ChooseSwapExtent(
            const VkSurfaceCapabilitiesKHR& capabilities,
            GLFWwindow* window);

    private:

        VkSwapchainKHR swapChain = VK_NULL_HANDLE;

        std::vector<VkImage> swapChainImages;

        std::vector<VkImageView> swapChainImageViews;

        VkFormat swapChainImageFormat;

        VkExtent2D swapChainExtent;

        VkDevice logicalDevice = VK_NULL_HANDLE;
        QueueFamilyIndices indices;
    };
}