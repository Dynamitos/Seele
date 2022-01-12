#pragma once
#include "View.h"
#include "Graphics/RenderPass/RenderGraph.h"
#include "Graphics/RenderPass/UIPass.h"
#include "UI/Elements/Panel.h"

namespace Seele
{
DECLARE_REF(Actor)
class InspectorView : public View
{
public:
    InspectorView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &createInfo);
    virtual ~InspectorView();

    virtual Job beginUpdate() override;
	virtual Job update() override;
	virtual void commitUpdate() override;
    
    virtual void prepareRender() override;
    virtual MainJob render() override;
    void selectActor();
protected:
    UIPass uiPass;
    
    UIPassData uiPassData;

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
