#pragma once
#include <Cosmos/EngineEnv.h>
#include <Cosmos/Engine.h>

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
        void createSurface(GLFWwindow* window);
        void CreatePresentationQueue();
    private:
        VkInstance instance;
        VkApplicationInfo appInfo{};
        VkInstanceCreateInfo createInfo{};

        // --- CÁC BIẾN MỚI CHO LOGICAL DEVICE & QUEUES ---
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue presentQueue = VK_NULL_HANDLE;
        VkSurfaceKHR surface;
        VkDeviceCreateInfo deviceCreateInfo{};

    private:
        void DestroyDebugMessenger();
        VkDebugUtilsMessengerEXT debugMessenger{};

    private:
        bool CheckValidationLayerSupport();
        std::vector<const char*> GetRequiredExtensions();

        void PopulateDebugMessengerCreateInfo(
            VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);

        // --- HÀM HELPER NỘI BỘ CHO BƯỚC TIẾP THEO ---
    };
}
