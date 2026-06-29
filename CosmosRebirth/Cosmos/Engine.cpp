#include <Cosmos/Engine.h>
#include <Cosmos/Log.h>

namespace Cosmos {
	Engine::Engine() : window(nullptr) {

	}
	bool Engine::Init() {
		Logger::Init();
		if (!glfwInit()) {
			CORE_INFO("GLFW init failed");
			return false;
		}

		CORE_INFO("Engine::Init() success");
		return true;
	}
	void Engine::CreateAppWindow(int width, int height, const char* title) {
		window = glfwCreateWindow(width, height, title, nullptr, nullptr);
		if (window == nullptr) {
			CORE_ERROR("Engine::CreateAppWindow() failed");
		}
		CORE_INFO("Engine::CreateAppWindow() success");
	}
	void Engine::EventHandle() {
		glfwPollEvents();
	}
	void Engine::Quit() {
		CORE_INFO("Engine::Quit() success");
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();
	}
	void Engine::SetWindowHint(int hint, int value) {
		glfwWindowHint(hint, value);
	}
	void Engine::ApplyWindowHint() {
		glfwMakeContextCurrent(window);
	}
	void Engine::Terminate() {
		CORE_ERROR("Engine::Terminate() executed");
		glfwTerminate();
	}
	bool Engine::ShouldClose() {
		return glfwWindowShouldClose(window);
	}
	void Engine::InitVkInstance() {
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vulkan App";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Cosmos Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		CORE_INFO("Engine::InitVkInstance success");
	}
	void Engine::LinkVkInstance() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;

		CORE_INFO("Engine::LinkVkInstace() success");
	}
	void Engine::CreateVkInstance() {
		createInfo.enabledLayerCount = 0;
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			CORE_ERROR("Failed to create VkInstance");
		}
		CORE_INFO("Engine::CreateVkInstance() success");
	}
	void Engine::ShowSupportedEXT() {
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> extensions(extensionCount);

		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		for (const auto& extension : extensions) {
			CORE_INFO(extension.extensionName);
		}
	}
}