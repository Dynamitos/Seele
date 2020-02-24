#pragma once
#include "MinimalEngine.h"
namespace Seele {
	struct WindowCreateInfo
	{
		int32 width;
		int32 height;
		const char* title;
	};
	class Window
	{
	public:
		Window(const WindowCreateInfo& createInfo);
		~Window();
	private:
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
		Section top;
		Section bot;
		Section left;
		Section right;
		Section center;
		uint32 width;
		uint32 height;
	};
}