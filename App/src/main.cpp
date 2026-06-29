#include <Cosmos/Engine.h>

int main() {
	Cosmos::Engine App;
	if (!App.Init()) {
		std::cout << "App init failed" << std::endl;
		App.Terminate();
	}
	App.SetWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	App.SetWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	App.CreateAppWindow(800, 600, "Engine Window");

	App.InitVkInstance();
	App.LinkVkInstance();
	App.CreateVkInstance();
	App.ShowSupportedEXT();

	while (!App.ShouldClose()) {
		App.EventHandle();
	}
	App.Quit();
}