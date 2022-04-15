#pragma once
#include "View.h"
#include "Graphics/RenderPass/RenderGraph.h"
#include "Graphics/RenderPass/UIPass.h"
#include "Graphics/RenderPass/TextPass.h"
#include "UI/Elements/Panel.h"

namespace Seele
{
DECLARE_REF(Actor)
class InspectorView : public View
{
public:
    InspectorView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &createInfo);
    virtual ~InspectorView();

    virtual void beginUpdate() override;
	virtual void update() override;
	virtual void commitUpdate() override;
    
    virtual void prepareRender() override;
    virtual void render() override;
    void selectActor();
protected:
    UIPass uiPass;
    TextPass textPass;
    
    UIPassData uiPassData;
    TextPassData textPassData;

    UI::PPanel rootPanel;
    PActor selectedActor;
    
	virtual void keyCallback(KeyCode code, InputAction action, KeyModifier modifier) override;
	virtual void mouseMoveCallback(double xPos, double yPos) override;
	virtual void mouseButtonCallback(MouseButton button, InputAction action, KeyModifier modifier) override;
	virtual void scrollCallback(double xOffset, double yOffset) override;
	virtual void fileCallback(int count, const char** paths) override;
};
DEFINE_REF(InspectorView)
} // namespace Seele
