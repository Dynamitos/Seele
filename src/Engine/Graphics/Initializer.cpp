#include "Initializer.h"

using namespace Seele;
using namespace Seele::Gfx;

LegacyPipelineCreateInfo::LegacyPipelineCreateInfo()
	: topology(Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
	, multisampleState(MultisampleState{
		.samples = 1,
		})
	, rasterizationState(RasterizationState{
		.polygonMode = Gfx::SE_POLYGON_MODE_FILL,
		.cullMode = Gfx::SE_CULL_MODE_BACK_BIT,
		.frontFace = Gfx::SE_FRONT_FACE_COUNTER_CLOCKWISE,
		})
	, depthStencilState(DepthStencilState{
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.stencilTestEnable = false,
		.depthCompareOp = Gfx::SE_COMPARE_OP_LESS_OR_EQUAL,
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f,
		})
	, colorBlend(ColorBlendState{
		.logicOpEnable = false,
		.attachmentCount = 0,
		.blendAttachments = {
			ColorBlendState::BlendAttachment{
				.colorWriteMask =
					Gfx::SE_COLOR_COMPONENT_R_BIT |
					Gfx::SE_COLOR_COMPONENT_G_BIT |
					Gfx::SE_COLOR_COMPONENT_B_BIT |
					Gfx::SE_COLOR_COMPONENT_A_BIT,
			}
		},
		.blendConstants = {
			1.0f,
			1.0f,
			1.0f,
			1.0f,
		},
		})
{
}

LegacyPipelineCreateInfo::~LegacyPipelineCreateInfo()
{
}


MeshPipelineCreateInfo::MeshPipelineCreateInfo()
	: multisampleState(MultisampleState{
		.samples = 1,
		})
	, rasterizationState(RasterizationState{
		.polygonMode = Gfx::SE_POLYGON_MODE_FILL,
		.cullMode = Gfx::SE_CULL_MODE_BACK_BIT,
		.frontFace = Gfx::SE_FRONT_FACE_COUNTER_CLOCKWISE,
		})
	, depthStencilState(DepthStencilState{
		.depthTestEnable = true,
		.depthWriteEnable = true,
		.stencilTestEnable = false,
		.depthCompareOp = Gfx::SE_COMPARE_OP_LESS_OR_EQUAL,
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f,
		})
	, colorBlend(ColorBlendState{
		.logicOpEnable = false,
		.attachmentCount = 0,
		.blendAttachments = {
			ColorBlendState::BlendAttachment{
				.colorWriteMask =
					Gfx::SE_COLOR_COMPONENT_R_BIT |
					Gfx::SE_COLOR_COMPONENT_G_BIT |
					Gfx::SE_COLOR_COMPONENT_B_BIT |
					Gfx::SE_COLOR_COMPONENT_A_BIT,
			}
		},
		.blendConstants = {
			1.0f,
			1.0f,
			1.0f,
			1.0f,
		},
		})
{
}

MeshPipelineCreateInfo::~MeshPipelineCreateInfo()
{
}
