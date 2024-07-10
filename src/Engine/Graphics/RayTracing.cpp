#include "RayTracing.h"
#include "RayTracing.h"

using namespace Seele::Gfx;

RayTracingPipeline::RayTracingPipeline(Gfx::PPipelineLayout layout) : layout(layout) {}

RayTracingPipeline::~RayTracingPipeline() {}

PPipelineLayout RayTracingPipeline::getPipelineLayout() const { return layout; }

BottomLevelAS::BottomLevelAS() {}

BottomLevelAS::~BottomLevelAS() {}

TopLevelAS::TopLevelAS() {}

TopLevelAS::~TopLevelAS() {}