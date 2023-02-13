#include "ArchiveBuffer.h"
#include "Graphics/Graphics.h"

using namespace Seele;

ArchiveBuffer::ArchiveBuffer()
{}

ArchiveBuffer::ArchiveBuffer(Gfx::PGraphics graphics)
    : graphics(graphics)
{}

void ArchiveBuffer::writeBytes(const void* data, uint64 size)
{
    if (size + position >= memory.size())
    {
        memory.resize(size + position);
    }
    std::memcpy(memory.data() + position, data, size);
    position += size;
}

void ArchiveBuffer::readBytes(void* dest, uint64 size)
{
    assert(position + size <= memory.size());
    std::memcpy(dest, memory.data() + position, size);
    position += size;
}

void ArchiveBuffer::writeToStream(std::ostream& stream)
{
    uint64 bufferLength = memory.size();
    stream.write((char*)&version, sizeof(uint64));
    stream.write((char*)&bufferLength, sizeof(uint64));
    stream.write((char*)memory.data(), memory.size());
}

void ArchiveBuffer::readFromStream(std::istream& stream)
{
    stream.read((char*)&version, sizeof(uint64));
    uint64 bufferLength = 0;
    stream.read((char*)&bufferLength, sizeof(uint64));
    memory.resize(bufferLength);
    stream.read((char*)memory.data(), bufferLength);
}

bool ArchiveBuffer::eof() const
{
    return position == memory.size();
}

void ArchiveBuffer::rewind()
{
    position = 0;
}

Gfx::PGraphics& ArchiveBuffer::getGraphics()
{
    return graphics;
}
