#pragma once
#include <Cosmos/EngineEnv.h>
#include <Cosmos/Engine.h>
#include <Cosmos/Renderer/Swapchain.h>

namespace Cosmos {
    class COSMOS_API Renderer {
    public:
        void Init();
        void ShowSupportedEXT();
        void Shutdown();
        bool SetupDebugMessenger();
        void ShowAvailableValidationLayers();
        void PickPhysicalDevice();
        void CreateLogicalDevice();
        void CreateSurface(GLFWwindow* window);
        void CreatePresentationQueue();
        VkDevice getVkDevice();
        VkPhysicalDevice getPhysicalDevice();
        VkQueue getGraphicsQueue();
        VkQueue getPresentQueue();
        VkSurfaceKHR getSurface();
        SwapChainSupportDetails QuerySwapChainSupport(
            VkPhysicalDevice physicalDevice,
            VkSurfaceKHR surface);
        QueueFamilyIndices getQueueFamilyIndices();

    private:
        VkInstance instance = VK_NULL_HANDLE;
        VkApplicationInfo appInfo{};
        VkInstanceCreateInfo createInfo{};

        // --- CÁC BIẾN MỚI CHO LOGICAL DEVICE & QUEUES ---
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue presentQueue = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkDeviceCreateInfo deviceCreateInfo{};
        QueueFamilyIndices indices;

    private:
        void DestroyDebugMessenger();
        VkDebugUtilsMessengerEXT debugMessenger{};

    private:
        bool CheckValidationLayerSupport();
        std::vector<const char*> GetRequiredExtensions();

        void PopulateDebugMessengerCreateInfo(
            VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        bool isDeviceSuitable(VkPhysicalDevice device);


        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);

        // Chọn Queue Family.
        QueueFamilyIndices FindQueueFamilies(
            VkPhysicalDevice device);

        // Kiểm tra GPU có hỗ trợ Extension không.
        bool CheckDeviceExtensionSupport(
            VkPhysicalDevice device);
        // --- HÀM HELPER NỘI BỘ CHO BƯỚC TIẾP THEO ---
    };
}
