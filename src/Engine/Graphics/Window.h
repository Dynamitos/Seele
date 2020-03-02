#pragma once
#include "MinimalEngine.h"
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
	struct Section
	{
	};
	DECLARE_REF(Section)

	class Window
	{
	public:
		Window(const WindowCreateInfo& createInfo);
		~Window();
	private:
		void* windowHandle;
		Section top;
		Section bot;
		Section left;
		Section right;
		Section center;
		uint32 width;
		uint32 height;
		Graphics* graphics;
	};
	DECLARE_REF(Window)
}