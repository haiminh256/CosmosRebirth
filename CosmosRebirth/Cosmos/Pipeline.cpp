#include <Cosmos/Renderer/Pipeline.h>
#include <Cosmos/EngineEnv.h>
#include <Cosmos/Renderer/Swapchain.h>
#include <Cosmos/Renderer/Shader.h>
#include <Cosmos/Core/Log.h>

namespace Cosmos {
	void Pipeline::CreatePipeline(VkDevice device, Swapchain swapchain) {
		this->device = device;

		CreateVertexInput();
		CreateInputAssembly();
		CreateViewportState(
			swapchain.GetExtent());

		CreateRasterizer();
		CreateMultisampling();
		CreateColorBlending();
		CreateDynamicState();
		CreatePipelineLayout();
	}
	void Pipeline::CreateVertexInput()
	{
		vertexInput.sType =
			VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		vertexInput.vertexBindingDescriptionCount = 0;
		vertexInput.vertexAttributeDescriptionCount = 0;
	}
	void Pipeline::CreateInputAssembly()
	{
		inputAssembly.sType =
			VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

		inputAssembly.topology =
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		inputAssembly.primitiveRestartEnable =
			VK_FALSE;
	}
	void Pipeline::CreateViewportState(
		VkExtent2D extent)
	{
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width =
			static_cast<float>(extent.width);
		viewport.height =
			static_cast<float>(extent.height);

		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		scissor.offset = { 0,0 };
		scissor.extent = extent;

		viewportState.sType =
			VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;

		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;
	}
	void Pipeline::CreateMultisampling()
	{
		multisampling.sType =
			VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

		multisampling.sampleShadingEnable =
			VK_FALSE;

		multisampling.rasterizationSamples =
			VK_SAMPLE_COUNT_1_BIT;

		multisampling.minSampleShading =
			1.0f;

		multisampling.pSampleMask =
			nullptr;

		multisampling.alphaToCoverageEnable =
			VK_FALSE;

		multisampling.alphaToOneEnable =
			VK_FALSE;
	}
	void Pipeline::CreateRasterizer()
	{
		rasterizer.sType =
			VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

		rasterizer.depthClampEnable =
			VK_FALSE;

		rasterizer.rasterizerDiscardEnable =
			VK_FALSE;

		rasterizer.polygonMode =
			VK_POLYGON_MODE_FILL;

		rasterizer.lineWidth =
			1.0f;

		rasterizer.cullMode =
			VK_CULL_MODE_BACK_BIT;

		rasterizer.frontFace =
			VK_FRONT_FACE_CLOCKWISE;

		rasterizer.depthBiasEnable =
			VK_FALSE;

		rasterizer.depthBiasConstantFactor =
			0.0f;

		rasterizer.depthBiasClamp =
			0.0f;

		rasterizer.depthBiasSlopeFactor =
			0.0f;
	}
	void Pipeline::CreateColorBlending()
	{
		colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;

		colorBlendAttachment.blendEnable =
			VK_FALSE;

		colorBlendAttachment.srcColorBlendFactor =
			VK_BLEND_FACTOR_ONE;

		colorBlendAttachment.dstColorBlendFactor =
			VK_BLEND_FACTOR_ZERO;

		colorBlendAttachment.colorBlendOp =
			VK_BLEND_OP_ADD;

		colorBlendAttachment.srcAlphaBlendFactor =
			VK_BLEND_FACTOR_ONE;

		colorBlendAttachment.dstAlphaBlendFactor =
			VK_BLEND_FACTOR_ZERO;

		colorBlendAttachment.alphaBlendOp =
			VK_BLEND_OP_ADD;

		colorBlending.sType =
			VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

		colorBlending.logicOpEnable =
			VK_FALSE;

		colorBlending.logicOp =
			VK_LOGIC_OP_COPY;

		colorBlending.attachmentCount =
			1;

		colorBlending.pAttachments =
			&colorBlendAttachment;

		colorBlending.blendConstants[0] =
			0.0f;

		colorBlending.blendConstants[1] =
			0.0f;

		colorBlending.blendConstants[2] =
			0.0f;

		colorBlending.blendConstants[3] =
			0.0f;
	}
	void Pipeline::CreateDynamicState()
	{
		dynamicStates =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		dynamicState.sType =
			VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

		dynamicState.dynamicStateCount =
			static_cast<uint32_t>(
				dynamicStates.size());

		dynamicState.pDynamicStates =
			dynamicStates.data();
	}
	void Pipeline::CreatePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

		pipelineLayoutInfo.sType =
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

		pipelineLayoutInfo.setLayoutCount =
			0;

		pipelineLayoutInfo.pSetLayouts =
			nullptr;

		pipelineLayoutInfo.pushConstantRangeCount =
			0;

		pipelineLayoutInfo.pPushConstantRanges =
			nullptr;

		if (vkCreatePipelineLayout(
			device,
			&pipelineLayoutInfo,
			nullptr,
			&pipelineLayout)
			!= VK_SUCCESS)
		{
			CORE_ERROR("Failed to create Pipeline Layout.");
		}
	}
	void Pipeline::DestroyPipeline()
	{
		if (pipelineLayout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(
				device,
				pipelineLayout,
				nullptr);

			pipelineLayout =
				VK_NULL_HANDLE;
		}
	}
}
