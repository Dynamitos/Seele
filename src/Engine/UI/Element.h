#pragma once
#include "Attributes.h"
#include "Containers/Array.h"
#include "MinimalEngine.h"
#include "Style/Style.h"

namespace Seele {
namespace UI {
struct UIRender {
    Gfx::PTexture2D backgroundTexture;
    Vector backgroundColor;
    Vector2 position;
    Vector2 dimensions;
    uint32 z;
    uint32 level;
};
DECLARE_REF(Element)
class Element {
  public:
    Element(Attributes attr, Array<Element*> children);
    virtual ~Element();
    void setParent(PElement p) { parent = p; }
    // calculates the final style object taking into account inherited properties
    virtual void applyStyle(Style parentStyle) = 0;
    void calcStyle(Style parentStyle) {
        applyStyle(parentStyle);
        for (auto& child : children) {
            child->calcStyle(style);
        }
    }
    // calculates the relative positions of the child elements for the applied layout style
    virtual void layout(UVector2 parentSize) {
        if (style.maxWidthType == DimensionType::Auto) {
            maxDimensions.x = (float)parentSize.x;
        }
        if (style.maxHeightType == DimensionType::Auto) {
            maxDimensions.y = (float)parentSize.y;
        }
        if (style.widthType == DimensionType::Pixel) {
            dimensions.x = style.width;
        } else if (style.widthType == DimensionType::Percent) {
            dimensions.x = parentSize.x * style.width / 100.0f;
        }
        if (style.heightType == DimensionType::Pixel) {
            dimensions.y = style.height;
        } else if (style.heightType == DimensionType::Percent) {
            dimensions.y = parentSize.y * style.height / 100.0f;
        }

        for (auto& child : children) {
            child->layout(maxDimensions - Vector2(style.marginLeft + style.marginRight, style.marginTop + style.marginBottom));
        }
        switch (style.innerDisplay) {
        case InnerDisplayType::Flow:
            flowLayout();
            break;
        }
    }
    // normal flow of the elements, inline elements go left to right, blocks get their own new lines
    void flowLayout() {
        // use a cursor going from left to right, top to bottom, starting at padding
        Vector2 cursor = Vector2(style.paddingLeft, style.paddingTop);
        uint32 lineHeight = 0;
        for (auto& child : children) {
            // inline elements flow left to right
            if (child->style.outerDisplay == OuterDisplayType::Inline) {
                // only static and relative elements actually reserve
                if (child->style.position == PositionType::Static || child->style.position == PositionType::Relative) {
                    // create a new line in case the element doesnt fit on the current one anymore,
                    // but keep in current line in case we are still at the start, as a new line wouldnt help here
                    if (cursor.x + child->dimensions.x > maxDimensions.x - child->style.left && cursor.x != 0) {
                        cursor.x = (float)style.paddingLeft;
                        cursor.y += lineHeight;
                        lineHeight = 0;
                    }
                    // todo: border
                    child->position.x = cursor.x + child->style.marginLeft;
                    child->position.y = cursor.y + child->style.marginTop;
                    cursor.x = child->position.x + child->dimensions.x;
                    // apply offsets after updating the cursor for relative elements
                    if (child->style.position == PositionType::Relative) {
                        child->position.x += child->style.left;
                        child->position.y += child->style.top;
                    }
                    lineHeight = std::max(lineHeight, child->style.lineHeight);
                }
                // block elements always get their own lines, like paragraphs
            } else if (child->style.outerDisplay == OuterDisplayType::Block) {
                // static and relative require space to be created for the elements
                if (child->style.position == PositionType::Static || child->style.position == PositionType::Relative) {
                    // create a new line in case we are not already at one
                    if (cursor.x != 0) {
                        cursor.x = (float)style.paddingLeft;
                        cursor.y += lineHeight;
                    }
                    child->position.x = cursor.x + child->style.marginLeft;
                    child->position.y = cursor.y + child->style.marginTop;
                    // new line after block
                    cursor.x = 0;
                    cursor.y = child->position.y + child->dimensions.y;
                }
            }
        }
        if (style.widthType == DimensionType::Auto) {
            dimensions.x = cursor.x + style.paddingRight;
        }
        if (style.heightType == DimensionType::Auto) {
            dimensions.y = lineHeight + cursor.y + style.paddingBottom;
        }
    }
    virtual Array<UIRender> render(Vector2 anchor, uint32 level) {
        Array<UIRender> result = {
            UIRender{
                .backgroundColor = style.backgroundColor,
                .position = position + anchor,
                .dimensions = dimensions,
                .z = style.z,
                .level = level,
            },
        };
        for (auto& child : children) {
            for (const auto& render : child->render(position + anchor, level+1)) {
                result.add(render);
            }
        }
        return result;
    }
    Style style;
    Vector2 maxDimensions = Vector2(0);
    // calculated position relative to parent
    Vector2 position = Vector2(0);
    // calculated dimensions in pixels after layouting
    Vector2 dimensions = Vector2(0);

  protected:
    Attributes attr;
    PElement parent = nullptr;
    Array<OElement> children;
};
DEFINE_REF(Element)
} // namespace UI
} // namespace Seele