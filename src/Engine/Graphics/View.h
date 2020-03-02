#pragma once
#include "Window.h"
#include "RenderPath.h"
namespace Seele
{
	// A view is a part of the window, which can be anything from a viewport to an editor
	class View
	{
	public:
		View(PSection owner);
		~View();
	private:
		PRenderPath renderer;
		PSection owner;
	};

	DECLARE_REF(View)
}