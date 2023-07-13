#pragma once
#include "MinimalEngine.h"
#include "Containers/Array.h"
#include "Concepts.h"
#include <string>

namespace Seele
{
DECLARE_NAME_REF(Gfx, Graphics)
class ArchiveBuffer
{
public:
    ArchiveBuffer();
    ArchiveBuffer(Gfx::PGraphics graphics);
    void writeBytes(const void* data, uint64 size);
    void readBytes(void* dest, uint64 size);
    void writeToStream(std::ostream& stream);
    void readFromStream(std::istream& stream);
    enum class SeekOp
    {
        CURRENT,
        BEGIN,
        END,
    };
    void seek(int64 s, SeekOp op);
    bool eof() const;
    size_t size() const;
    void rewind();
    Gfx::PGraphics& getGraphics();
private:
    Gfx::PGraphics graphics;
    uint64 version = 0;
    uint64 position = 0;
    Array<uint8> memory;
};
} // namespace Seele