#pragma once

#include <Cosmos/EngineEnv.h>
#include <Cosmos/Renderer/Swapchain.h>

namespace Cosmos
{
    class COSMOS_API Pipeline
    {
    public:
        void CreatePipeline(VkDevice device, Swapchain swapchain);
        void DestroyPipeline();

    private:
        void CreateVertexInput();
        void CreateInputAssembly();
        void CreateViewportState(VkExtent2D extent);
        void CreateRasterizer();
        void CreateMultisampling();
        void CreateColorBlending();
        void CreateDynamicState();
        void CreatePipelineLayout();

    private:
        VkDevice device = VK_NULL_HANDLE;

        VkPipeline graphicsPipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

        VkPipelineVertexInputStateCreateInfo vertexInput{};
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        VkPipelineViewportStateCreateInfo viewportState{};
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        VkPipelineMultisampleStateCreateInfo multisampling{};
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        VkPipelineDynamicStateCreateInfo dynamicState{};

        VkViewport viewport{};
        VkRect2D scissor{};

        std::vector<VkDynamicState> dynamicStates;
    };
}
