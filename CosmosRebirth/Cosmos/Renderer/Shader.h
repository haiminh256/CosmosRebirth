#pragma once
#include <Cosmos/EngineEnv.h>

namespace Cosmos {
	class COSMOS_API Shader {
	public:
		void CreateGraphicsPipeline(VkDevice device, const std::string& vertexPath, const std::string& fragmentPath);
		void CleanUp(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule);
		void CreateStage();
		VkShaderModule getVertShaderModule();
		VkShaderModule getFragShaderModule();
	private:
		std::vector<char> readFile(const std::string& filename);
		VkShaderModule CreateShaderModule(const std::vector<char>& code, VkDevice device);
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;
	};
}