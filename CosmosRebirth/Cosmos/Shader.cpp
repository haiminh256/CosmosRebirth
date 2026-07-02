#include <Cosmos/Renderer/Shader.h>
#include <Cosmos/Core/Log.h>

namespace Cosmos {
    std::vector<char> Shader::readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            CORE_ERROR("failed to open file!");
        }
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }
    VkShaderModule Shader::CreateShaderModule(const std::vector<char>& code, VkDevice device) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            CORE_ERROR("failed to create shader module!");
        }

        return shaderModule;
    }
    void Shader::CreateGraphicsPipeline(VkDevice device, const std::string& vertexPath, const std::string& fragmentPath) {
        auto vertShaderCode = readFile(vertexPath);
        auto fragShaderCode = readFile(fragmentPath);

        vertShaderModule = CreateShaderModule(vertShaderCode, device);
        fragShaderModule = CreateShaderModule(fragShaderCode, device);
        CORE_INFO("Created graphicsPipeline");
    }
    VkShaderModule Shader::getVertShaderModule() {
        return vertShaderModule;
    }
    VkShaderModule Shader::getFragShaderModule() {
        return fragShaderModule;    
    }
    void Shader::CleanUp(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule) {
        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
        CORE_INFO("CleanUp shaderModule");
    }
    void Shader::CreateStage() {
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        CORE_INFO("Created a stage");
    }
}