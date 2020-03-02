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
		virtual void init() = 0;
		virtual void beginFrame() = 0;
		virtual void render() = 0;
		virtual void endFrame() = 0;
	protected:
		Graphics* graphics;
	};
	DECLARE_REF(RenderPath);
}