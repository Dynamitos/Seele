#pragma once
#include "RenderPath.h"
namespace Seele
{
	// A view is a part of the window, which can be anything from a viewport to an editor
	class View
	{
	public:
		View();
		virtual ~View();
		virtual void initRenderer() = 0;
		void beginFrame();
		void endFrame();
		void applyArea(Rect area);
	protected:
		PRenderPath renderer;
	};

	DECLARE_REF(View)
}