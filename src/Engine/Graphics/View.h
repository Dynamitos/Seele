#pragma once
#include "RenderPath.h"
namespace Seele
{
	class Graphics;
	// A view is a part of the window, which can be anything from a viewport to an editor
	class View
	{
	public:
		View(Graphics* graphics);
		virtual ~View();
		void beginFrame();
		void endFrame();
		void applyArea(Rect area);
	protected:
		Graphics* graphics;
		PRenderPath renderer;
	};

	DECLARE_REF(View)
}