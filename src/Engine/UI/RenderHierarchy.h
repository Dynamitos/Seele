#pragma once
#include "Elements/Element.h"
#include "Containers/List.h"
#include "Graphics/GraphicsResources.h"

namespace Seele
{
namespace UI
{
struct RenderElementStyle
{
    Vector position = Vector(0, 0, 0);
    uint32 backgroundImageIndex = UINT32_MAX;
    Vector backgroundColor = Vector(1, 1, 1);
    float opacity = 1.0f;
    //Vector4 borderBottomColor = Vector4(1, 1, 1, 1);
    //Vector4 borderLeftColor = Vector4(1, 1, 1, 1);
    //Vector4 borderRightColor = Vector4(1, 1, 1, 1);
    //Vector4 borderTopColor = Vector4(1, 1, 1, 1);
    //float borderBottomLeftRadius = 0;
    //float borderBottomRightRadius = 0;
    //float borderTopLeftRadius = 0;
    //float borderTopRightRadius = 0;
    Vector2 dimensions = Vector2(1, 1);
};
class RenderElement
{
public:
    RenderElement() = default;
    RenderElement(Element* parent, Element* referencedElement)
        : parent(parent)
        , referencedElement(referencedElement)
    {}
    ~RenderElement() = default;
    Element* parent;
    Element* referencedElement;
};

struct RenderHierarchyUpdate
{
    virtual void apply(Array<RenderElement>& elements) = 0;
};

struct AddElementRenderHierarchyUpdate : public RenderHierarchyUpdate
{
    Element* addedElement;
    Element* parent;
    AddElementRenderHierarchyUpdate(Element* addedElement, Element* parent)
        : addedElement(addedElement)
        , parent(parent)
    {}
    virtual void apply(Array<RenderElement>& elements) override;
};

struct RemoveElementRenderHierarchyUpdate : public RenderHierarchyUpdate
{
    Element* element;
    bool removeChildren;
    RemoveElementRenderHierarchyUpdate(Element* elementToRemove, bool removeChildren = false)
        : element(elementToRemove)
        , removeChildren(removeChildren)
    {}
    virtual void apply(Array<RenderElement>& elements) override;
};
class RenderHierarchy
{
public:
    RenderHierarchy();
    ~RenderHierarchy();
    // logic thread interface, queue hierarchy changes
    void addElement(PElement addedElement);
    void removeElement(PElement elementToRemove);
    void moveElement(PElement elementToMove, PElement newParent);

    // render thread interface, apply changes
    void updateHierarchy();
private:
    static_assert(std::is_trivially_copyable_v<RenderElement>);
    // List of all drawable elements in draw order
    Array<RenderElement> drawElements;
    // Shader data used for styling elements
    Array<RenderElementStyle> elementStyles;
    Array<Gfx::PTexture> usedTextures;

    List<RenderHierarchyUpdate*> updates;
    std::mutex updateLock;
};
} // namespace UI
} // namespace Seele
