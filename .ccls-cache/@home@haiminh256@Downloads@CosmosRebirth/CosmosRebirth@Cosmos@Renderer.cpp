#include <Cosmos/Renderer/Renderer.h>
#include <Cosmos/Engine.h>
#include <Cosmos/Core/Log.h>
#include <Cosmos/Renderer/Swapchain.h>

namespace {
    const std::vector<const char*> ValidationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* createInfo,
        const VkAllocationCallbacks* allocator,
        VkDebugUtilsMessengerEXT* messenger) {

        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkCreateDebugUtilsMessengerEXT"
        );

        if (func != nullptr) {
            return func(instance, createInfo, allocator, messenger);
        }
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT messenger,
        const VkAllocationCallbacks* allocator) {

        if (instance == VK_NULL_HANDLE || messenger == VK_NULL_HANDLE) {
            return;
        }

        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkDestroyDebugUtilsMessengerEXT"
        );

        if (func != nullptr) {
            func(instance, messenger, allocator);
        }
        else {
            CORE_ERROR("Vulkan Driver Error: Cannot load vkDestroyDebugUtilsMessengerEXT function pointer!");
        }
    }
}

namespace Cosmos {

    void Renderer::Init() {
        if (EnableValidationLayers && !CheckValidationLayerSupport()) {
            CORE_ERROR("Validation layers unavailable.");
            return;
        }

        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan App";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Cosmos Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        std::vector<const char*> extensions = GetRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (EnableValidationLayers) {
            createInfo.enabledLayerCount = 0;

            PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            CORE_ERROR("Failed to create VkInstance.");
            return;
        }

        createInfo.pNext = nullptr;

        CORE_INFO("Engine::InitVkInstance success");
    }

    void Renderer::Shutdown() {
        CORE_INFO("Renderer::Shutdown() started");

        if (debugMessenger != VK_NULL_HANDLE) {
            CORE_INFO("Destroying Debug Messenger...");
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
            debugMessenger = VK_NULL_HANDLE;
        }

        createInfo.pNext = nullptr;
        createInfo.enabledLayerCount = 0;

        if (surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(instance, surface, nullptr);
            surface = VK_NULL_HANDLE;
        }
        if (instance != VK_NULL_HANDLE) {
            CORE_INFO("Destroying VkInstance...");
            vkDestroyInstance(instance, nullptr);
            instance = VK_NULL_HANDLE;
        }

        CORE_INFO("Renderer::Shutdown() finished");
    }


    bool Renderer::CheckValidationLayerSupport() {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> layers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

        for (const char* layerName : ValidationLayers) {
            bool found = false;
            for (const auto& layer : layers) {
                if (strcmp(layer.layerName, layerName) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        return true;
    }

    std::vector<const char*> Renderer::GetRequiredExtensions() {
        uint32_t count = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&count);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + count);

        if (EnableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }
        return extensions;
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void*) {

        if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            CORE_ERROR(callbackData->pMessage);
        }
        else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            CORE_WARN(callbackData->pMessage);
        }
        else {
            CORE_INFO(callbackData->pMessage);
        }
        return VK_FALSE;
    }

    void Renderer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
    }

    bool Renderer::SetupDebugMessenger() {
        if (!EnableValidationLayers) return true;

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        PopulateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            CORE_ERROR("Failed to create Debug Messenger.");
            return false;
        }
        return true;
    }

    void Renderer::DestroyDebugMessenger() {
        if (EnableValidationLayers && debugMessenger != VK_NULL_HANDLE) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

            debugMessenger = VK_NULL_HANDLE;
        }
    }
    void Renderer::ShowSupportedEXT() {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        CORE_INFO("Supported Vulkan Extensions:");
        for (const auto& extension : extensions) {
            CORE_INFO(" - {0} (Spec Version: {1})", extension.extensionName, extension.specVersion);
        }
    }
    void Renderer::ShowAvailableValidationLayers() {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        CORE_INFO("Available Vulkan Validation Layers:");
        for (const auto& layer : availableLayers) {
            CORE_INFO(" - {0} (Version: {1})", layer.layerName, layer.implementationVersion);
        }
    }
    void Renderer::PickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            CORE_ERROR("Failed to find GPUs with Vulkan support!");
            return;
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);

            indices = Renderer::FindQueueFamilies(device);
            bool extensionsSupported = CheckDeviceExtensionSupport(device);

            if (indices.isComplete() && extensionsSupported) {
                physicalDevice = device;
                CORE_INFO("CosmosRebirth selected Physical Device: {0}", deviceProperties.deviceName);
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) {
            CORE_ERROR("Failed to find a suitable GPU!");
        }
    }
    void Renderer::CreateLogicalDevice() {
        if (physicalDevice == VK_NULL_HANDLE) return;

        QueueFamilyIndices indices = Renderer::FindQueueFamilies(physicalDevice);
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        std::set<uint32_t> uniqueQueueFamilies = {
            indices.graphicsFamily.value(),
            indices.presentFamily.value()
        };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        deviceCreateInfo.enabledLayerCount = 0;
        

        if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
            CORE_ERROR("Failed to create Logical Device!");
            return;
        }
        CORE_INFO("CosmosRebirth Create Logical Device success!");

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        CORE_INFO("Successfully extracted Graphics and Present Queues.");
    }
    void Renderer::CreateSurface(GLFWwindow* window) {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            CORE_ERROR("failed to create window surface!");
        }
    }
    void Renderer::CreatePresentationQueue() {
            QueueFamilyIndices indices =
            Renderer::FindQueueFamilies(physicalDevice);

        if (!indices.presentFamily.has_value())
        {
            CORE_ERROR("Present queue not found.");
            return;
        }

        vkGetDeviceQueue(
            device,
            indices.presentFamily.value(),
            0,
            &presentQueue
        );

        CORE_INFO(
            "Presentation Queue Family = {}",
            indices.presentFamily.value()
        );
    }
    bool Renderer::isDeviceSuitable(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices = FindQueueFamilies(device);

        bool extensionsSupported =
            CheckDeviceExtensionSupport(device);

        bool swapChainAdequate = false;

        if (extensionsSupported)
        {
            auto swapChainSupport =
                QuerySwapChainSupport(
                    device,
                    surface);

            swapChainAdequate =
                !swapChainSupport.formats.empty() &&
                !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures{};
        vkGetPhysicalDeviceFeatures(
            device,
            &supportedFeatures);

        return
            indices.isComplete() &&
            extensionsSupported &&
            swapChainAdequate &&
            supportedFeatures.samplerAnisotropy;
    }

    bool Renderer::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }
    QueueFamilyIndices
        Renderer::FindQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(
            device,
            &queueFamilyCount,
            nullptr
        );

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);

        vkGetPhysicalDeviceQueueFamilyProperties(
            device,
            &queueFamilyCount,
            queueFamilies.data()
        );

        uint32_t i = 0;

        for (const auto& queueFamily : queueFamilies)
        {
            // Graphics Queue
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }

            // Present Queue
            VkBool32 presentSupport = VK_FALSE;

            vkGetPhysicalDeviceSurfaceSupportKHR(
                device,
                i,
                surface,
                &presentSupport
            );

            if (presentSupport)
            {
                indices.presentFamily = i;
            }

            if (indices.isComplete())
            {
                break;
            }

            ++i;
        }

        return indices;
    }
    VkDevice Renderer::getVkDevice() {
        return device;
    }
    SwapChainSupportDetails Renderer::QuerySwapChainSupport(VkPhysicalDevice physicalDevice,VkSurfaceKHR surface) {
        SwapChainSupportDetails details;

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
    VkPhysicalDevice Renderer::getPhysicalDevice() {
        return physicalDevice;
    }
    VkQueue Renderer::getGraphicsQueue() {
        return graphicsQueue;
    }
    VkQueue Renderer::getPresentQueue() {
        return presentQueue;
    }
    VkSurfaceKHR Renderer::getSurface() {
        return surface;
    }
    QueueFamilyIndices Renderer::getQueueFamilyIndices() {
        return indices;
    }
}