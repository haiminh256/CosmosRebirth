#include <Cosmos/Renderer/SwapChain.h>
#include <Cosmos/Renderer/Renderer.h>
#include <Cosmos/Core/Log.h>

namespace Cosmos
{
    VkSurfaceFormatKHR
        Swapchain::ChooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& formats)
    {
        for (const auto& format : formats)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return format;
            }
        }

        return formats[0];
    }

    VkPresentModeKHR
        Swapchain::ChooseSwapPresentMode(
            const std::vector<VkPresentModeKHR>& presentModes)
    {
        for (const auto& mode : presentModes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
                return mode;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D
        Swapchain::ChooseSwapExtent(
            const VkSurfaceCapabilitiesKHR& capabilities,
            GLFWwindow* window)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
            return capabilities.currentExtent;

        int width;
        int height;

        glfwGetFramebufferSize(
            window,
            &width,
            &height);

        VkExtent2D extent =
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        extent.width =
            std::clamp(
                extent.width,
                capabilities.minImageExtent.width,
                capabilities.maxImageExtent.width);

        extent.height =
            std::clamp(
                extent.height,
                capabilities.minImageExtent.height,
                capabilities.maxImageExtent.height);

        return extent;
    }

    void Swapchain::CreateSwapchain(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkSurfaceKHR surface,
        GLFWwindow* window,
        QueueFamilyIndices& indices)
    {
        logicalDevice = device;

        SwapChainSupportDetails support =
            QuerySwapChainSupport(
                physicalDevice,
                surface);

        VkSurfaceFormatKHR surfaceFormat =
            ChooseSwapSurfaceFormat(
                support.formats);

        VkPresentModeKHR presentMode =
            ChooseSwapPresentMode(
                support.presentModes);

        VkExtent2D extent =
            ChooseSwapExtent(
                support.capabilities,
                window);

        uint32_t imageCount =
            support.capabilities.minImageCount + 1;

        if (support.capabilities.maxImageCount > 0 &&
            imageCount > support.capabilities.maxImageCount)
        {
            imageCount =
                support.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};

        createInfo.sType =
            VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

        createInfo.surface = surface;

        createInfo.minImageCount = imageCount;

        createInfo.imageFormat = surfaceFormat.format;

        createInfo.imageColorSpace =
            surfaceFormat.colorSpace;

        createInfo.imageExtent = extent;

        createInfo.imageArrayLayers = 1;

        createInfo.imageUsage =
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueFamilies[] =
        {
       
            indices.graphicsFamily.value(),
            indices.presentFamily.value()
        };

        if (indices.graphicsFamily != indices.  presentFamily)
        {
            createInfo.imageSharingMode =
                VK_SHARING_MODE_CONCURRENT;

            createInfo.queueFamilyIndexCount = 2;

            createInfo.pQueueFamilyIndices =
                queueFamilies;
        }
        else
        {
            createInfo.imageSharingMode =
                VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform =
            support.capabilities.currentTransform;

        createInfo.compositeAlpha =
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode =
            presentMode;

        createInfo.clipped = VK_TRUE;

        if (vkCreateSwapchainKHR(
            device,
            &createInfo,
            nullptr,
            &swapChain) != VK_SUCCESS)
        {
            CORE_ERROR("Failed to create SwapChain.");
            return;
        }

        vkGetSwapchainImagesKHR(
            device,
            swapChain,
            &imageCount,
            nullptr);

        swapChainImages.resize(imageCount);

        vkGetSwapchainImagesKHR(
            device,
            swapChain,
            &imageCount,
            swapChainImages.data());

        swapChainImageFormat =
            surfaceFormat.format;

        swapChainExtent =
            extent;
    }

    void Swapchain::CreateImageViews()
    {
        swapChainImageViews.resize(
            swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};

            createInfo.sType =
                VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

            createInfo.image =
                swapChainImages[i];

            createInfo.viewType =
                VK_IMAGE_VIEW_TYPE_2D;

            createInfo.format =
                swapChainImageFormat;

            createInfo.components.r =
                VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.components.g =
                VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.components.b =
                VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.components.a =
                VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask =
                VK_IMAGE_ASPECT_COLOR_BIT;

            createInfo.subresourceRange.baseMipLevel = 0;

            createInfo.subresourceRange.levelCount = 1;

            createInfo.subresourceRange.baseArrayLayer = 0;

            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(
                logicalDevice,
                &createInfo,
                nullptr,
                &swapChainImageViews[i]) != VK_SUCCESS)
            {
                CORE_ERROR("Failed to create Image View.");
            }
        }
    }

    void Swapchain::Destroy(VkDevice device)
    {
        for (auto imageView : swapChainImageViews)
        {
            vkDestroyImageView(
                device,
                imageView,
                nullptr);
        }

        vkDestroySwapchainKHR(
            device,
            swapChain,
            nullptr);
    }
    SwapChainSupportDetails Swapchain::QuerySwapChainSupport(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface)
    {
        SwapChainSupportDetails details{};

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            physicalDevice,
            surface,
            &details.capabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice,
            surface,
            &formatCount,
            nullptr);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);

            vkGetPhysicalDeviceSurfaceFormatsKHR(
                physicalDevice,
                surface,
                &formatCount,
                details.formats.data());
        }

        uint32_t presentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice,
            surface,
            &presentModeCount,
            nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);

            vkGetPhysicalDeviceSurfacePresentModesKHR(
                physicalDevice,
                surface,
                &presentModeCount,
                details.presentModes.data());
        }

        return details;
    }
}