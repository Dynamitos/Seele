#include "Resources.h"
#include "Command.h"
#include "Enums.h"
#include "Graphics.h"
#include "Window.h"

using namespace Seele;
using namespace Seele::Vulkan;

SemaphoreHandle::SemaphoreHandle(PGraphics graphics, const std::string& name) : CommandBoundResource(graphics, name) {
    VkSemaphoreCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };
    VK_CHECK(vkCreateSemaphore(graphics->getDevice(), &info, nullptr, &handle));
}

SemaphoreHandle::~SemaphoreHandle() { vkDestroySemaphore(graphics->getDevice(), handle, nullptr); }

Semaphore::Semaphore(PGraphics graphics) : graphics(graphics) {}

Semaphore::~Semaphore() {
    for (auto& h : handles) {
        graphics->getDestructionManager()->queueResourceForDestruction(std::move(h));
    }
}

void Semaphore::rotateSemaphore() {
    for (uint32 i = 0; i < handles.size(); ++i) {
        if (handles[i]->isCurrentlyBound()) {
            continue;
        }
        currentHandle = i;
        return;
    }
    currentHandle = (uint32)handles.size();
    handles.add(new SemaphoreHandle(graphics, "Semaphore"));
}

Fence::Fence(PGraphics graphics) : graphics(graphics), status(Status::Ready) {
    VkFenceCreateInfo info = {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .pNext = nullptr, .flags = 0};
    VK_CHECK(vkCreateFence(graphics->getDevice(), &info, nullptr, &fence));
}

Fence::~Fence() {
    vkWaitForFences(graphics->getDevice(), 1, &fence, true, 10000000);
    vkDestroyFence(graphics->getDevice(), fence, nullptr);
}

void Fence::submit() { status = Status::InUse; }

bool Fence::isSignaled() {
    if (status == Status::Signalled) {
        return true;
    }
    VkResult r = vkGetFenceStatus(graphics->getDevice(), fence);
    if (r == VK_SUCCESS) {
        status = Status::Signalled;
        return true;
    }
    if (r == VK_NOT_READY) {
        return false;
    } else {
        VK_CHECK(r);
        return false;
    }
}

void Fence::reset() {
    while (status == Status::InUse) {
        wait(1000000);
    }
    if (status == Status::Signalled) {
        VK_CHECK(vkResetFences(graphics->getDevice(), 1, &fence));
        status = Status::Ready;
    }
}

void Fence::wait(uint64 timeout) {
    VkFence fences[] = {fence};
    VkResult r = vkWaitForFences(graphics->getDevice(), 1, fences, true, timeout);
    if (r == VK_SUCCESS) {
        status = Status::Signalled;
    } else if (r == VK_TIMEOUT) {
        return;
    } else if (r != VK_NOT_READY) {
        VK_CHECK(r);
    }
}

DestructionManager::DestructionManager(PGraphics graphics) : graphics(graphics) {}

DestructionManager::~DestructionManager() {}

void DestructionManager::queueResourceForDestruction(OCommandBoundResource resource) {
    if (resource == nullptr)
        return;
    if (resource->isCurrentlyBound()) {
        resources.add(std::move(resource));
    }
}

void DestructionManager::notifyCommandComplete() {
    for (size_t i = 0; i < resources.size(); ++i) {
        if (!resources[i]->isCurrentlyBound()) {
            resources.removeAt(i, false);
            i--;
        }
    }
}

SamplerHandle::SamplerHandle(PGraphics graphics, SamplerCreateInfo createInfo) : CommandBoundResource(graphics, "Sampler") {
    VkSamplerCreateInfo vkInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = createInfo.flags,
        .magFilter = cast(createInfo.magFilter),
        .minFilter = cast(createInfo.minFilter),
        .mipmapMode = cast(createInfo.mipmapMode),
        .addressModeU = cast(createInfo.addressModeU),
        .addressModeV = cast(createInfo.addressModeV),
        .addressModeW = cast(createInfo.addressModeW),
        .mipLodBias = createInfo.mipLodBias,
        .anisotropyEnable = createInfo.anisotropyEnable,
        .maxAnisotropy = createInfo.maxAnisotropy,
        .compareEnable = createInfo.compareEnable,
        .compareOp = cast(createInfo.compareOp),
        .minLod = createInfo.minLod,
        .maxLod = createInfo.maxLod,
        .borderColor = cast(createInfo.borderColor),
        .unnormalizedCoordinates = createInfo.unnormalizedCoordinates,
    };
    vkCreateSampler(graphics->getDevice(), &vkInfo, nullptr, &sampler);
}

SamplerHandle::~SamplerHandle() { vkDestroySampler(graphics->getDevice(), sampler, nullptr); }

Sampler::Sampler(PGraphics graphics, SamplerCreateInfo createInfo)
    : Gfx::Sampler(createInfo), graphics(graphics), handle(new SamplerHandle(graphics, createInfo)) {}

Sampler::~Sampler() { graphics->getDestructionManager()->queueResourceForDestruction(std::move(handle)); }
