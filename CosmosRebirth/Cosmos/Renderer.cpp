#include <Cosmos/Renderer/Renderer.h>
#include <Cosmos/Engine.h>
#include <Cosmos/Core/Log.h>

namespace {
    // Mảng hằng số Validation Layers toàn cục trong file cpp để tránh lỗi cảnh báo C4251
    const std::vector<const char*> ValidationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    // --- THÊM MỚI: Mảng lưu các Extension bắt buộc của phần cứng (để tạo Swapchain)
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

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
            
            QueueFamilyIndices indices = Renderer::FindQueueFamilies(device);
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
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
        deviceCreateInfo.enabledLayerCount = 0;
        

        if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) {
            CORE_ERROR("Failed to create Logical Device!");
            return;
        }
        CORE_INFO("CosmosRebirth Create Logical Device success!");

        // Trích xuất Handle của luồng thực thi lệnh (Queue) ra biến Class để điều khiển render
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
    bool Renderer::isDeviceSuitable(VkPhysicalDevice device) {
        // Tìm các Queue Family của GPU
        QueueFamilyIndices indices = Renderer::FindQueueFamilies(device);

        // Kiểm tra GPU có hỗ trợ VK_KHR_swapchain không
        bool extensionsSupported =
            CheckDeviceExtensionSupport(device);

        bool swapChainAdequate = false;

        // Chỉ kiểm tra Swapchain nếu Extension đã tồn tại
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainSupport =
                QuerySwapChainSupport(device);

            swapChainAdequate =
                !swapChainSupport.formats.empty() &&
                !swapChainSupport.presentModes.empty();
        }

        // GPU chỉ hợp lệ khi đáp ứng đủ cả 3 điều kiện
        return indices.isComplete() &&
            extensionsSupported &&
            swapChainAdequate;

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
    Renderer::SwapChainSupportDetails Renderer::QuerySwapChainSupport(
        VkPhysicalDevice device)
    {
        // Struct sẽ chứa toàn bộ thông tin trả về.
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            device,
            surface,
            &details.capabilities
        );
        uint32_t formatCount = 0;

        vkGetPhysicalDeviceSurfaceFormatsKHR(
            device,
            surface,
            &formatCount,
            nullptr
        );

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);

            vkGetPhysicalDeviceSurfaceFormatsKHR(
                device,
                surface,
                &formatCount,
                details.formats.data()
            );
        }
        uint32_t presentModeCount = 0;

        vkGetPhysicalDeviceSurfacePresentModesKHR(
            device,
            surface,
            &presentModeCount,
            nullptr
        );

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);

            vkGetPhysicalDeviceSurfacePresentModesKHR(
                device,
                surface,
                &presentModeCount,
                details.presentModes.data()
            );
        }

        return details;
    }
    VkSurfaceFormatKHR Renderer::ChooseSwapSurfaceFormat(
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
    VkPresentModeKHR Renderer::ChooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& presentModes)
    {
        for (const auto& mode : presentModes)
        {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return mode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }
    VkExtent2D Renderer::ChooseSwapExtent(
        const VkSurfaceCapabilitiesKHR& capabilities,
        GLFWwindow* window)
    {
        // Window size đã được hệ điều hành quyết định.
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            return capabilities.currentExtent;
        }

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

        // Clamp theo giới hạn GPU.

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
    void Renderer::CreateSwapChain(GLFWwindow* window)
    {
        // ------------------------------------------
        // Lấy toàn bộ thông tin Swapchain
        // ------------------------------------------
        SwapChainSupportDetails support =
            QuerySwapChainSupport(physicalDevice);

        // ------------------------------------------
        // Chọn Format
        // ------------------------------------------
        VkSurfaceFormatKHR surfaceFormat =
            ChooseSwapSurfaceFormat(support.formats);

        // ------------------------------------------
        // Chọn Present Mode
        // ------------------------------------------
        VkPresentModeKHR presentMode =
            ChooseSwapPresentMode(support.presentModes);
        VkExtent2D extent =
            ChooseSwapExtent(support.capabilities, window);
        uint32_t imageCount =
            support.capabilities.minImageCount + 1;

        if (support.capabilities.maxImageCount > 0 &&
            imageCount > support.capabilities.maxImageCount)
        {
            imageCount =
                support.capabilities.maxImageCount;
        }

        // ------------------------------------------
        // Tạo Swapchain
        // ------------------------------------------
        VkSwapchainCreateInfoKHR createInfo{};

        createInfo.sType =
            VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

        createInfo.surface = surface;

        createInfo.minImageCount = imageCount;

        createInfo.imageFormat =
            surfaceFormat.format;

        createInfo.imageColorSpace =
            surfaceFormat.colorSpace;

        createInfo.imageExtent =
            extent;

        createInfo.imageArrayLayers = 1;

        // Image sẽ dùng làm Color Attachment.
        createInfo.imageUsage =
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        //------------------------------------------
        // Queue Sharing Mode
        //------------------------------------------

        QueueFamilyIndices indices =
            Renderer::FindQueueFamilies(physicalDevice);

        uint32_t queueFamilyIndices[] =
        {
            indices.graphicsFamily.value(),
            indices.presentFamily.value()
        };

        // Graphics Queue và Present Queue khác nhau.
        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode =
                VK_SHARING_MODE_CONCURRENT;

            createInfo.queueFamilyIndexCount = 2;

            createInfo.pQueueFamilyIndices =
                queueFamilyIndices;
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

        createInfo.clipped =
            VK_TRUE;

        createInfo.oldSwapchain =
            VK_NULL_HANDLE;

        //------------------------------------------
        // Tạo Swapchain
        //------------------------------------------

        if (vkCreateSwapchainKHR(
            device,
            &createInfo,
            nullptr,
            &swapChain) != VK_SUCCESS)
        {
            CORE_ERROR("Failed to create Swapchain.");
            return;
        }

        //------------------------------------------
        // Lấy Image của Swapchain
        //------------------------------------------

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

        CORE_INFO("Renderer::CreateSwapChain() success");
    }
    void Renderer::CreateImageViews()
    {
        swapChainImageViews.resize(
            swapChainImages.size());

        for (size_t i = 0;
            i < swapChainImages.size();
            i++)
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
                device,
                &createInfo,
                nullptr,
                &swapChainImageViews[i]) != VK_SUCCESS)
            {
                CORE_ERROR("Failed to create ImageView.");
                return;
            }
        }

        CORE_INFO("Renderer::CreateImageViews() success");
    }
    Renderer::QueueFamilyIndices
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
}