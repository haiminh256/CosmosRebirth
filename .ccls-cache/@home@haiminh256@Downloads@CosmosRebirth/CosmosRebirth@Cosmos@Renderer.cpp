#include <Cosmos/Renderer/Renderer.h>
#include <Cosmos/Engine.h>
#include <Cosmos/Core/Log.h>

namespace {
    // Mảng hằng số Validation Layers toàn cục trong file cpp để tránh lỗi cảnh báo C4251
    const std::vector<const char*> ValidationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    // --- THÊM MỚI: Mảng lưu các Extension bắt buộc của phần cứng (để tạo Swapchain)
    const std::vector<const char*> DeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    // --- THÊM MỚI: Cấu trúc kiểm tra chỉ số hàng đợi (Queue Family Index)
    struct QueueFamilyIndices {
    
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    // --- THÊM MỚI: Hàm helper truy vấn nhóm hàng đợi từ GPU phần cứng
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            // Nếu dự án của bạn chưa tạo Surface, tạm thời ép true để quét được GPU
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

            if (presentSupport) {
                indices.presentFamily = i;
            }

            if (indices.isComplete()) break;
            i++;
        }
        return indices;
    }

    // --- THÊM MỚI: Hàm helper kiểm tra GPU có hỗ trợ các Extension cần thiết không
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        for (const char* required : DeviceExtensions) {
            bool found = false;
            for (const auto& available : availableExtensions) {
                if (strcmp(required, available.extensionName) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        return true;
    }
    // Hàm tạo Debug Messenger chuẩn hóa
    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* createInfo,
        const VkAllocationCallbacks* allocator,
        VkDebugUtilsMessengerEXT* messenger) {

        // Sử dụng reinterpret_cast chuẩn kèm theo định nghĩa macro của Vulkan
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkCreateDebugUtilsMessengerEXT"
        );

        if (func != nullptr) {
            return func(instance, createInfo, allocator, messenger);
        }
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    // Hàm hủy Debug Messenger chuẩn hóa (Ép kiểu hàm chi tiết)
    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT messenger,
        const VkAllocationCallbacks* allocator) {

        if (instance == VK_NULL_HANDLE || messenger == VK_NULL_HANDLE) {
            return;
        }

        // Ép kiểu tường minh theo chuẩn định nghĩa của Vulkan SDK dành cho DLL
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkDestroyDebugUtilsMessengerEXT"
        );

        if (func != nullptr) {
            func(instance, messenger, allocator);
        }
        else {
            // Log này sẽ cảnh báo nếu driver không cho phép nạp hàm hủy
            CORE_ERROR("Vulkan Driver Error: Cannot load vkDestroyDebugUtilsMessengerEXT function pointer!");
        }
    }
}

namespace Cosmos {

    // Gộp các hàm nhỏ lẻ lại để tránh lỗi giải phóng bộ nhớ sớm của Vector extensions
    void Renderer::Init() {
        // 1. Kiểm tra validation layers nếu được bật
        if (EnableValidationLayers && !CheckValidationLayerSupport()) {
            CORE_ERROR("Validation layers unavailable.");
            return;
        }

        // 2. Thiết lập thông tin App
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Vulkan App";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Cosmos Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        // 3. Thiết lập Instance Create Info
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // 4. Lấy Extensions (Giữ vector này sống trong suốt hàm Init)
        std::vector<const char*> extensions = GetRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // 5. Thiết lập Validation Layers và Debug Messenger lúc khởi tạo
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (EnableValidationLayers) {
            createInfo.enabledLayerCount = 0;

            // Bật debug ngay trong quá trình tạo/hủy Instance
            PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        // 6. Tạo Instance trực tiếp tại đây để đảm bảo con trỏ extension hợp lệ
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            CORE_ERROR("Failed to create VkInstance.");
            return;
        }

        // ĐÃ SỬA: Ngắt kết nối pNext ngay lập tức sau khi Instance được tạo thành công.
        // Điều này ngăn không cho biến Class createInfo giữ địa chỉ của debugCreateInfo (đã bị hủy khi ra khỏi hàm Init)
        createInfo.pNext = nullptr;

        CORE_INFO("Engine::InitVkInstance success");

        // 7. Thiết lập Debug Messenger cho các lỗi sau khi khởi tạo
    }

    void Renderer::Shutdown() {
        CORE_INFO("Renderer::Shutdown() started");

        // --- THAY ĐỔI: Phải hủy thiết bị logic con trước khi chạm vào Instance cha ---
        if (device != VK_NULL_HANDLE) {
            CORE_INFO("Destroying VkDevice...");
            vkDestroyDevice(device, nullptr);
            device = VK_NULL_HANDLE;
        }

        // ... Giữ nguyên phần dọn dẹp debugMessenger và instance cũ bên dưới ...
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
            // Gọi hàm wrapper tự tạo ở anonymous namespace đầu file
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

            // Quan trọng: Đặt lại giá trị về VK_NULL_HANDLE sau khi xóa thành công
            debugMessenger = VK_NULL_HANDLE;
        }
    }

    // ĐÃ HOÀN THIỆN: Hàm truy vấn và xuất log danh sách extensions hệ thống hỗ trợ
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
    // Thêm hàm này vào cuối file Renderer.cpp, ngay dưới hàm ShowSupportedEXT()
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

            // Hiện tại truyền VK_NULL_HANDLE cho surface, sau này khi bạn code Window, hãy truyền biến VkSurfaceKHR vào
            QueueFamilyIndices indices = findQueueFamilies(device, surface);
            bool extensionsSupported = checkDeviceExtensionSupport(device);

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

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

        // Dùng std::set để lọc trùng ID vì trên chip Intel 520, Graphics và Present thường chung ID = 0
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

        VkPhysicalDeviceFeatures deviceFeatures{}; // Tạm thời để cấu hình trống

        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
        deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();

        if (EnableValidationLayers) {
            deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
        }
        else {
            deviceCreateInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
            CORE_ERROR("Failed to create Logical Device!");
            return;
        }
        CORE_INFO("CosmosRebirth Create Logical Device success!");

        // Trích xuất Handle của luồng thực thi lệnh (Queue) ra biến Class để điều khiển render
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        CORE_INFO("Successfully extracted Graphics and Present Queues.");
    }
    void Renderer::createSurface(GLFWwindow* window) {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            CORE_ERROR("failed to create window surface!");
        }
    }
    void Renderer::CreatePresentationQueue() {
            QueueFamilyIndices indices =
            findQueueFamilies(physicalDevice, surface);

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
}
