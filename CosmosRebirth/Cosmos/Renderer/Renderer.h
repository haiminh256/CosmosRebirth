#pragma once
#include <Cosmos/EngineEnv.h>

namespace Cosmos {
	class COSMOS_API Renderer {
	public:
		void Init();
		void ShowSupportedEXT();
		void Shutdown();        
        bool SetupDebugMessenger();
        void ShowAvailableValidationLayers();
	private:
		VkInstance instance;
        VkApplicationInfo appInfo{};
        VkInstanceCreateInfo createInfo{};
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
	};
}