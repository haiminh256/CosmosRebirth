#include <Cosmos/Engine.h>
#include <Cosmos/Core/Log.h>
#include <Cosmos/Renderer/Renderer.h>
#include <Cosmos/Renderer/Shader.h>
#include <Cosmos/Renderer/Swapchain.h>
#include <Cosmos/Renderer/Pipeline.h>

int main() {
	Cosmos::Engine App;
	Cosmos::Renderer Renderer;
	Cosmos::Shader Shader;
	Cosmos::Swapchain Swapchain;
	Cosmos::Pipeline Pipeline;

	if (!App.Init()) {
		App.Terminate();
	}
	App.SetWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	App.SetWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	App.CreateAppWindow(800, 600, "Engine Window");


    Renderer.Init();
    Renderer.ShowSupportedEXT();
    Renderer.ShowAvailableValidationLayers();
    Renderer.SetupDebugMessenger();

    Renderer.CreateSurface(App.getWindow());

    Renderer.PickPhysicalDevice();
    Renderer.CreateLogicalDevice();
    Renderer.CreatePresentationQueue();
	Swapchain.CreateSwapchain(Renderer.getPhysicalDevice(), Renderer.getVkDevice(), Renderer.getSurface(), App.getWindow(), Renderer.getQueueFamilyIndices());
	Swapchain.CreateImageViews();

	Pipeline.CreatePipeline(Renderer.getVkDevice(), Swapchain);

	Shader.CreateGraphicsPipeline(Renderer.getVkDevice(), "Shader/vert.spv", "Shader/frag.spv");
	Shader.CreateStage();

	while (!App.ShouldClose()) {
		App.EventHandle();
	}

	Pipeline.DestroyPipeline();
	Shader.CleanUp(Renderer.getVkDevice(), Shader.getVertShaderModule(), Shader.getFragShaderModule());
	Renderer.Shutdown();
	App.Quit();
}
