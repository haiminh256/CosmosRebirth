#pragma once
#include <Cosmos/EngineEnv.h>

namespace Cosmos {
	class COSMOS_API Engine {
	public:
		Engine();
		bool Init();
		void CreateAppWindow(int width, int height, const char* title);
		void EventHandle();
		void Quit();
		void SetWindowHint(int hint, int value);
		void ApplyWindowHint();
		void Terminate();
		bool ShouldClose();
		void InitVkInstance();
	private:
		GLFWwindow* window;
	};
}