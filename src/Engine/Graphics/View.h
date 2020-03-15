#pragma once
#include "RenderPath.h"
namespace Seele
{
	DECLARE_REF(Graphics);
	// A view is a part of the window, which can be anything from a viewport to an editor
	class View
	{
	public:
		View(PGraphics graphics);
		virtual ~View();
		void beginFrame();
		void endFrame();
		void applyArea(Rect area);
	protected:
		PGraphics graphics;
		PRenderPath renderer;
	};

	DEFINE_REF(View)
}