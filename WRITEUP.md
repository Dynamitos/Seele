# Seele Engine

Seele is a Proof-of-Concept 3D render engine using Vulkan as it's primary graphics API, but other APIs could also be implemented. Due to this, a lot of terminology will be Vulkan-related, but it can be translated to other APIs.
The focus of the project is to make the renderer use as many of the system resources as efficiently as possible to maximize the framerate
while minimizing resource usage.
It does this by being designed to support multithreading for more recent APIs, like DX12 or Vulkan, while still providing an interface that could be implemented by an
API that doesn't support it, like OpenGL. These APIs would suffer a significant performance loss, but would still work correctly.

## Custom containers and other utilities

Some of the basic containers are reimplemented, mostly not for any particular feature, but because that makes them more customizable and extendable for future uses and
performance optimizations, like serialization for example. Due to some of them occuring in the code samples later, they will be quickly mentioned here.

- `Array`: A heap-array backed dynamically resizable container, like `std::vector`
- `StaticArray`: A static-array backed non-resizable container, like `std::array`
- `Map`: A key-value splay-tree associative container, like `std::map`
- `UniquePtr`: A smart pointer claiming ownership of an object, like `std::unique_pointer`
- `RefPtr`: A reference counting smart pointer, similar to `std::shared_pointer`, with a few differences:
  - global pointer-based reference counting
  - this means a `RefPtr` can be constructed from a raw pointer and will retain its reference count correctly
  - for example if you construct `RefPtr<HelloWorld> ptr = new HelloWorld();`, and in the constructor of HelloWorld create `RefPtr<HelloWorld ptr = this;`, both would have a reference count of 2.
- `DEFINE_REF`/`DECLARE_REF`: Macros to automatically typedef different types of smart pointer for convenience, for example DEFINE_REF(HelloWorld) would typedef:
  - `PHelloWorld`, the same as `RefPtr<HelloWorld>`
  - `UPHelloWorld`, the same as `UniquePtr<HelloWorld>`

## Basic Architecture

The first piece to the puzzle is the `WindowManager`, a container that manages a `Graphics` object, which is the interface for the graphics API, as well as all `Window`s that are active. Each `Window` represents, well, a window, and contains several `View`s. A `View` represents a viewport in the window. For an analogy to Unity, a `View` would be the Inspector, the Asset Browser or the Hierarchy.

Since all viewports in a window are backed by the same image and can only be presented together, the rendering for all viewports is synchronized, while the logic for each `View` runs in a separate thread.

The interactions between logic and rendering are modelled through two parts of the interface: `beginUpdate()`, `update()` and `commitUpdate()` for the logic side, and `prepareRender()` and `render()` for the rendering side. The views use a set of concrete `RenderPass` objects to render a new frame, depending on what kind of content should be displayed. In order to draw a frame, a `RenderPass` needs an "update packet" in form of a struct, but the contents of that packet is dependent on the render pass itself. A depth prepass for example might only need a mesh list, while a light culling pass needs the dynamic lights or a base pass might need both.

In order to avoid inheritance and casting for these update packets, each `RenderPass` needs to be updated individually by the view that uses it, so the thread interactions are not enforced in the `View` base class, instead `commitUpdate()` is called from a locked context from the logic thread, so that the view can copy the packet to a shared storage, while `prepareRender()` is locked and called from the render thread, so that it can be passed to the render passes.

## RenderPass

To better illustrate how render passes work, here is part of the `RenderPass` base class:

```cpp
template<typename RenderPassDataType>
class RenderPass
{
public:
    RenderPass(Gfx::PGraphics graphics, Gfx::PViewport viewport)
        : graphics(graphics)
        , viewport(viewport)
    {}
    virtual ~RenderPass()
    {}
    void updateViewFrame(RenderPassDataType viewFrame) {
        passData = std::move(viewFrame);
    }
    virtual void beginFrame() = 0;
    virtual void render() = 0;
    virtual void endFrame() = 0;
    virtual void publishOutputs() = 0;
    virtual void createRenderPass() = 0;
    //....
private:
    PRenderGraphResources resources;
    RenderPassDataType passData;
    Gfx::PGraphics graphics;
    //....
};
```

All render passes in a `View` share a `RenderGraphResources` object, which is used to share outputs between the different passes. During initialization, every render pass "publishes" any outputs that it produces in `publishOutputs()`, so that they can be consumed by other render passes. For example the output of a depth prepass is the depth buffer, for a light culling pass it is the culled light indices. Next the render passes get any resources that they require and have been registered by other passes to create the render pass in `createRenderPass()`. This is split into two parts, so that the order of initialization of different renderpasses doesn't matter, as all outputs are available before any are requested.

## Materials

The material system fully embraced a Vulkan-style descriptor system, abstracted by the `Graphics` API.  A material is a file stored on disk in form of a JSON document, here is an example:

```json
{
    "name": "Placeholder",
    "profile": "BlinnPhong",
    "params": {
        "diffuseTexture": {
            "type": "Texture2D"
        },
        "specularTexture": {
            "type": "Texture2D"
        },
        "normalTexture": {
            "type": "Texture2D"
        },
        "uvScale": {
            "type": "float",
            "default": "0.0f"
        },
        "metallic": {
            "type": "float",
            "default": "0.0f"
        },
        "roughness": {
            "type": "float",
            "default": "0.5f"
        },
        "sheen": {
            "type": "float",
            "default": "0.0f"
        },
        "worldOffset": {
            "type": "float3",
            "default": "float3(0, 0, 0)"
        },
        "textureSampler": {
            "type": "SamplerState"
        }
    },
    "code": {
        "baseColor": [
            "return diffuseTexture.Sample(textureSampler, input.texCoords[0] * uvScale).xyz;"
        ],
        "metallic": [
            "return metallic;"
        ],
        "normal": [
            "return normalTexture.Sample(textureSampler, input.texCoords[0] * uvScale).xyz;"
        ],
        "specular": [
            "return specularTexture.Sample(textureSampler, input.texCoords[0] * uvScale).x;"
        ],
        "roughness": [
            "return roughness;"
        ],
        "sheen": [
            "return sheen;"
        ],
        "worldOffset": [
            "return worldOffset;"
        ]
    }
}
```

The document starts with the asset `name`, followed by the `profile`, which tells the engine which BRDF function it uses, and thus what outputs it can expect from the material. Next are the parameters, which are the inputs that need to get passed to the shader later. The `type` field must be a valid HLSL datatype and `default` a valid value for that datatype. In the `code` section, there must be an entry for every output required by the `profile`, each of which is a string array for each line of shader code to compute that value and ends by `return`ing that value.

### MaterialAsset

A `MaterialAsset` represents such a JSON document on disk, as well as the GPU resources needed to render a mesh with the material. When loading the file, the a descriptor layout is generated to fit the `slang` code that will be generated later by consolidating the plain variables (`float`, `double`, etc.) into a single uniform buffer. The parameters are stored in an array of `ShaderParameter`s, which contain the current parameter value, like texture references or plain data, as well as information on how to update the GPU resources.

At the same time a slang-compilable shader is generated, which matches the parameters provided by the descriptor layout. The generated shader is a `Material`, which is an interface that "prepares" a BRDF by initializing its input values, similar to official slang example. How each input value is calculated is read from the `code` section of the JSON document, and wrapped in an accessor to allow for multi line calculations.

Here is the material generated from the above JSON example:

```cpp
import VERTEX_INPUT_IMPORT;
import Material;
import BRDF;
import MaterialParameter;

struct Placeholder: IMaterial {
    layout(offset = 0)float anisotropic;
    layout(offset = 4)float clearCoat;
    layout(offset = 8)float clearCoatGloss;
    Texture2D diffuseTexture;
    layout(offset = 12)float metallic;
    Texture2D normalTexture;
    layout(offset = 16)float roughness;
    layout(offset = 20)float sheen;
    layout(offset = 24)float sheenTint;
    Texture2D specularTexture;
    layout(offset = 28)float specularTint;
    layout(offset = 32)float subsurface;
    SamplerState textureSampler;
    layout(offset = 36)float uvScale;
    layout(offset = 40)float3 worldOffset;

    float3 getBaseColor(MaterialFragmentParameter input) {
        return diffuseTexture.Sample(textureSampler, input.texCoords[0] * uvScale).xyz;;
    }
    float getMetallic(MaterialFragmentParameter input) {
        return 0;;
    }
    float3 getNormal(MaterialFragmentParameter input) {
        return normalTexture.Sample(textureSampler, input.texCoords[0] * uvScale).xyz;;
    }
    float getSpecular(MaterialFragmentParameter input) {
        return specularTexture.Sample(textureSampler, input.texCoords[0] * uvScale).x;;
    }
    float getRoughness(MaterialFragmentParameter input) {
        return roughness;;
    }
    float getSheen(MaterialFragmentParameter input) {
        return sheen;;
    }
    float3 getWorldOffset() {
    return worldOffset;;
    }
    typedef BlinnPhong BRDF;
    BlinnPhong prepare(MaterialFragmentParameter geometry){
        BlinnPhong result;
        result.baseColor = getBaseColor(geometry);
        result.metallic = getMetallic(geometry);
        result.normal = getNormal(geometry);
        result.specular = getSpecular(geometry);
        result.roughness = getRoughness(geometry);
        result.sheen = getSheen(geometry);
        return result;
    }
};
```

### Renderpass interaction

Each render pass has a "base file" containing the vertex and fragment stages. From that base file a variant is generated for each material that will be used with that render pass using `#define` macros to change the material import, and a `type_param` to update the `ParameterBlock` containing the material itself. Other renderpass dependent data like camera parameter or lights are stored in separate descriptor sets, and to avoid code duplication, they parts that are used by more than one render pass are implemented in separate slang files. But since in Vulkan pipeline layouts cannot have empty indices, the descriptor set indices sometimes need to change. This is done using the static `modifyRenderPassMacros` function in each renderpass to `#define` the correct set indices for each shared descriptor layout.
