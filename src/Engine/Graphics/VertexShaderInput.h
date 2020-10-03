#pragma once
#include "MinimalEngine.h"
#include "GraphicsEnums.h"
#include "GraphicsResources.h"

namespace Seele
{
// Minimal vertex source used for building commands
struct VertexInputStream
{
    uint32 streamIndex : 4;
    uint32 offset : 28;
    Gfx::PVertexBuffer vertexBuffer;

    VertexInputStream()
        : streamIndex(0), offset(0), vertexBuffer(nullptr)
    {
    }

    VertexInputStream(uint32 streamIndex, uint32 offset, Gfx::PVertexBuffer vertexBuffer)
        : streamIndex(streamIndex), offset(offset), vertexBuffer(vertexBuffer)
    {
        assert(this->streamIndex == streamIndex && this->offset == offset);
    }

    inline bool operator==(const VertexInputStream &rhs) const
    {
        if (streamIndex != rhs.streamIndex || offset != rhs.offset || vertexBuffer != rhs.vertexBuffer)
        {
            return false;
        }
        return true;
    }
    inline bool operator!=(const VertexInputStream &rhs) const
    {
        return !(*this == rhs);
    }
};
typedef Array<VertexInputStream> VertexInputStreamArray; //TODO inline allocation

// Typed data source for a vertex factory
struct VertexStreamComponent
{
    // Source vertex buffer
    Gfx::PVertexBuffer vertexBuffer = nullptr;

    // Offset to the start of the vertex buffer fetch
    uint32 streamOffset = 0;

    // Offset of the data, relative to the beginnning of each element in the vertex buffer
    uint32 offset = 0;

    // Stride of the data
    uint32 stride = 0;

    Gfx::SeFormat type = Gfx::SE_FORMAT_UNDEFINED;

    VertexStreamComponent()
    {}
    
    VertexStreamComponent(const Gfx::PVertexBuffer vertexBuffer, uint32 offset, uint32 stride, Gfx::SeFormat type)
        : vertexBuffer(vertexBuffer)
        , streamOffset(0)
        , offset(offset)
        , stride(stride)
        , type(type)
    {}

    VertexStreamComponent(const Gfx::PVertexBuffer vertexBuffer, uint32 streamOffset, uint32 offset, uint32 stride, Gfx::SeFormat type)
        : vertexBuffer(vertexBuffer)
        , streamOffset(streamOffset)
        , offset(offset)
        , stride(stride)
        , type(type)
    {}
};

#define STRUCTMEMBER_VERTEXSTREAMCOMPONENT(vertexBuffer, vertexType, member, memberType) \
    VertexStreamComponent(vertexBuffer, offsetof(vertexType, member), sizeof(vertexType), memberType)

class VertexInputType
{
public:
    static List<VertexInputType*>& getTypeList();
    static VertexInputType* getVertexInputByName(const std::string& name);

    VertexInputType(
        const char* name,
        const char* shaderFilename
    );
    virtual ~VertexInputType();

    const char* getName();
    const char* getShaderFilename();
private:
    const char* name;
    const char* shaderFilename;

    static List<VertexInputType*> globalTypeList;
};

#define DECLARE_VERTEX_INPUT_TYPE(inputClass) \
    public: \
    static VertexInputType staticType; \
    virtual VertexInputType* getType() const override;

#define IMPLEMENT_VERTEX_INPUT_TYPE(inputClass, shaderFilename) \
    VertexInputType inputClass::staticType( \
        #inputClass, \
        shaderFilename); \
        VertexInputType* inputClass::getType() const { return &staticType; }

struct MeshDescription;
DECLARE_REF(VertexShaderInput);
class VertexShaderInput
{
public:
    VertexShaderInput(std::string name);
    virtual ~VertexShaderInput();
    virtual void init(Gfx::PGraphics graphics) {};
    void getStreams(VertexInputStreamArray& outVertexStreams) const;
    void getPositionOnlyStream(VertexInputStreamArray& outVertexStreams) const;
    virtual bool supportsTesselation() { return false; }
    virtual VertexInputType* getType() const { return nullptr; }
    Gfx::PVertexDeclaration getDeclaration() const {return declaration;}
    Gfx::PVertexDeclaration getPositionDeclaration() const {return positionDeclaration;}
    std::string getName() const { return name; }
protected:
    Gfx::VertexElement accessStreamComponent(const VertexStreamComponent& component, uint8 attributeIndex);
    Gfx::VertexElement accessPositionStreamComponent(const VertexStreamComponent& component, uint8 attributeIndex);

    void initDeclaration(Gfx::PGraphics graphics, Array<Gfx::VertexElement>& elements);
    void initPositionDeclaration(Gfx::PGraphics graphics, const Array<Gfx::VertexElement>& elements);

    static List<PVertexShaderInput> registeredInputs;
    static std::mutex registeredInputsLock;
    Array<Gfx::VertexAttribute> layout;

    // internal helper class, basically a VertexInputStream plus stride
    struct VertexStream
    {
        Gfx::PVertexBuffer vertexBuffer = nullptr;
        uint8 stride = 0;
        uint8 offset = 0;
        friend bool operator==(const VertexStream& a, const VertexStream& b)
        {
            return a.vertexBuffer == b.vertexBuffer &&
                a.stride == b.stride &&
                a.offset == b.offset;
        }
        VertexStream()
        {
        }
    };
    Array<VertexStream> streams;
    Array<VertexStream> positionStreams;
    Gfx::PVertexDeclaration declaration;
    Gfx::PVertexDeclaration positionDeclaration;
    std::string name;
};
DEFINE_REF(VertexShaderInput);
} // namespace Seele
