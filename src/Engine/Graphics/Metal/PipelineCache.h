#pragma once
#include "Graphics/Initializer.h"
#include "Pipeline.h"

namespace Seele {
namespace Metal {
class PipelineCache
{
public:
  PipelineCache(PGraphics graphics, const std::string& cacheFilePath);
  ~PipelineCache();
  PGraphicsPipeline createPipeline(Gfx::LegacyPipelineCreateInfo createInfo);
  PGraphicsPipeline createPipeline(Gfx::MeshPipelineCreateInfo createInfo);
  PComputePipeline createPipeline(Gfx::ComputePipelineCreateInfo createInfo);
private:
  PGraphics graphics;
  Map<uint32, OGraphicsPipeline> graphicsPipelines;
  Map<uint32, OComputePipeline> computePipelines;
  std::string cacheFile;
};
DEFINE_REF(PipelineCache)
}
}