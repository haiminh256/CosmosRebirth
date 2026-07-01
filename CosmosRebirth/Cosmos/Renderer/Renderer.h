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
        void CreateSurface(GLFWwindow* window);
        void CreatePresentationQueue();
        void CreateSwapChain(GLFWwindow* window);
        void CreateImageViews();
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

        VkSwapchainKHR swapChain = VK_NULL_HANDLE;

        std::vector<VkImage> swapChainImages;

        std::vector<VkImageView> swapChainImageViews;

        VkFormat swapChainImageFormat;

        VkExtent2D swapChainExtent;

        struct QueueFamilyIndices {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> presentFamily;

            bool isComplete() {
                return graphicsFamily.has_value() && presentFamily.has_value();
            }
        };

        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

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
        // Lấy toàn bộ thông tin Swapchain của GPU.
        SwapChainSupportDetails QuerySwapChainSupport(
            VkPhysicalDevice device);

        // Chọn Queue Family.
        QueueFamilyIndices FindQueueFamilies(
            VkPhysicalDevice device);

        // Kiểm tra GPU có hỗ trợ Extension không.
        bool CheckDeviceExtensionSupport(
            VkPhysicalDevice device);

        // Chọn Surface Format tốt nhất.
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR>& formats);

        // Chọn Present Mode.
        VkPresentModeKHR ChooseSwapPresentMode(
            const std::vector<VkPresentModeKHR>& presentModes);

        // Chọn kích thước Swapchain.
        VkExtent2D ChooseSwapExtent(
            const VkSurfaceCapabilitiesKHR& capabilities,
            GLFWwindow* window);

        // --- HÀM HELPER NỘI BỘ CHO BƯỚC TIẾP THEO ---
    };
}
