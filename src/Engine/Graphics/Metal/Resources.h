#pragma once
#include "Graphics/Initializer.h"
#include "Graphics/Resources.h"
#include "Containers/List.h"
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>

namespace Seele {
namespace Metal {
DECLARE_REF(Graphics);

DECLARE_REF(CommandBoundResource)
class DestructionManager {
  public:
    DestructionManager(PGraphics graphics);
    ~DestructionManager();
    void queueResourceForDestruction(OCommandBoundResource resource);
    void notifyCommandComplete();

  private:
    PGraphics graphics;
    Array<OCommandBoundResource> resources; // todo: maybe list?
};
DEFINE_REF(DestructionManager)

class CommandBoundResource {
  public:
    CommandBoundResource(PGraphics graphics) : graphics(graphics) {}
    virtual ~CommandBoundResource() {
        if (isCurrentlyBound())
            abort();
    }
    constexpr bool isCurrentlyBound() const { return bindCount > 0; }
    constexpr void bind() { bindCount++; }
    constexpr void unbind() { bindCount--; }

  protected:
    PGraphics graphics;
    uint64 bindCount = 0;
};
DEFINE_REF(CommandBoundResource)

class Fence {
  public:
    Fence(PGraphics graphics);
    ~Fence();
    MTL::Fence* getHandle() const { return handle; }

  private:
    MTL::Fence* handle;
};
DEFINE_REF(Fence);
class Event {
  public:
    Event(PGraphics graphics);
    ~Event();
    MTL::Event* getHandle() const { return handle; }

  private:
    MTL::Event* handle;
};
DEFINE_REF(Event)
class Sampler : public Gfx::Sampler {
  public:
    Sampler(PGraphics graphics, const SamplerCreateInfo& createInfo);
    virtual ~Sampler();
    MTL::SamplerState* getHandle() const { return sampler; }

  private:
    MTL::SamplerState* sampler;
};
DEFINE_REF(Sampler)
} // namespace Metal
} // namespace Seele
