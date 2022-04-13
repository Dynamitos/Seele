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
    Vector2 position;
    Vector2 dimensions;
    Vector backgroundColor;
    uint32 backgroundImageIndex;
    Vector4 borderBottomColor;
    Vector4 borderLeftColor;
    Vector4 borderRightColor;
    Vector4 borderTopColor;
    float borderBottomLeftRadius;
    float borderBottomRightRadius;
    float borderTopLeftRadius;
    float borderTopRightRadius;
    Vector4 fontColor;
    float fontSize;
    float opacity;
};
static_assert(sizeof(RenderElementStyle) == 34*4);
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
