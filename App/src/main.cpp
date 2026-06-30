#include <Cosmos/Engine.h>
#include <Cosmos/Core/Log.h>
#include <Cosmos/Renderer/Renderer.h>

int main() {
	Cosmos::Engine App;
	Cosmos::Renderer Renderer;
	if (!App.Init()) {
		App.Terminate();
	}
	App.SetWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	App.SetWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	App.CreateAppWindow(800, 600, "Engine Window");

	Renderer.Init();
	Renderer.ShowSupportedEXT();
	Renderer.ShowAvailableValidationLayers();

	while (!App.ShouldClose()) {
		App.EventHandle();
	}

	Renderer.Shutdown();
	App.Quit();
}