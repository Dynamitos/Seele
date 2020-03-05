#pragma once
#include "GraphicsResources.h"
#include "View.h"
namespace Seele {
	
	// A window is divided into 5 sections, using a border layout
	// +--------------TOP------------------+
	// |								   |
	// L								   R
	// E								   I
	// F			CENTER				   G
	// T								   H
	// |								   T
	// +-------------BOTTOM----------------+
	// In every section, there can be any number
	// of views, when there are multiple, they stack to tabs
	class Section
	{
	public:
		Section()
		{}
		void resizeArea(Rect area)
		{
			this->area = area;
		}
		void beginFrame()
		{
			views[0]->beginFrame();
		}
		void endFrame()
		{
			views[0]->endFrame();
		}
		void addView(PView view)
		{
			view->applyArea(area);
			views.add(view);
		}
		void clearViews(PView view)
		{
			views.clear();
		}
		void removeView(PView view)
		{
			views.remove(views.find(view));
		}
	private:
		Rect area;
		Array<PView> views;
	};
	DECLARE_REF(Section)
	class Graphics;
	class Window
	{
	public:
		Window(const WindowCreateInfo& createInfo);
		~Window();
		void beginFrame();
		void endFrame();
	private:
		void* windowHandle;
		PSection center;
		uint32 width;
		uint32 height;
		Graphics* graphics;
	};
	DECLARE_REF(Window)
}