#pragma once
#include "Graphics.h"
namespace Seele
{
	//A renderpath is a general Renderer for a view.
	class RenderPath
	{
	public:
		RenderPath(Graphics* graphics);
		virtual ~RenderPath();
		virtual void applyArea(Rect area) = 0;
		virtual void init() = 0;
		virtual void beginFrame() = 0;
		virtual void render() = 0;
		virtual void endFrame() = 0;
	protected:
		Graphics* graphics;
		Rect area;
	};
	DECLARE_REF(RenderPath);
}