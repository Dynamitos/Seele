#include "ArchiveBuffer.h"
#include "Graphics/Graphics.h"
#include <istream>
#include <ostream>


using namespace Seele;

ArchiveBuffer::ArchiveBuffer() {}

ArchiveBuffer::ArchiveBuffer(Gfx::PGraphics graphics) : graphics(graphics) {}

void ArchiveBuffer::writeBytes(const void* data, uint64 size) {
    if (size + position >= memory.size()) {
        memory.resize(size + position);
    }
    std::memcpy(memory.data() + position, data, size);
    position += size;
}

void ArchiveBuffer::readBytes(void* dest, uint64 size) {
    assert(position + size <= memory.size());
    std::memcpy(dest, memory.data() + position, size);
    position += size;
}

void ArchiveBuffer::writeToStream(std::ostream& stream) {
    uint64 bufferLength = memory.size();
    stream.write((char*)&version, sizeof(uint64));
    stream.write((char*)&bufferLength, sizeof(uint64));
    stream.write((char*)memory.data(), memory.size());
}

void ArchiveBuffer::readFromStream(std::istream& stream) {
    stream.read((char*)&version, sizeof(uint64));
    uint64 bufferLength = 0;
    stream.read((char*)&bufferLength, sizeof(uint64));
    memory.resize(bufferLength);
    stream.read((char*)memory.data(), bufferLength);
}

void ArchiveBuffer::seek(int64 s, SeekOp op) {
    int64 newPos = position;
    switch (op) {
    case SeekOp::BEGIN:
        newPos = s;
        break;
    case SeekOp::END:
        newPos = memory.size() + s;
        break;
    case SeekOp::CURRENT:
        newPos = position + s;
        break;
    }
    assert(newPos >= 0 && size_t(newPos) < memory.size());
    position = newPos;
}

bool ArchiveBuffer::eof() const { return position == memory.size(); }

size_t Seele::ArchiveBuffer::size() const { return memory.size(); }

void ArchiveBuffer::rewind() { position = 0; }

Gfx::PGraphics& ArchiveBuffer::getGraphics() { return graphics; }
