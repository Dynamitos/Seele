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
		break;
	default:
		break;
	}
}
IndexBuffer::~IndexBuffer()
{
}
UniformBuffer::UniformBuffer(QueueFamilyMapping mapping, const DataSource& sourceData)
	: Buffer(mapping, sourceData.owner)
	, contents(sourceData.size)
{
	if (sourceData.data != nullptr)
	{
		std::memcpy(contents.data(), sourceData.data, contents.size());
	}
}

UniformBuffer::~UniformBuffer()
{
}

bool UniformBuffer::updateContents(const DataSource& sourceData)
{
	assert(contents.size() == sourceData.size);
	if (std::memcmp(contents.data(), sourceData.data, contents.size()) == 0)
	{
		return false;
	}
	std::memcpy(contents.data(), sourceData.data, contents.size());
	return true;
}

ShaderBuffer::ShaderBuffer(QueueFamilyMapping mapping, uint32 numElements, const DataSource& sourceData)
	: Buffer(mapping, sourceData.owner)
	, contents(sourceData.size)
	, numElements(numElements)
{
	if (sourceData.data != nullptr)
	{
		std::memcpy(contents.data(), sourceData.data, sourceData.size);
	}
}

ShaderBuffer::~ShaderBuffer()
{
}

bool ShaderBuffer::updateContents(const DataSource& sourceData)
{
	assert(contents.size() >= sourceData.size);
	if (std::memcmp(contents.data(), sourceData.data, sourceData.size) == 0)
	{
		return false;
	}
	std::memcpy(contents.data(), sourceData.data, sourceData.size);
	return true;
}
