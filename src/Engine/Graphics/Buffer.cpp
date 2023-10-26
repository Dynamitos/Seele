#include "Buffer.h"

using namespace Seele;
using namespace Seele::Gfx;

Buffer::Buffer(QueueFamilyMapping mapping, QueueType startQueue)
	: QueueOwnedResource(mapping, startQueue)
{
}

Buffer::~Buffer()
{
}

UniformBuffer::UniformBuffer(QueueFamilyMapping mapping, const BulkResourceData& resourceData)
	: Buffer(mapping, resourceData.owner)
	, contents(resourceData.size)
{
	if (resourceData.data != nullptr)
	{
		std::memcpy(contents.data(), resourceData.data, contents.size());
	}
}

UniformBuffer::~UniformBuffer()
{
}

bool UniformBuffer::updateContents(const BulkResourceData& resourceData)
{
	assert(contents.size() == resourceData.size);
	if (std::memcmp(contents.data(), resourceData.data, contents.size()) == 0)
	{
		return false;
	}
	std::memcpy(contents.data(), resourceData.data, contents.size());
	return true;
}

ShaderBuffer::ShaderBuffer(QueueFamilyMapping mapping, uint32 stride, uint32 numElements, const BulkResourceData& resourceData)
	: Buffer(mapping, resourceData.owner)
	, contents(resourceData.size)
	, stride(stride)
	, numElements(numElements)
{
	if (resourceData.data != nullptr)
	{
		std::memcpy(contents.data(), resourceData.data, resourceData.size);
	}
}
ShaderBuffer::~ShaderBuffer()
{
}

bool ShaderBuffer::updateContents(const BulkResourceData& resourceData)
{
	assert(contents.size() >= resourceData.size);
	if (std::memcmp(contents.data(), resourceData.data, resourceData.size) == 0)
	{
		return false;
	}
	std::memcpy(contents.data(), resourceData.data, resourceData.size);
	return true;
}
VertexBuffer::VertexBuffer(QueueFamilyMapping mapping, uint32 numVertices, uint32 vertexSize, QueueType startQueueType)
	: Buffer(mapping, startQueueType)
	, numVertices(numVertices)
	, vertexSize(vertexSize)
{
}
VertexBuffer::~VertexBuffer()
{
}
IndexBuffer::IndexBuffer(QueueFamilyMapping mapping, uint64 size, Gfx::SeIndexType indexType, QueueType startQueueType)
	: Buffer(mapping, startQueueType)
	, indexType(indexType)
{
	switch (indexType)
	{
	case SE_INDEX_TYPE_UINT16:
		numIndices = size / sizeof(uint16);
		break;
	case SE_INDEX_TYPE_UINT32:
		numIndices = size / sizeof(uint32);
	default:
		break;
	}
}
IndexBuffer::~IndexBuffer()
{
}